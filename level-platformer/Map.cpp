#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <math.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Map.h"

Map::Map(const int width, const int height, const int* level_data, GLuint texture_id, float tile_size, int tilemap_width, int tilemap_height)
{
    m_width = width;
    m_height = height;

    m_level_data = level_data;
    m_texture_id = texture_id;

    m_tile_size = tile_size;
    m_tilemap_width = tilemap_width;
    m_tilemap_height = tilemap_height;

    build();
}

void Map::build()
{
    // useful intermediate values
    float tile_width = 1.0f / (float)m_tilemap_width;
    float tile_height = 1.0f / (float)m_tilemap_height;

    float x_offset = -(m_tile_size / 2); // From center of tile
    float y_offset = m_height - (m_tile_size / 2); // From center of tile

    // Since this is a 2D map, we need a nested for-loop
    for (int y_coord = 0; y_coord < m_height; y_coord++)
    {
        for (int x_coord = 0; x_coord < m_width; x_coord++)
        {
            // Get the current tile
            int tile = m_level_data[y_coord * m_width + x_coord];

            // If the tile number is 0 i.e. not solid, skip to the next one
            if (tile == 0) continue;

            // Otherwise, calculate its UV-coordinated
            float u_coord = (float)(tile % m_tilemap_width) / (float)m_tilemap_width;
            float v_coord = (float)(tile / m_tilemap_width) / (float)m_tilemap_height;

            // So we can store them inside our std::vectors
            m_vertices.insert(m_vertices.end(), {
                x_offset + (m_tile_size * x_coord),  y_offset + -m_tile_size * y_coord,
                x_offset + (m_tile_size * x_coord),  y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord), y_offset + -m_tile_size * y_coord,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset + (-m_tile_size * y_coord) - m_tile_size,
                x_offset + (m_tile_size * x_coord) + m_tile_size, y_offset + -m_tile_size * y_coord
                });

            m_texture_coordinates.insert(m_texture_coordinates.end(), {
                u_coord, v_coord,
                u_coord, v_coord + (tile_height),
                u_coord + tile_width, v_coord + (tile_height),
                u_coord, v_coord,
                u_coord + tile_width, v_coord + (tile_height),
                u_coord + tile_width, v_coord
                });
        }
    }

    // The bounds are dependent on the size of the tiles
    m_left_bound = -(m_tile_size / 2);
    m_right_bound = (m_tile_size * m_width) - (m_tile_size / 2);
    m_top_bound = (m_tile_size * m_height) - (m_tile_size / 2); 
    m_bottom_bound = -(m_tile_size / 2);
}

void Map::render(ShaderProgram* program)
{
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, m_vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, m_texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glDrawArrays(GL_TRIANGLES, 0, (int)m_vertices.size() / 2);
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_position_attribute());
}

bool Map::is_solid(glm::vec3 position, float* penetration_x, float* penetration_y, glm::vec3* tile_pos)
{
    // The penetration between the map and the object
    // The reason why these are pointers is because we want to reassign values
    // to them in case that we are colliding. That way the object that originally
    // passed them as values will keep track of these distances
    // inb4: we're passing by reference
    if (penetration_x) *penetration_x = 0;
    if (penetration_y) *penetration_y = 0;

    // If we are out of bounds, it is not solid
    if (position.x < m_left_bound || position.x > m_right_bound) { return false; }
    if (position.y > m_top_bound || position.y < m_bottom_bound) { return false; }

    int tile_x = floor((position.x + (m_tile_size / 2)) / m_tile_size);
    int tile_y = floor((position.y + (m_tile_size / 2)) / m_tile_size);
    if (tile_pos) *tile_pos = glm::vec3(tile_x, tile_y, 0.0f);

    // If the tile index is negative or greater than the dimensions, it is not solid
    if (tile_x < 0 || tile_x >= m_width)  { return false; }
    if (tile_y < 0 || tile_y >= m_height) { return false; }

    // If the tile index is 0 i.e. an open space, it is not solid
    int tile = m_level_data[(m_height - tile_y - 1) * m_width + tile_x];
    if (tile == 0) { return false; }

    // And we likely have some overlap
    float tile_center_x = (tile_x * m_tile_size);
    float tile_center_y = (tile_y * m_tile_size);

    // And because we likely have some overlap, we adjust for that
    if (penetration_x) *penetration_x = (m_tile_size / 2) - fabs(position.x - tile_center_x);
    if (penetration_y) *penetration_y = (m_tile_size / 2) - fabs(position.y - tile_center_y);

    return true;
}

bool Map::is_solid(glm::vec3 position, float* penetration_x, float* penetration_y) {
    return Map::is_solid(position, penetration_x, penetration_y, nullptr);
}

bool Map::is_solid(glm::vec3 position) {
    return Map::is_solid(position, nullptr, nullptr, nullptr);
}