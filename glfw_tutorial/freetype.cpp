#include "freetype.h"
#include "glfw_core.h"

void font_manager::load_font(GLfloat width, GLfloat height)
{
	shader_id = LoadShaders("FreeType.vert", "FreeType.frag");
	glUseProgram(shader_id);
	projection_id = glGetUniformLocation(shader_id, "projection");
	text_id = glGetUniformLocation(shader_id, "text");
	textColor_id = glGetUniformLocation(shader_id, "textColor");

	projection = glm::ortho(0.0f, width, 0.0f, height);

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	//	Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	//	Configure VAO/VBO for texture quads
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void font_manager::render_text(std::string text, GLuint x, GLuint y, GLfloat scale, glm::vec3 color)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(shader_id);

	glUniformMatrix4fv(projection_id, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(textColor_id, color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);

	//	Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;

		//	Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,		ypos + h,	0.0, 0.0},
			{ xpos,		ypos,		0.0, 1.0 },
			{ xpos + w,	ypos,		1.0, 1.0 },

			{ xpos,		ypos + h,	0.0, 0.0 },
			{ xpos + w,	ypos,		1.0, 1.0 },
			{ xpos + w,	ypos + h,	1.0, 0.0 },
		};
		//	Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		//	Update content of VBO memory
		glEnableVertexAttribArray(0);
		//	같은 vbo를 반복해서 사용하는 것은 그리는 vertices들의 크기가 고정되어 있어서 buffer 크기가 불변하므로??
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//	Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//	Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale;	//	Bitshift by  to get value in pixels
	}
	//glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}



GLuint font_manager::vbo = 0;
GLuint font_manager::shader_id = 0; 
GLuint font_manager::projection_id = 0;
GLuint font_manager::text_id = 0;
GLuint font_manager::textColor_id = 0;
glm::mat4 font_manager::projection = glm::mat4();
std::map<GLchar, Character> font_manager::Characters;