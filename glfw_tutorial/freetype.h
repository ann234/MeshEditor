#pragma once

#include <map>
#include <iostream>

#include <gl\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <freetype\ft2build.h>
#include FT_FREETYPE_H

#pragma comment(lib, "freetype.lib")

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

class font_manager
{
public:
	static void font_manager::load_font(GLfloat width, GLfloat height);
	static void font_manager::render_text(std::string text, GLuint x, GLuint y, GLfloat scale, glm::vec3 color);

	static std::map<GLchar, Character> Characters;
	static GLuint vbo;
	static GLuint shader_id, projection_id, text_id, textColor_id;
	static glm::mat4 projection;
private:

};