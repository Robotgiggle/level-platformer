#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define NUMBER_OF_TEXTURES 1
#define LEVEL_OF_DETAIL    0
#define TEXTURE_BORDER     0
#define FONTBANK_SIZE      16
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <SDL_image.h>
#include "stb_image.h"
#include "Scene.h"
#include "Entity.h"
#include "Map.h"
#include "entities/WalkerEntity.h"
#include "entities/CrawlerEntity.h"
#include "entities/FlyerEntity.h"
#include "Utility.h"

GLuint Utility::load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void Utility::draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, 
    float screen_size, float spacing, glm::vec3 position)
{
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;


    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    for (size_t i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Utility::player_death(Entity* player, GlobalInfo* globalInfo) {
    globalInfo->lives -= 1;
    globalInfo->playerDead = true;
    globalInfo->deathTimer = 0.5f;
    globalInfo->changeScenes = true;
    if (player->get_position().y > 0.0f) {
        globalInfo->deathTimer = 2.5f;
        player->set_collision(false);
        player->set_velocity(glm::vec3(0.0f, 5.5f, 0.0f));
        player->set_acceleration(glm::vec3(0.0f, -14.0f, 0.0f));
    }
}

int Utility::enemy_collision(Entity* player, Entity** entities, int numEntities) {
    for (int i = 0; i < numEntities; i++) {
        // make sure the selected entity exists and is actually an enemy
        Entity* enemy = entities[i];
        if (!enemy) continue;
        const std::type_info& eType = typeid(*enemy);
        if (eType != typeid(WalkerEntity) and
            eType != typeid(CrawlerEntity) and
            eType != typeid(FlyerEntity)) continue;
        // process collision
        if (player->check_collision(enemy)) {
            if ((player->get_velocity().y < 0 or enemy->get_velocity().y > 0)
                and player->get_position().y > 0.3f + enemy->get_position().y) {
                if (eType == typeid(CrawlerEntity) and !enemy->get_angle()) {
                    // stomping a crawler kills you if the spike is pointing up
                    return 1;
                }
                enemy->despawn();
                player->set_velocity(glm::vec3(0.0f, 4.0f, 0.0f));
                return 2;
            }
            else {
                return 1;
            }
        }
    }
    return 0;
}