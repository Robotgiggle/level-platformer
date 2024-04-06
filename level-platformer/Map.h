#pragma once
#define LOG(argument) std::cout << argument << '\n'
#include <vector>

class Map {
private:
    int m_width;
    int m_height;

    // Here, the level_data is the numerical "drawing" of the map
    const int* m_level_data;
    GLuint     m_texture_id;

    float m_tile_size;
    int   m_tilemap_width;
    int   m_tilemap_height;

    // Just like with rendering text, we're rendering several sprites at once
    // So we need vectors to store their respective vertices and texture coordinates
    std::vector<float> m_vertices;
    std::vector<float> m_texture_coordinates;

    // The boundaries of the map
    float m_left_bound, m_right_bound, m_top_bound, m_bottom_bound;

public:
    // Constructor
    Map(const int width, const int height, const int* level_data, GLuint texture_id, float tile_size, int
        tilemap_width, int tilemap_height);

    // Methods
    void build();
    void render(ShaderProgram* program);
    bool is_solid(glm::vec3 position);
    bool is_solid(glm::vec3 position, float* penetration_x, float* penetration_y);
    bool is_solid(glm::vec3 position, float* penetration_x, float* penetration_y, glm::vec3* tile_pos);

    // Getters
    int const get_width()  const { return m_width; }
    int const get_height() const { return m_height; }

    const int* const get_level_data() const { return m_level_data; }
    GLuint     const get_texture_id() const { return m_texture_id; }

    float const get_tile_size()    const { return m_tile_size; }
    int   const get_tilemap_width() const { return m_tilemap_width; }
    int   const get_tilemap_height() const { return m_tilemap_height; }

    std::vector<float> const get_vertices()            const { return m_vertices; }
    std::vector<float> const get_texture_coordinates() const { return m_texture_coordinates; }

    float const get_left_bound()   const { return m_left_bound; }
    float const get_right_bound()  const { return m_right_bound; }
    float const get_top_bound()    const { return m_top_bound; }
    float const get_bottom_bound() const { return m_bottom_bound; }
};