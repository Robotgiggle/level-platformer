#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Entity.h"

Entity::Entity(Scene* scene)
{
    m_scene = scene;
    
    // ––––– PHYSICS ––––– //
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // ––––– TRANSFORMATION ––––– //
    m_angle = 0;
    m_rotation = 0;
    m_movement = glm::vec3(0.0f);
    m_scale = glm::vec3(1.0f);
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    bool indices = true;
    for (int i = 0; i < 4; i++) {
        if (m_walking[i] == m_animation_indices) indices = false;
        delete[] m_walking[i];
    }
    if (indices) delete m_animation_indices;
}

void Entity::despawn() {
    m_scene->m_state.entities[m_array_index] = nullptr;
    delete this;
}

void Entity::setup_anim(int cols, int rows, int frames, int fps, int type, int index, float time) {
    m_animation_cols = cols;
    m_animation_rows = rows;

    m_animation_frames = frames;
    m_animation_index = index;
    m_animation_time = time;
    m_frames_per_second = fps;

    m_animation_type = static_cast<AnimType>(type);
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float xRadius = 0.5f * (m_sprite_scale.x / m_scale.x);
    float yRadius = 0.5f * (m_sprite_scale.y / m_scale.y);
    float vertices[] =
    {
        // bottom right triangle
        -xRadius, -yRadius,
        xRadius, -yRadius,
        xRadius, yRadius, 
        // top left triangle
        -xRadius, -yRadius,
        xRadius,  yRadius,
        -xRadius, yRadius
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::update(float delta_time, Entity* solid_entities, int solid_entity_count, Map* map)
{
    // ––––– INDEXING ––––– //
    if (m_array_index == -1) {
        for (int i = 0; i < m_scene->m_entityCap; i++) {
            if (m_scene->m_state.entities[i] == this) {
                m_array_index = i;
                break;
            }
        }
    }
    assert((m_array_index != -1));

    // ––––– BASICS ––––– //
    if (!m_is_active) return;
    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    // ––––– ANIMATION ––––– //
    if (m_animation_indices != NULL)
    {
        switch (m_animation_type) {
        case MOVING:
            if (glm::length(m_movement) == 0) {
                m_animation_index = 0;
                break;  // if movement isn't 0, fall through to ALWAYS
            }
        case ALWAYS:
            m_animation_time += delta_time;
            if (m_animation_time >= 1.0f / m_frames_per_second){
                m_animation_time = 0.0f;
                m_animation_index = (m_animation_index + 1) % m_animation_frames;
            }
            break;
        case NEVER:
            m_animation_index = 0;
        default:
            break;
        }
    }

    // ––––– MOTION ––––– //
    switch (m_motion_type) {
    case NONE:
        // completely immobile
        m_velocity = glm::vec3(0.0f);
        break;
    case TOP_DOWN:
        // controlled on both X and Y axes
        m_velocity = m_movement * m_speed;
        break;
    case SIDE_ON:
        // controlled on X axis, physics on Y axis
        m_velocity.x = m_movement.x * m_speed;
        m_velocity += m_acceleration * delta_time;
        break;
    case SIDE_ON_Y:
        // controlled on Y axis, physics on X axis
        m_velocity.y = m_movement.y * m_speed;
        m_velocity += m_acceleration * delta_time;
        break;
    case PHYSICS:
        // physics only
        m_velocity += m_acceleration * delta_time;
        break;
    default:
        break;
    }

    float leftBarrier = map->get_left_bound() + (m_scale.x / 2);
    float rightBarrier = map->get_right_bound() - (m_scale.x / 2);
    if ((m_position.x > leftBarrier or m_velocity.x > 0.0f) and (m_position.x < rightBarrier or m_velocity.x < 0.0f)) {
        m_position += m_velocity * delta_time;
    } else {
        m_position.y += m_velocity.y * delta_time;
    }

    if (m_has_collision) {
        check_solid_collision(solid_entities, solid_entity_count);
        check_solid_collision(map);
    }

    // ––––– ROTATION ––––– //
    m_angle += m_rotation * m_rot_speed * 45.0f * delta_time;

    // ––––– JUMPING ––––– //
    if (m_is_jumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        m_is_jumping = false;

        // STEP 2: The player now acquires an upward velocity
        m_velocity.y += m_jumping_power;
    }

    // ––––– TRANSFORMATIONS ––––– //
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(m_model_matrix, glm::radians(m_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);

}

void const Entity::check_solid_collision(Entity* solid_entities, int solid_entity_count)
{
    for (int i = 0; i < solid_entity_count; i++)
    {
        // STEP 1: For every entity that our player can collide with...
        Entity* solid_entity = &solid_entities[i];

        if (check_collision(solid_entity))
        {
            // STEP 2: Calculate the distance between its centre and our centre and use 
            //         that to calculate the amount of overlap between both bodies.     
            float y_distance = fabs(m_position.y - solid_entity->m_position.y);
            float y_overlap = fabs(y_distance - (get_height() / 2.0f) - (solid_entity->get_height() / 2.0f));

            float x_distance = fabs(m_position.x - solid_entity->m_position.x);
            float x_overlap = fabs(x_distance - (get_width() / 2.0f) - (solid_entity->get_width() / 2.0f));

            // STEP 3: "Unclip" ourselves from the other entity, and zero our velocity.
            if (m_velocity.y > 0 and y_overlap < x_overlap) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;
                m_collided_top = true;
            } else if (m_velocity.y < 0 and y_overlap < x_overlap) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_collided_bottom = true;
            } else if (m_velocity.x > 0 and x_overlap < y_overlap) {
                m_position.x -= x_overlap;
                m_velocity.x = 0;
                m_collided_right = true;
            } else if (m_velocity.x < 0 and x_overlap < y_overlap) {
                m_position.x += x_overlap;
                m_velocity.x = 0;
                m_collided_left = true;
            }
        }
    }
}

void const Entity::check_solid_collision(Map* map) {
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_scale.y / 2), 0.0f);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_scale.x / 2), m_position.y + (m_scale.y / 2), 0.0f);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_scale.x / 2), m_position.y + (m_scale.y / 2), 0.0f);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_scale.y / 2), 0.0f);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_scale.x / 2), m_position.y - (m_scale.y / 2), 0.0f);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_scale.x / 2), m_position.y - (m_scale.y / 2), 0.0f);

    // Probes for tiles left and right
    glm::vec3 left = glm::vec3(m_position.x - (m_scale.x / 2), m_position.y, 0.0f);
    glm::vec3 right = glm::vec3(m_position.x + (m_scale.x / 2), m_position.y, 0.0f);

    float penetration_x = 0;
    float penetration_y = 0;
    glm::vec3 tile_pos;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    } else if (map->is_solid(top_left, &penetration_x, &penetration_y, &tile_pos) && m_velocity.y > 0) {
        if (penetration_y < penetration_x && !map->is_solid(left)) {
            m_position.y -= penetration_y;
            m_velocity.y = 0;
            m_collided_top = true;
        } else {
            m_position.x += penetration_x;
            m_velocity.x = 0;
            m_collided_left = true;
        }
    } else if (map->is_solid(top_right, &penetration_x, &penetration_y, &tile_pos) && m_velocity.y > 0) {
        if (penetration_y < penetration_x && !map->is_solid(right)) {
            m_position.y -= penetration_y;
            m_velocity.y = 0;
            m_collided_top = true;
        } else {
            m_position.x -= penetration_x;
            m_velocity.x = 0;
            m_collided_right = true;
        }
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    } else if (map->is_solid(bottom_left, &penetration_x, &penetration_y, &tile_pos) && m_velocity.y < 0) {
        if (penetration_y < penetration_x && !map->is_solid(left)) {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
        } else {
            m_position.x += penetration_x;
            m_velocity.x = 0;
            m_collided_left = true;
        }
    } else if (map->is_solid(bottom_right, &penetration_x, &penetration_y, &tile_pos) && m_velocity.y < 0) {
        if (penetration_y < penetration_x && !map->is_solid(right)) {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
        } else {
            m_position.x -= penetration_x;
            m_velocity.x = 0;
            m_collided_right = true;
        }
    }

    // And the two points on either side
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0) {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    } else if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0) {
        m_position.x -= penetration_x;
        m_velocity.x = -m_velocity.x;
        m_collided_right = true;
    }
}

void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) return;

    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float xRadius = 0.5f * (m_sprite_scale.x / m_scale.x);
    float yRadius = 0.5f * (m_sprite_scale.y / m_scale.y);
    float vertices[] =
    {
        // bottom right triangle
        -xRadius, -yRadius,
        xRadius, -yRadius,
        xRadius, yRadius,
        // top left triangle
        -xRadius, -yRadius,
        xRadius,  yRadius,
        -xRadius, yRadius
    };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_scale.x + other->m_scale.x) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_scale.y + other->m_scale.y) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}
