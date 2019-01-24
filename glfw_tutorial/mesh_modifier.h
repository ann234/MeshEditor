#pragma once
#include "Object.h"

glm::vec3 get_ray_by_window(GLfloat width, GLfloat height, const float xpos, const float ypos);
MeshObject* ray_casting(GLFWwindow* window, std::vector<MeshObject*> objects, bool is_vertex_or_face, bool is_delete_or_search);
void delete_vertex(MeshObject& obj, MyMesh::VertexHandle& vh);
void delete_face(MeshObject& obj, MyMesh::FaceHandle fh);
void bfs_and_note(MeshObject* obj, MyMesh::FaceHandle fh);
void bfs_and_note(MeshObject* obj, MyMesh::VertexHandle fh);
std::vector<MyMesh::VertexHandle> breadth_first_search(MeshObject& obj, MyMesh::VertexHandle start_vh, unsigned int depth);
void change_color(MeshObject& obj, std::vector<MyMesh::VertexHandle> vertices, glm::vec3 color);
int get_euler_characteristc(MyMesh obj);

extern bool is_selected_vertices;
extern std::vector<MyMesh::VertexHandle> selected_vertices;