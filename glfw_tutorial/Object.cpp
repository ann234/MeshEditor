#include "Object.h"

MeshObject::MeshObject(object_manager& _obj_man, const glm::vec3* vertices,
	glm::vec3* colors,
	glm::vec3* normals,
	const unsigned int* indices)
{
	//	Generate vertex buffer and store ida
	add_buffer_id(1, _obj_man.vertex_buffer_ids);
	vertex_id = _obj_man.vertex_buffer_ids[_obj_man.vertex_buffer_ids.size() - 1];

	//	Store vertices data
	for (int i = 0; i < 3; i++)
	{
		vertex_buffer.push_back(vertices[i]);
	}
	//	Bind vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
	glBufferData(GL_ARRAY_BUFFER, 
		sizeof(glm::vec3) * vertex_buffer.size(), 
		&vertex_buffer[0], 
		GL_STATIC_DRAW);


	//	Generate color buffer and store id
	add_buffer_id(1, _obj_man.color_buffer_ids);
	color_id = _obj_man.color_buffer_ids[_obj_man.color_buffer_ids.size() - 1];

	//	Store color data. if NULL, (0, 0, 0)
	if (colors == NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			colors[i] = glm::vec3(0, 0, 0);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		color_buffer.push_back(colors[i]);
	}
	//	Bind color buffer data
	glBindBuffer(GL_ARRAY_BUFFER, color_id);
	glBufferData(GL_ARRAY_BUFFER,
		color_buffer.size() * sizeof(glm::vec3),
		&color_buffer[0],
		GL_STATIC_DRAW);


	//	Generate normal buffer if exists
	if (normals != NULL)
	{
		//	Generate normal buffer and store id
		add_buffer_id(1, _obj_man.normal_buffer_ids);
		normal_id = _obj_man.normal_buffer_ids[_obj_man.normal_buffer_ids.size() - 1];

		//	Store normal data.
		for (int i = 0; i < sizeof(normals) / sizeof(glm::vec3); i++)
		{
			normal_buffer.push_back(normals[i]);
		}
		//	Bind normal buffer data
		glBindBuffer(GL_ARRAY_BUFFER, normal_id);
		glBufferData(GL_ARRAY_BUFFER,
			normal_buffer.size() * sizeof(glm::vec3),
			&normal_buffer[0],
			GL_STATIC_DRAW);

		is_normal = true;
	}
	else
		is_normal = false;


	//	Generate index buffer if exists
	if (indices != NULL)
	{
		//	Generate index buffer and store id
		add_buffer_id(1, _obj_man.index_buffer_ids);
		index_id = _obj_man.index_buffer_ids[_obj_man.index_buffer_ids.size() - 1];

		//	Store index data.
		for (int i = 0; i < sizeof(indices) / sizeof(glm::vec3); i++)
		{
			index_buffer.push_back(indices[i]);
		}
		//	Bind index buffer data
		glBindBuffer(GL_ARRAY_BUFFER, index_id);
		glBufferData(GL_ARRAY_BUFFER,
			index_buffer.size() * sizeof(glm::vec3),
			&index_buffer[0],
			GL_STATIC_DRAW);

		is_index = true;
	}
	else
		is_index = false;

	this->obj_man = &_obj_man;
}

MeshObject::MeshObject(object_manager& _obj_man, 
	const std::vector<glm::vec3> vertices,
	std::vector<glm::vec3> colors,
	std::vector<glm::vec3> normals,
	std::vector<unsigned int> indices,
	MyMesh& mesh)
{
	//	Generate vertex buffer and store ida
	add_buffer_id(1, _obj_man.vertex_buffer_ids);
	vertex_id = _obj_man.vertex_buffer_ids[_obj_man.vertex_buffer_ids.size() - 1];

	//	Store vertices data
	for (int i = 0; i < vertices.size(); i++)
	{
		vertex_buffer.push_back(vertices[i]);
	}
	//	Bind vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * vertex_buffer.size(),
		&vertex_buffer[0],
		GL_STATIC_DRAW);


	//	Generate color buffer and store id
	add_buffer_id(1, _obj_man.color_buffer_ids);
	color_id = _obj_man.color_buffer_ids[_obj_man.color_buffer_ids.size() - 1];

	for (int i = 0; i < colors.size(); i++)
	{
		color_buffer.push_back(colors[i]);
	}
	//	Bind color buffer data
	glBindBuffer(GL_ARRAY_BUFFER, color_id);
	glBufferData(GL_ARRAY_BUFFER,
		color_buffer.size() * sizeof(glm::vec3),
		&color_buffer[0],
		GL_STATIC_DRAW);


	//	Generate normal buffer and store id
	add_buffer_id(1, _obj_man.normal_buffer_ids);
	normal_id = _obj_man.normal_buffer_ids[_obj_man.normal_buffer_ids.size() - 1];

	//	Store normal data.
	for (int i = 0; i < normals.size(); i++)
	{
		normal_buffer.push_back(normals[i]);
	}
	//	Bind normal buffer data
	glBindBuffer(GL_ARRAY_BUFFER, normal_id);
	glBufferData(GL_ARRAY_BUFFER,
		normal_buffer.size() * sizeof(glm::vec3),
		&normal_buffer[0],
		GL_STATIC_DRAW);

	is_normal = true;


	//	Generate index buffer and store id
	add_buffer_id(1, _obj_man.index_buffer_ids);
	index_id = _obj_man.index_buffer_ids[_obj_man.index_buffer_ids.size() - 1];

	//	Store index data.
	for (int i = 0; i < indices.size(); i++)
	{
		index_buffer.push_back(indices[i]);
	}
	//	Bind index buffer data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		index_buffer.size() * sizeof(unsigned int),
		&index_buffer[0],
		GL_STATIC_DRAW);

	is_index = true;

	is_mesh = true;
	MyMesh cloned_mesh = mesh;
	this->mesh = cloned_mesh;

	this->obj_man = &_obj_man;
}

MeshObject::~MeshObject()
{
	auto vb_iter = std::find(obj_man->vertex_buffer_ids.begin(), obj_man->vertex_buffer_ids.end(), vertex_id);
	if (vb_iter != obj_man->vertex_buffer_ids.end())
	{
		int idx = std::distance(obj_man->vertex_buffer_ids.begin(), vb_iter);
		glDeleteBuffers(1, &obj_man->vertex_buffer_ids[idx]);
		obj_man->vertex_buffer_ids.erase(vb_iter);
	}

	auto cb_iter = std::find(obj_man->color_buffer_ids.begin(), obj_man->color_buffer_ids.end(), color_id);
	if (cb_iter != obj_man->color_buffer_ids.end())
	{
		int idx = std::distance(obj_man->color_buffer_ids.begin(), cb_iter);
		glDeleteBuffers(1, &obj_man->color_buffer_ids[idx]);
		obj_man->color_buffer_ids.erase(cb_iter);
	}

	if (is_normal)
	{
		auto nb_iter = std::find(obj_man->normal_buffer_ids.begin(), obj_man->normal_buffer_ids.end(), normal_id);
		if (nb_iter != obj_man->normal_buffer_ids.end())
		{
			int idx = std::distance(obj_man->normal_buffer_ids.begin(), nb_iter);
			glDeleteBuffers(1, &obj_man->normal_buffer_ids[idx]);
			obj_man->normal_buffer_ids.erase(nb_iter);
		}
	}
	
	if (is_index)
	{
		auto ib_iter = std::find(obj_man->index_buffer_ids.begin(), obj_man->index_buffer_ids.end(), index_id);
		if (ib_iter != obj_man->index_buffer_ids.end())
		{
			int idx = std::distance(obj_man->index_buffer_ids.begin(), ib_iter);
			glDeleteBuffers(1, &obj_man->index_buffer_ids[idx]);
			obj_man->index_buffer_ids.erase(ib_iter);
		}
	}
}

void MeshObject::draw_object(glm::vec3 position, bool is_wire, shader_info shader)
{
	//	model_matrix matrix
	glm::mat4 model_matrix = glm::translate(glm::mat4(), position);
	//	Send to the currently bound shader, in the "M" uniform
	glUniformMatrix4fv(shader.model_id, 1, GL_FALSE, &model_matrix[0][0]);

	//	MVP matrix
	glm::mat4 mvp_matrix = object_manager::projection_matrix * object_manager::view_matrix * model_matrix; // * position
																	//	Send to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(shader.mvp_id, 1, GL_FALSE, &mvp_matrix[0][0]);

	// 1nd attribute buffer : vertice 
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_id);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	if (is_normal)
	{
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normal_id);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	if (is_index)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id);
		if (is_wire)
		{
			glUniform1i(shader.is_wire_id, 1);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			{
				glDrawElements(GL_TRIANGLES, index_buffer.size(), GL_UNSIGNED_INT, (void*)0);
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			glPolygonOffset(1, 1);
			glUniform1i(shader.is_wire_id, 0);
			//glEnable(GL_POLYGON_OFFSET_FILL);
			{
				glDrawElements(GL_TRIANGLES, index_buffer.size(), GL_UNSIGNED_INT, (void*)0);
			}
			//glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, vertex_buffer.size() * 3);
	}

	this->position = position;
}

void add_buffer_id(int size, std::vector<GLuint>& buffer)
{
	glGenBuffers(size, &buffer[buffer.size()]);

	std::vector<GLuint> temp;
	for (int i = 0; i < size; i++)
	{
		temp.push_back(buffer[buffer.size() + i]);
	}
	for (int i = 0; i < size; i++)
	{
		buffer.push_back(temp[i]);
	}
}

shader_info object_manager::normal_shader = shader_info();
shader_info object_manager::phong_shader = shader_info();

glm::mat4 object_manager::projection_matrix = glm::mat4();
glm::mat4 object_manager::view_matrix = glm::mat4();