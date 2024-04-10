#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "../glm/mat4x4.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../ShaderProgram.h"
#include "../Utility.h"
#include "../entities/CrawlerEntity.h"
#include "Menu.h"

// terrain map
const int MENU_WIDTH = 10,
          MENU_HEIGHT = 7;
const int MENU_DATA[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 4, 0, 0, 0, 0, 2, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
};

// sprite filepaths
const char SPRITESHEET_FILEPATH[] = "assets/player.png",
           BACKGROUND_FILEPATH[] = "assets/background.png",
           CRAWLER_FILEPATH[] = "assets/crawler.png",
           FONT_FILEPATH[] = "assets/pixel_font.png",
           MAP_TILES_FILEPATH[] = "assets/map_tiles.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3",
           JUMP_FILEPATH[] = "assets/default_jump.wav";

// useful constants
const float ACC_OF_GRAVITY = -4.91f;

// constructor definition
Menu::Menu(int cap) : Scene(cap) {}

// other methods
void Menu::initialise() {
    // ————— BASICS ————— //
    Scene::initialise();
    m_state.nextSceneID = 1;
    m_unordered_render_start = 1;

    // ————— FONT ————— //
    m_font_id = Utility::load_texture(FONT_FILEPATH);

    // ————— TERRAIN ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(MENU_WIDTH, MENU_HEIGHT, MENU_DATA, map_texture_id, 1.0f, 6, 4);

    // ————— BACKGROUND ————— //
    // create entity
    e_background = new Entity(this);

    // setup basic attributes
    e_background->set_position(glm::vec3(4.5f, 3.25f, 0.0f));
    e_background->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_background->m_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);

    e_background->update(0.0f, NULL, 0, m_state.map);

    // ————— PLAYER ————— //
    // create entity
    e_player = new Entity(this);

    // setup basic attributes
    e_player->set_motion_type(Entity::SIDE_ON);
    e_player->set_position(glm::vec3(4.5f, 1.0f, 0.0f));
    e_player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_player->set_speed(1.8f);
    e_player->set_rot_speed(1.0f);
    e_player->set_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->set_sprite_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->m_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    e_player->m_jumping_power = 2.5f;

    // setup walking animation
    e_player->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
    e_player->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
    e_player->m_animation_indices = e_player->m_walking[Entity::RIGHT];
    e_player->setup_anim(2, 2, 2, 6);

    // ————— CRAWLERS ————— //
    // create entities
    e_crawler1 = new CrawlerEntity(this, 0, true);
    e_crawler2 = new CrawlerEntity(this, 2, true);

    // setup basic attributes
    e_crawler1->set_position(glm::vec3(1.0f, 4.0f, 0.0f));
    e_crawler2->set_position(glm::vec3(8.0f, 2.0f, 0.0f));
    for (Entity* crawler : { e_crawler1, e_crawler2 }) {
        crawler->set_speed(3.0f);
        crawler->set_scale(glm::vec3(0.6f, 0.75f, 0.0f));
        crawler->set_sprite_scale(glm::vec3(0.7f, 0.8f, 0.0f));
        crawler->m_texture_id = Utility::load_texture(CRAWLER_FILEPATH);

        // setup walking animation
        crawler->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        crawler->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        crawler->setup_anim(2, 2, 2, 6);
    }
    e_crawler1->m_animation_indices = e_crawler1->m_walking[Entity::RIGHT];
    e_crawler2->m_animation_indices = e_crawler2->m_walking[Entity::LEFT];

    // ————— AUDIO ————— //
    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);

    m_state.jumpSfx = Mix_LoadWAV(JUMP_FILEPATH);
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
}

void Menu::process_event(SDL_Event event) {
    // process event triggers
    switch (event.type) {
    case SDL_KEYDOWN:
        // process keydown triggers specifically
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
            m_global_info->changeScenes = true;
            break;
        default:
            break;
        }
    }
}

void Menu::process_input()
{
    
}

void Menu::update(float delta_time) {
    // player jump animation
    if (e_player->m_collided_bottom && m_timer <= 0) {
        e_player->m_is_jumping = true;
        m_timer = 1.0f;
    }
    m_timer -= delta_time;

    // update entities
    Scene::update(delta_time);
}

void Menu::render(ShaderProgram* program) {
    e_background->render(program);
    Scene::render(program);
    
    Utility::draw_text(program, m_font_id, "Bounder", 1.0f, 0.0f, glm::vec3(1.6f, 5.4f, 0.0f));
    Utility::draw_text(program, m_font_id, "Press", 0.45f, 0.0f, glm::vec3(3.65f, 3.5f, 0.0f));
    Utility::draw_text(program, m_font_id, "ENTER", 0.37f, 0.0f, glm::vec3(3.82f, 2.9f, 0.0f));
}