#pragma once

namespace Utility {
	GLuint load_texture(const char* filepath);
	void draw_text(ShaderProgram* program, GLuint font_texture_id,
				   std::string text, float screen_size, float spacing, glm::vec3 position);
};