#include "mesh_modifier.h"
#include "freetype.h"

#include <direct.h>
GLFWwindow* window;

std::vector<glm::vec3> vertex_buffer_data_temp;
std::vector<glm::vec3> color_buffer_data_temp;
std::vector<glm::vec3> normal_buffer_data_temp;
std::vector<unsigned int> index_buffer_data_temp;
MyMesh mesh_temp;

object_manager obj_man;

std::vector<GLuint> vertex_buffer;
//GLuint vertex_buffer[4];
GLuint color_buffer[4];
GLuint index_buffer;
GLuint normal_buffer;

void drawTriangle();
void drawCube(glm::vec3 pos);
void drawBunny(bool is_wire);

GLuint programID_normal;
GLuint programID_phong;
bool is_phong_or_normal = false;
shader_info curr_shader;

//	Light
glm::vec3 light_pos = glm::vec3(2, 2, 2);

//	interface variables
double mouse_old_xpos, mouse_old_ypos;
float speed = 0.01f; // 3 units / second
float mouseSpeed = 0.003f;
bool is_leftmouse_down = false;
bool is_vertex_or_face = false;
bool is_delete_or_search = false;

int euler_characteristic = 0;	//Euler characteristic(V - E + F)

double lastTime = glfwGetTime();
//

//	OpenMesh
//MyMesh* myMesh;

void renderScene();
void render_objects();
void render_UI();
void init();
void computeMatrices();
void computeMatricesFromInputs();
void mouse_input();
void window_size_callback(GLFWwindow* window, int _width, int _height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

glm::vec3 bunny_pos;

void mouseScroll(GLFWwindow* window, double xpos, double ypos);
std::string vec3_toString(const glm::vec3 v)
{
	std::string s = "(" + std::to_string(v.x);
	s += ", " + std::to_string(v.y);
	s += ", " + std::to_string(v.z) + ")";
	return s;
}

MyMesh readOffModel(std::string filePath, std::vector<glm::vec3> &vertex_buffer_data,
	std::vector<glm::vec3> &color_buffer_data,
	std::vector<glm::vec3> &normal_buffer_data,
	std::vector<unsigned int> &index_buffer_data, float scale)
{
	MyMesh mesh;
	OpenMesh::IO::Options readOptions;
	readOptions += OpenMesh::IO::Options::VertexNormal;
	readOptions += OpenMesh::IO::Options::VertexColor;
	readOptions += OpenMesh::IO::Options::FaceColor;
	if (!OpenMesh::IO::read_mesh(mesh, filePath, readOptions))
	{
		std::cerr << "Error loading mesh from file " << filePath << std::endl;
	}
	//	vertex normal을 구하기 위해서는 우선 face normal을 구해야만 한다.
	//	face normal을 사용하기 위해서는 mesh에서 request_face_normal 함수를 콜해서
	//	face normal을 사용함을 알려야 한다. 그 후 vertex normal에 대해 request를 해야 한다.
	//	필요한 request를 끝낸 후, update_normals 함수를 통해 face, vertex normal을 연산, 저장한다.
	if (!readOptions.check(OpenMesh::IO::Options::VertexNormal))
	{
		mesh.request_face_normals();
		mesh.request_vertex_normals();
		mesh.update_normals();
	}
	if (!readOptions.check(OpenMesh::IO::Options::FaceColor))
	{
		mesh.request_vertex_colors();
		mesh.request_face_colors();
	}

	//	Resize normal buffer with mesh's number of vertices
	//std::cout << mesh.n_vertices() << std::endl;
	normal_buffer_data.reserve(mesh.n_vertices());
	//	save vertex data
	for each(MyMesh::VertexHandle vh in mesh.vertices())
	{
		OpenMesh::Vec3f p, n;
		p = mesh.point(vh) * scale;
		mesh.set_point(vh, p);
		vertex_buffer_data.push_back(glm::vec3(p[0], p[1], p[2]));
		//std::cout << p << std::endl;

		//	save normal data
		n = mesh.normal(vh);
		normal_buffer_data.push_back(glm::vec3(n[0], n[1], n[2]));
		//std::cout << n << std::endl;

		mesh.set_color(vh, f_to_uc(0, 0.8, 0));
		OpenMesh::Vec3f color = uc_to_f(mesh.color(vh));
		color_buffer_data.push_back(glm::vec3(color[0], color[1], color[2]));
	}
	//	save index data
	for each (MyMesh::FaceHandle fh in mesh.faces())
	{
		MyMesh::ConstFaceVertexIter cfvIt;
		for (cfvIt = mesh.cfv_begin(fh); cfvIt.is_valid(); ++cfvIt)
		{
			index_buffer_data.push_back(cfvIt->idx());
			//std::cout << cfvlt->idx() << std::endl;
		};
	}

	std::cout << "Number of vertices: " << mesh.n_vertices() << std::endl;
	std::cout << "Number of edges: " << mesh.n_edges() << std::endl;
	std::cout << "Number of triangles: " << mesh.n_faces() << std::endl;
	euler_characteristic = get_euler_characteristc(mesh);

	return mesh;
}

int main()
{
	//	OpenMesh initialize
	char* c_curDirPath = (char*)malloc(sizeof(char) * 1000);
	_getcwd(c_curDirPath, 1000);
	std::string input_mesh; float mesh_scale;
	std::cout << "Input mesh file name:";	std::cin >> input_mesh;
	std::cout << "Input mesh scale:";		std::cin >> mesh_scale;
	std::string curDirPath = c_curDirPath + std::string("\\mesh_models\\" + input_mesh);
	std::cout << "Mesh: " << curDirPath.c_str() << std::endl;
	mesh_temp = readOffModel(curDirPath, vertex_buffer_data_temp,
		color_buffer_data_temp, normal_buffer_data_temp, index_buffer_data_temp, mesh_scale);

	if (!glfwInit())
	{
		fprintf(stderr, "GLFW 초기화 실패\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);	//	4x 안티에일리어싱
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	//	OpenGL 3.3 사용
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	//	MacOS를 위한 설정?
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//	고대 OpenGL 사용 X

	//	새 창을 열고, OpenGL 컨텍스트 생성
	window = glfwCreateWindow(width, height, "Mesh Viewer", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "GLFW 윈도우를 여는데 실패했습니다. Intel GPU를 사용한다면, 3.3을 지원하지 않습니다");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);	//	GLEW 초기화
	glewExperimental = true;	//	코어 프로파일을 위해 필요
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	init();

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	//	밑에서 Escape 키가 눌리는 것을 감지할 수 있도록 할 것
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	do
	{
		glfwSetScrollCallback(window, mouseScroll);
		mouse_input();
		computeMatricesFromInputs();

		glUniform3f(curr_shader.light_pos_id, light_pos.x, light_pos.y, light_pos.z);

		renderScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}	//	만약 ESC 키가 눌렸는지 혹은 창이 닫혔는지 체크
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
		&& glfwWindowShouldClose(window) == 0);
}

void init()
{
	glClearColor(0.8, 0.8, 0.8, 1);

	//	Enable depth test
	glEnable(GL_DEPTH_TEST);
	//	Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//	Reserve all buffers(100)
	obj_man.vertex_buffer_ids.reserve(100);
	obj_man.color_buffer_ids.reserve(100);
	obj_man.normal_buffer_ids.reserve(100);
	obj_man.index_buffer_ids.reserve(100);

	//	Generate vertex buffer
	add_buffer_id(3, obj_man.vertex_buffer_ids);

	//	cube
	for (int i = 0; i < sizeof(g_vertex_buffer_data) / sizeof(GLfloat); i++)
	{
		g_vertex_buffer_data[i] *= 0.01f;
	}
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	//	triangle
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_tri), g_vertex_buffer_data_tri, GL_STATIC_DRAW);

	//	bunny.off
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[2]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data_temp.size() * sizeof(glm::vec3), &vertex_buffer_data_temp[0], GL_STATIC_DRAW);

	//	Generate color buffer
	add_buffer_id(3, obj_man.color_buffer_ids);
	//	cube
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
	//	triangle
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data_tri), g_color_buffer_data_tri, GL_STATIC_DRAW);
	//	bunny
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[2]);
	glBufferData(GL_ARRAY_BUFFER, color_buffer_data_temp.size() * sizeof(glm::vec3), &color_buffer_data_temp[0], GL_STATIC_DRAW);

	//	Generate normal buffer
	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normal_buffer_data_temp.size() * sizeof(glm::vec3), &normal_buffer_data_temp[0], GL_STATIC_DRAW);

	//	Generate index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data_temp.size() * sizeof(unsigned int), &index_buffer_data_temp[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> cube_vtx;
	for (int i = 0; i < sizeof(g_vertex_buffer_data) / sizeof(GLfloat); i += 3)
	{
		cube_vtx.push_back(glm::vec3(g_vertex_buffer_data[i],
			g_vertex_buffer_data[i + 1],
			g_vertex_buffer_data[i + 2]));
	}
	std::vector<glm::vec3> cube_color;
	for (int i = 0; i < sizeof(g_color_buffer_data) / sizeof(GLfloat); i += 3)
	{
		cube_color.push_back(glm::vec3(g_color_buffer_data[i],
			g_color_buffer_data[i + 1],
			g_color_buffer_data[i + 2]));
	}

	/*for (int i = 0; i < 5; i++)
	{
		MeshObject* obj = new MeshObject(obj_man, vertex_buffer_data_temp,
			color_buffer_data_temp,
			normal_buffer_data_temp,
			index_buffer_data_temp,
			mesh_temp);
		obj_man.objects.push_back(obj);
	}*/

	MeshObject* obj = new MeshObject(obj_man, vertex_buffer_data_temp,
		color_buffer_data_temp,
		normal_buffer_data_temp,
		index_buffer_data_temp,
		mesh_temp);
	obj_man.objects.push_back(obj);

	programID_phong = LoadShaders("PhongVert.vert", "PhongFrag.frag");
	glUseProgram(programID_phong);

	object_manager::phong_shader.mvp_id = glGetUniformLocation(programID_phong, "MVP");
	object_manager::phong_shader.model_id = glGetUniformLocation(programID_phong, "M");
	object_manager::phong_shader.view_id = glGetUniformLocation(programID_phong, "V");
	object_manager::phong_shader.light_pos_id = glGetUniformLocation(programID_phong, "LightPosition_worldspace");
	object_manager::phong_shader.is_wire_id = glGetUniformLocation(programID_phong, "is_wire");

	programID_normal = LoadShaders("SimpleVert.vert", "SimpleFrag.frag");
	glUseProgram(programID_normal);

	object_manager::normal_shader.mvp_id = glGetUniformLocation(programID_normal, "MVP");
	object_manager::normal_shader.model_id = glGetUniformLocation(programID_normal, "M");
	object_manager::normal_shader.view_id = glGetUniformLocation(programID_normal, "V");
	object_manager::normal_shader.light_pos_id = glGetUniformLocation(programID_normal, "LightPosition_worldspace");
	object_manager::normal_shader.is_wire_id = glGetUniformLocation(programID_normal, "is_wire");

	curr_shader = object_manager::normal_shader;

	bunny_pos = glm::vec3(0, 0, 0);

	//	font
	font_manager::load_font((GLfloat)width, (GLfloat)height);
}

void render_objects()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glShadeModel(GL_FLAT);
	if (is_phong_or_normal)
	{
		glUseProgram(programID_phong);
	}
	else
	{
		glUseProgram(programID_normal);
	}

	/*for (int i = 0; i < obj_man.objects.size(); i++)
	{
		obj_man.objects[i]->draw_object(glm::vec3(i*1.5 - 2, 0, 0), false);
		obj_man.objects[i]->draw_object(glm::vec3(i*1.5 - 2, 0, 0), true);
	}*/

	if (!is_phong_or_normal)
	{
		obj_man.objects[0]->draw_object(glm::vec3(0, 0, 0), true, curr_shader);
	}
	obj_man.objects[0]->draw_object(glm::vec3(0, 0, 0), false, curr_shader);
}

void render_UI()
{
	font_manager::render_text("Current euler characteristic: " + std::to_string(euler_characteristic), 20.0f, 20.0f, 0.5f, glm::vec3(0.2, 0.2, 0.2));

	float s_h_interval = 18.0f;// *((float)height / (float)init_height);

	//	Key explain
	font_manager::render_text("Face Selection: F", 12.0f, init_height - s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Vertex Selection: V", 12.0f, init_height - 2 * s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Delete : C", 12.0f, init_height - 3 * s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Breadth first search : B", 12.0f, init_height - 4 * s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Turn on/off phong shading : P", 12.0f, init_height - 5 * s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Camera moving : Press right mouse button + (WASD, QE)", 12.0f, init_height - 6.3 * s_h_interval, 0.3f, glm::vec3(0.2, 0.2, 0.2));

	std::string curr_select_mode;
	if (is_vertex_or_face)
		curr_select_mode = "Vertex";
	else
		curr_select_mode = "Face";

	std::string curr_inst_mode;
	if (is_delete_or_search)
		curr_inst_mode = "Delete";
	else
		curr_inst_mode = "Search";

	font_manager::render_text("Current select mode: " + curr_select_mode, width - 220.0f, init_height - s_h_interval, 0.33f, glm::vec3(0.2, 0.2, 0.2));
	font_manager::render_text("Current instruction mode: " + curr_inst_mode, width - 220.0f, init_height - 2 * s_h_interval, 0.33f, glm::vec3(0.2, 0.2, 0.2));

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_objects();
	if (!is_phong_or_normal)
	{
		render_UI();
	}
}

void mouse_input()
{
	if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		&& !is_leftmouse_down)
	{
		MeshObject* obj = ray_casting(window, obj_man.objects, is_vertex_or_face, is_delete_or_search);
		if(obj != NULL)
			euler_characteristic = get_euler_characteristc(obj->mesh);
		is_leftmouse_down = true;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		is_leftmouse_down = false;
	}
}

void computeMatrices()
{
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector : perpendicular to both direction and right
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera_pos += direction  * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera_pos -= direction  * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera_pos += right  * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera_pos -= right  * speed;
	}
	//	Up
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera_pos += up * speed;
	}
	//	Down
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera_pos -= up * speed;
	}

	object_manager::projection_matrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	object_manager::view_matrix = glm::lookAt(
		camera_pos,	//	Camera camera_pos
		camera_pos + direction,	//	look here
		up
	);

	//	Send to the currently bound shader, in the "V" uniform
	glUniformMatrix4fv(curr_shader.view_id, 1, GL_FALSE, &object_manager::view_matrix[0][0]);
}

void computeMatricesFromInputs()
{
	//	Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		|| glm::abs(xpos) >= width || glm::abs(ypos) >= height)
	{
		//	Update previous frame's mouse position
		mouse_old_xpos = xpos;
		mouse_old_ypos = ypos;
		return;
	}

	//	Reset mouse position for next frame
	//glfwSetCursorPos(window, width / 2, height / 2);

	/*double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);*/

	//	Compute new orientation
	horizontalAngle += mouseSpeed  * float(mouse_old_xpos - xpos);
	verticalAngle += mouseSpeed  * float(mouse_old_ypos - ypos);

	computeMatrices();

	lastTime = glfwGetTime();

	//	Update previous frame's mouse position
	mouse_old_xpos = xpos;
	mouse_old_ypos = ypos;
}

void mouseScroll(GLFWwindow* window, double xpos, double ypos)
{
	//FoV = initialFoV - 5 * ypos;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			//	Vertex select mode
		case GLFW_KEY_V:
			is_vertex_or_face = true;
			break;
			//	Face select mode
		case GLFW_KEY_F:
			is_vertex_or_face = false;
			break;
			//	Delete mode(Not D!)
		case GLFW_KEY_C:
			is_delete_or_search = true;
			break;
			//	Breadth first search mode
		case GLFW_KEY_B:
			is_delete_or_search = false;
			break;
		case GLFW_KEY_P:
			is_phong_or_normal = !is_phong_or_normal;
			if (is_phong_or_normal)
			{
				curr_shader = object_manager::phong_shader;
			}
			else
			{
				curr_shader = object_manager::normal_shader;
			}
			break;
		}
	}
}

void drawBunny(bool is_wire = false)
{
	//	model_matrix matrix
	glm::mat4 model_matrix = glm::translate(glm::mat4(), bunny_pos);
	//model_matrix = glm::mat4(1.0f);
	//	Send to the currently bound shader, in the "M" uniform
	glUniformMatrix4fv(curr_shader.model_id, 1, GL_FALSE, &model_matrix[0][0]);

	//	MVP matrix
	glm::mat4 mvp_matrix = object_manager::projection_matrix * object_manager::view_matrix * model_matrix; // * position
	//	Send to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(curr_shader.mvp_id, 1, GL_FALSE, &mvp_matrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (is_wire)
	{
		glUniform1i(curr_shader.is_wire_id, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		{
			glDrawElements(GL_TRIANGLES, index_buffer_data_temp.size(), GL_UNSIGNED_INT, (void*)0);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glUniform1i(curr_shader.is_wire_id, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glPolygonOffset(1, 1);
		glEnable(GL_POLYGON_OFFSET_FILL);
		{
			glDrawElements(GL_TRIANGLES, index_buffer_data_temp.size(), GL_UNSIGNED_INT, (void*)0);
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void drawCube(glm::vec3 pos)
{
	//	model_matrix matrix
	glm::mat4 model_matrix = glm::translate(glm::mat4(), pos);
	//	Send to the currently bound shader, in the "M" uniform
	glUniformMatrix4fv(curr_shader.model_id, 1, GL_FALSE, &model_matrix[0][0]);

	//	MVP matrix
	glm::mat4 mvp = object_manager::projection_matrix * object_manager::view_matrix * model_matrix; // * position
	//	Send to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(curr_shader.mvp_id, 1, GL_FALSE, &mvp[0][0]);

	// 1nd attribute buffer : vertice
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[0]);
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
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[0]);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
}

void drawTriangle()
{
	//	model_matrix matrix for triangle
	glm::mat4 model_matrix = glm::translate(glm::mat4(), glm::vec3(1, 0, 0));
	//	MVP matrix for triangle
	glm::mat4 mvp = object_manager::projection_matrix * object_manager::view_matrix * model_matrix; // * position
												 //	Send to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(curr_shader.mvp_id, 1, GL_FALSE, &mvp[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.vertex_buffer_ids[1]);
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
	glBindBuffer(GL_ARRAY_BUFFER, obj_man.color_buffer_ids[1]);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}