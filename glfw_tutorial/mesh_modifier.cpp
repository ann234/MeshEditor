#include "mesh_modifier.h"

glm::vec3 get_ray_by_window(GLfloat width, GLfloat height, const float xpos, const float ypos)
{
	//	3d normalized Device Coordinates
	float x = (2.0f * xpos) / width - 1.0f;		//	[0, 1024] -> [-1, 1]
	float y = 1.0f - (2.0f * ypos) / height;	//	[0, 768] -> [-1, 1]
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	//	4d Homogeneous Clip Coordinates
	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

	//	4d Eye(Camera) Coordinates
	glm::vec4 ray_eye = glm::inverse(object_manager::projection_matrix) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	//	4d World Coordinates
	glm::vec3 ray_wor = (glm::inverse(object_manager::view_matrix) * ray_eye);
	ray_wor = glm::normalize(ray_wor);

	//std::cout << "Ray direction: " << vec3_toString(ray_wor) << std::endl;

	return ray_wor;
}



MeshObject* ray_casting(GLFWwindow* window, std::vector<MeshObject*> objects, 
	bool is_vertex_or_face, bool is_delete_or_search)
{
	//std::cout << "Ray casting starts" << std::endl;

	//	Get mouse position
	//	2d Viewport Coordinate
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glm::vec3 ray_dir = get_ray_by_window(width, height, xpos, ypos);

	//	obj에서 picking된 vertex or face를 찾았나?
	bool is_found = false;
	MeshObject* nearest_obj = NULL;

	//	모든 MeshObject(obb)를 돌면서 picking 검사
	if (is_vertex_or_face)
	{
		//	Camera와 가장 가까운 vertex
		OpenMesh::VertexHandle nearest_vertex;
		float nearest_distance = 100;

		for each(MeshObject* obj in objects)
		{
			MyMesh mesh = obj->mesh;
			//	MeshObject의 vertex들을 돌면서
			for each(MyMesh::VertexHandle vh in mesh.vertices())
			{
				for (float t = 0.1f; t < 3; t += 0.01f)
				{
					glm::vec3 ray_t = ray_dir * t;
					glm::vec3 ray_pos = camera_pos + ray_t;
					glm::vec3 point = caster(mesh.point(vh)) + obj->position;

					float distance = glm::length(camera_pos - point);
					float len = glm::length(ray_pos - point);

					//	Vertex normal
					glm::vec3 v_normal = caster(mesh.normal(vh));
					//	If ray and vertex normal's dot product is positive number, outside
					float is_valid = glm::dot(ray_dir, v_normal);

					if (is_valid < 0 && len <= 0.01f && distance < nearest_distance)
					{
						nearest_obj = obj;
						nearest_distance = len;
						nearest_vertex = vh;
						is_found = true;
						break;
					}
				}
			}
		}
		if (is_found)
		{
			//std::cout << "picked: " << nearest_distance << std::endl;
			if (is_delete_or_search)
			{
				delete_vertex(*nearest_obj, nearest_vertex);
			}
			else
			{
				bfs_and_note(nearest_obj, nearest_vertex);
			}
		}
	}
	else
	{
		//	Camera와 가장 가까운 face
		OpenMesh::FaceHandle nearest_face;
		float nearest_distance = 100;
		for each(MeshObject* obj in objects)
		{
			MyMesh mesh = obj->mesh;
			//	MeshObject의 face들을 돌면서
			for each(MyMesh::FaceHandle fh in mesh.faces())
			{
				MyMesh::ConstFaceVertexIter cfvIt;

				glm::vec3 vertices[3];
				int count = 0;
				//	Face의 vertices 위치를 가져올 때,
				//	기존 mesh의 vertex 위치 + MeshObject의 이동거리
				for (cfvIt = mesh.cfv_begin(fh); cfvIt.is_valid(); ++cfvIt)
				{
					vertices[count++] = caster(mesh.point(cfvIt.handle())) + obj->position;
				}

				//	Edge vectors
				glm::vec3 e_1 = vertices[1] - vertices[0];
				glm::vec3 e_2 = vertices[2] - vertices[0];
				//	Face normal
				glm::vec3 n = caster(mesh.normal(fh));

				for (float t = 0.1f; t < 3; t += 0.01f)
				{
					glm::vec3 ray_t = ray_dir * t;

					glm::vec3 q = glm::cross(ray_t, e_2);
					float a = glm::dot(e_1, q);

					//	Backfacing or nearly parallel
					if (glm::dot(n, ray_t) < 0 && glm::abs(a) > 0.00001f)
					{
						glm::vec3 s = (camera_pos - vertices[0]) / a;
						glm::vec3 r = glm::cross(s, e_1);

						float b[3];
						b[0] = glm::dot(s, q);
						b[1] = glm::dot(r, ray_t);
						b[2] = 1.0f - b[0] - b[1];

						if ((b[0] >= 0.0f) && (b[1] >= 0.0f) && (b[2] >= 0.0f))
						{
							float distance = glm::dot(e_2, r);
							//	이전에 찾은 face보다 camera와 가까이 있을 경우, nearest 교체 
							if (distance > 0 && distance <= nearest_distance)
							{
								nearest_obj = obj;
								nearest_face = fh;
								nearest_distance = distance;
								is_found = true;
							}
							break;
						}
					}
				}
			}
		}
		if (is_found)	//	가장 가까운 triangle을 찾으면 종료
		{
			//std::cout << "picked: " << nearest_distance << std::endl;
			if (is_delete_or_search)
			{
				delete_face(*nearest_obj, nearest_face);
			}
			else
			{
				bfs_and_note(nearest_obj, nearest_face);
			}
		}
	}

	if (is_found)
	{
		return nearest_obj;
	}
	else
	{
		return NULL;
	}
	
	std::cout << "Ray casting end" << std::endl;
}

void change_color(MeshObject& obj, std::vector<MyMesh::VertexHandle> vertices, glm::vec3 color)
{
	for each(MyMesh::VertexHandle vertex in vertices)
	{
		obj.mesh.set_color(vertex, f_to_uc(color));
		obj.color_buffer[vertex.idx()] = color;
	}

	glBindBuffer(GL_ARRAY_BUFFER, obj.get_color_id());
	glBufferData(GL_ARRAY_BUFFER, obj.color_buffer.size() * sizeof(glm::vec3), &obj.color_buffer[0], GL_STATIC_DRAW);
}

void delete_vertex(MeshObject& obj, MyMesh::VertexHandle& vh)
{
	obj.vertex_buffer.clear();
	obj.color_buffer.clear();
	obj.normal_buffer.clear();
	obj.index_buffer.clear();

	//	delete_vertex 함수 실행 전, status들을 request 해주어야 한다고 한다.
	obj.mesh.request_face_status();
	obj.mesh.request_vertex_status();
	obj.mesh.request_edge_status();
	obj.mesh.request_halfedge_status();
	obj.mesh.delete_vertex(vh);

	//	delete_face 함수의 주석에서 보이듯, 함수를 콜해도 delete할 것이라고 표시만 해놓을 뿐,
	//	실제로 삭제하기 위해서는 garbage_colleciton 함수를 콜해야 한다고 한다.
	obj.mesh.garbage_collection();
	
	MyMesh::VertexIter vit = obj.mesh.vertices_sbegin();
	for (; vit != obj.mesh.vertices_end(); vit++)
	{
		OpenMesh::Vec3f p, n;
		p = obj.mesh.point(vit.handle());
		obj.vertex_buffer.push_back(glm::vec3(p[0], p[1], p[2]));
		//std::cout << p << std::endl;

		//	save normal data
		n = obj.mesh.normal(vit.handle());
		obj.normal_buffer.push_back(glm::vec3(n[0], n[1], n[2]));
		//std::cout << n << std::endl;

		obj.mesh.set_color(vit.handle(), f_to_uc(0, 0.8, 0));
		OpenMesh::Vec3f color = uc_to_f(obj.mesh.color(vit.handle()));
		obj.color_buffer.push_back(glm::vec3(color[0], color[1], color[2]));
	}
	
	MyMesh::FaceIter fit = obj.mesh.faces_sbegin();
	for (; fit != obj.mesh.faces_end(); fit++)
	{
		MyMesh::ConstFaceVertexIter cfvIt;
		for (cfvIt = obj.mesh.cfv_begin(fit.handle()); cfvIt.is_valid(); ++cfvIt)
		{
			obj.index_buffer.push_back(cfvIt->idx());
			//std::cout << cfvlt->idx() << std::endl;
		};
	}

	//for each(MyMesh::VertexHandle vh in obj.mesh.vertices())
	//{
	//	OpenMesh::Vec3f p, n;
	//	p = obj.mesh.point(vh);
	//	obj.vertex_buffer.push_back(glm::vec3(p[0], p[1], p[2]));
	//	//std::cout << p << std::endl;

	//	//	save normal data
	//	n = obj.mesh.normal(vh);
	//	obj.normal_buffer.push_back(glm::vec3(n[0], n[1], n[2]));
	//	//std::cout << n << std::endl;

	//	obj.mesh.set_color(vh, f_to_uc(0, 0.8, 0));
	//	OpenMesh::Vec3f color = uc_to_f(obj.mesh.color(vh));
	//	obj.color_buffer.push_back(glm::vec3(color[0], color[1], color[2]));
	//}
	////	save index data
	//for each (MyMesh::FaceHandle fh in obj.mesh.faces())
	//{
	//	MyMesh::ConstFaceVertexIter cfvIt;
	//	for (cfvIt = obj.mesh.cfv_begin(fh); cfvIt.is_valid(); ++cfvIt)
	//	{
	//		obj.index_buffer.push_back(cfvIt->idx());
	//		//std::cout << cfvlt->idx() << std::endl;
	//	};
	//}

	//	Bind vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_vertex_id());
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * obj.vertex_buffer.size(),
		&obj.vertex_buffer[0],
		GL_STATIC_DRAW);

	//	Bind color buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_color_id());
	glBufferData(GL_ARRAY_BUFFER,
		obj.color_buffer.size() * sizeof(glm::vec3),
		&obj.color_buffer[0],
		GL_STATIC_DRAW);

	//	Bind normal buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_normal_id());
	glBufferData(GL_ARRAY_BUFFER,
		obj.normal_buffer.size() * sizeof(glm::vec3),
		&obj.normal_buffer[0],
		GL_STATIC_DRAW);

	//	Bind index buffer data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.get_index_id());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		obj.index_buffer.size() * sizeof(unsigned int),
		&obj.index_buffer[0],
		GL_STATIC_DRAW);
}

void delete_face(MeshObject& obj, MyMesh::FaceHandle fh)
{
	//	delete_face 함수 실행 전, status를 request 해주어야 한다고 한다.
	obj.mesh.request_face_status();
	obj.mesh.delete_face(fh);
	//	delete_face 함수의 주석에서 보이듯, 함수를 콜해도 delete할 것이라고 표시만 해놓을 뿐,
	//	실제로 삭제하기 위해서는 garbage_colleciton 함수를 콜해야 한다고 한다.
	obj.mesh.garbage_collection();

	obj.vertex_buffer.clear();
	obj.color_buffer.clear();
	obj.normal_buffer.clear();
	obj.index_buffer.clear();

	//	Resize normal buffer with mesh's number of vertices
	//std::cout << mesh.n_vertices() << std::endl;
	//normal_buffer_data_bunny.reserve(obj.mesh.n_vertices());
	//	save vertex data
	for each(MyMesh::VertexHandle vh in obj.mesh.vertices())
	{
		OpenMesh::Vec3f p, n;
		p = obj.mesh.point(vh);
		obj.vertex_buffer.push_back(glm::vec3(p[0], p[1], p[2]));
		//std::cout << p << std::endl;

		//	save normal data
		n = obj.mesh.normal(vh);
		obj.normal_buffer.push_back(glm::vec3(n[0], n[1], n[2]));
		//std::cout << n << std::endl;

		obj.mesh.set_color(vh, f_to_uc(0, 0.8, 0));
		OpenMesh::Vec3f color = uc_to_f(obj.mesh.color(vh));
		obj.color_buffer.push_back(glm::vec3(color[0], color[1], color[2]));
	}
	//	save index data
	for each (MyMesh::FaceHandle fh in obj.mesh.faces())
	{
		MyMesh::ConstFaceVertexIter cfvIt;
		for (cfvIt = obj.mesh.cfv_begin(fh); cfvIt.is_valid(); ++cfvIt)
		{
			obj.index_buffer.push_back(cfvIt->idx());
			//std::cout << cfvlt->idx() << std::endl;
		};
	}

	//	Bind vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_vertex_id());
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * obj.vertex_buffer.size(),
		&obj.vertex_buffer[0],
		GL_STATIC_DRAW);

	//	Bind color buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_color_id());
	glBufferData(GL_ARRAY_BUFFER,
		obj.color_buffer.size() * sizeof(glm::vec3),
		&obj.color_buffer[0],
		GL_STATIC_DRAW);

	//	Bind normal buffer data
	glBindBuffer(GL_ARRAY_BUFFER, obj.get_normal_id());
	glBufferData(GL_ARRAY_BUFFER,
		obj.normal_buffer.size() * sizeof(glm::vec3),
		&obj.normal_buffer[0],
		GL_STATIC_DRAW);

	//	Bind index buffer data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.get_index_id());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		obj.index_buffer.size() * sizeof(unsigned int),
		&obj.index_buffer[0],
		GL_STATIC_DRAW);
}

void bfs_and_note(MeshObject* obj, MyMesh::FaceHandle fh)
{
	MyMesh::ConstFaceVertexIter cfvIt;
	for (cfvIt = obj->mesh.cfv_begin(fh); cfvIt.is_valid(); ++cfvIt)
	{
		obj->mesh.set_color(cfvIt.handle(), f_to_uc(1, 0, 0));
		obj->color_buffer[cfvIt.handle().idx()] = glm::vec3(1, 0, 0);
		/*OpenMesh::Vec3f point = mesh.point(cfvIt.handle());
		mesh.set_point(cfvIt.handle(), point * 1.2f);*/

		if (is_selected_vertices)
		{
			change_color(*obj, selected_vertices, glm::vec3(0, 0.8, 0));
		}
		selected_vertices = breadth_first_search(*obj, cfvIt.handle(), 3);
		is_selected_vertices = true;
		change_color(*obj, selected_vertices, glm::vec3(1, 0, 0));
		glBindBuffer(GL_ARRAY_BUFFER, obj->get_color_id());
		glBufferData(GL_ARRAY_BUFFER, obj->color_buffer.size() * sizeof(glm::vec3), &obj->color_buffer[0], GL_STATIC_DRAW);
	}
}

void bfs_and_note(MeshObject* obj, MyMesh::VertexHandle vh)
{
	obj->mesh.set_color(vh, f_to_uc(1, 0, 0));
	obj->color_buffer[vh.idx()] = glm::vec3(1, 0, 0);

	if (is_selected_vertices)
	{
		change_color(*obj, selected_vertices, glm::vec3(0, 0.8, 0));
	}
	selected_vertices = breadth_first_search(*obj, vh, 3);
	is_selected_vertices = true;
	change_color(*obj, selected_vertices, glm::vec3(1, 0, 0));
	glBindBuffer(GL_ARRAY_BUFFER, obj->get_color_id());
	glBufferData(GL_ARRAY_BUFFER, obj->color_buffer.size() * sizeof(glm::vec3), &obj->color_buffer[0], GL_STATIC_DRAW);
}

std::vector<MyMesh::VertexHandle> breadth_first_search(MeshObject& obj, MyMesh::VertexHandle start_vh, unsigned int depth)
{
	std::vector<MyMesh::VertexHandle> det;

	//	breadth first search를 위한 deque
	std::deque<MyMesh::VertexHandle> q;

	//	시작 vertex 넣음
	q.push_back(start_vh);
	//	Search중 depth 검사를 위해 임의의 null값을 삽입
	//	deque를 pop하는 중 null이 나오는 경우: 다음 depth로 넘어갔음을 알림
	MyMesh::VertexHandle null;
	q.push_back(null);
	//	시작 vertex의 color 변경
	det.push_back(start_vh);
	obj.color_buffer[start_vh.idx()] = glm::vec3(1, 0, 0);

	while (!q.empty())
	{
		if (depth <= 0)
			break;

		MyMesh::VertexHandle vhi = q.front();	q.pop_front();
		if (!vhi.is_valid())
		{
			depth--;
			MyMesh::VertexHandle null;
			q.push_back(null);
			continue;
		}
		for (MyMesh::VOHIter vohIt = obj.mesh.voh_iter(vhi); vohIt.is_valid(); vohIt++)
		{
			MyMesh::VertexHandle vhj = obj.mesh.to_vertex_handle(*vohIt);
			det.push_back(vhj);
			obj.color_buffer[vhj.idx()] = glm::vec3(1, 0, 0);
			q.push_back(vhj);
		}
	}

	return det;
}

int get_euler_characteristc(MyMesh mesh)
{
	int nv, ne, nf;
	nv = mesh.n_vertices();
	ne = mesh.n_edges();
	nf = mesh.n_faces();
	
	return nv - ne + nf;
}

bool is_selected_vertices;
std::vector<MyMesh::VertexHandle> selected_vertices;