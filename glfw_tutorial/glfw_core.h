#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <gl\glew.h>
#include <GLFW\glfw3.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;

extern int init_width;
extern int init_height;
extern int width;
extern int height;

//	Viewing variables

// position
extern glm::vec3 camera_pos;
// horizontal angle : toward -Z
extern float horizontalAngle;
// vertical angle : 0, look at the horizon
extern float verticalAngle;
// Initial Field of View
extern float initialFoV;
//	Current Field of View
extern float FoV;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);