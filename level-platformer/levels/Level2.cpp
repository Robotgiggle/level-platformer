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
#include "../WalkerEntity.h"
#include "../CrawlerEntity.h"
#include "../FlyerEntity.h"
#include "Level2.h"

// terrain map
const int LV2_WIDTH = 30,
          LV2_HEIGHT = 7;
const int LV2_DATA[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 10,  0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  0,  0,  0,
    0,  0,  0,  0,  2,  3,  4,  0,  0,  0,  8, 12, 16,  0,  0,  0, 13,  0,  0,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 12, 17, 16,  0,  0,  0, 13,  0,  0,  0, 13,  0,  0,  0,  8,  9,  9,  9,  9,  9,
    9,  9, 10,  0,  0,  0,  0,  0,  8, 12, 17, 17, 16,  0,  0,  0, 13,  0,  0,  0, 13,  0,  0,  0, 14, 17, 17, 17, 17, 17,
   17, 17, 11,  9,  9,  9,  9,  9, 12, 17, 17, 17, 16,  0,  0,  0, 13,  0,  0,  0, 13,  0,  0,  0, 14, 17, 17, 17, 17, 17,
};

// sprite filepaths
const char SPRITESHEET_FILEPATH[] = "assets/player.png",
           BACKGROUND_FILEPATH[] = "assets/background.png",
           WALKER_FILEPATH[] = "assets/walker.png",
           CRAWLER_FILEPATH[] = "assets/crawler.png",
           FLYER_FILEPATH[] = "assets/flyer.png",
           MAP_TILES_FILEPATH[] = "assets/map_tiles.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3";

// useful constants
const float ACC_OF_GRAVITY = -4.91f;

// constructor definition
Level2::Level2(int cap) : Scene(cap) {}

// other methods
void Level2::initialise() {
    // ————— NEXT SCENE ————— //
    m_state.nextSceneID = 3;

    // ————— TERRAIN ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(LV2_WIDTH, LV2_HEIGHT, LV2_DATA, map_texture_id, 1.0f, 6, 4);

    // ————— BACKGROUND ————— //
    // create entity
    e_background = new Entity(this);

    // setup basic attributes
    e_background->set_position(glm::vec3(4.5f, 3.25f, 0.0f));
    e_background->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_background->m_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);

    // ————— PLAYER ————— //
    // create entity
    e_player = new Entity(this);

    // setup basic attributes
    e_player->set_motion_type(Entity::SIDE_ON);
    e_player->set_position(glm::vec3(1.0f, 2.0f, 0.0f));
    e_player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_player->set_speed(1.8f);
    e_player->set_rot_speed(1.0f);
    e_player->set_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->set_sprite_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->m_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    e_player->m_jumping_power = 4.5f;

    // setup walking animation
    e_player->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
    e_player->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
    e_player->m_animation_indices = e_player->m_walking[Entity::RIGHT];
    e_player->setup_anim(2, 2, 2, 6);

    // ————— WALKERS ————— //
    // create entity
    e_walker1 = new WalkerEntity(this, 1);
    e_walker2 = new WalkerEntity(this, 0);

    // setup basic attributes
    e_walker1->set_position(glm::vec3(3.0f, 1.0f, 0.0f));
    e_walker2->set_position(glm::vec3(6.0f, 6.0f, 0.0f));
    for (Entity* walker : { e_walker1, e_walker2 }) {
        walker->set_motion_type(Entity::SIDE_ON);
        walker->set_movement(glm::vec3(0.0f));
        walker->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
        walker->set_speed(2.0f);
        walker->set_scale(glm::vec3(0.765f, 0.9f, 0.0f));
        walker->set_sprite_scale(glm::vec3(0.765f, 0.9f, 0.0f));
        walker->m_texture_id = Utility::load_texture(WALKER_FILEPATH);

        // setup walking animation
        walker->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        walker->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        walker->m_animation_indices = walker->m_walking[Entity::RIGHT];
        walker->setup_anim(2, 2, 2, 6);
    }

    // ————— CRAWLERS ————— //
    // create entities
    e_crawler1 = new CrawlerEntity(this, 0, false);
    e_crawler2 = new CrawlerEntity(this, 2, false);

    // setup basic attributes
    e_crawler1->set_position(glm::vec3(25.5f, 4.9f, 0.0f));
    e_crawler2->set_position(glm::vec3(25.5f, 3.1f, 0.0f));
    for (Entity* crawler : { e_crawler1, e_crawler2 }) {
        crawler->set_speed(1.87f);
        crawler->set_scale(glm::vec3(0.7f, 0.8f, 0.0f));
        crawler->set_sprite_scale(glm::vec3(0.7f, 0.8f, 0.0f));
        crawler->m_texture_id = Utility::load_texture(CRAWLER_FILEPATH);

        // setup walking animation
        crawler->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        crawler->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        crawler->setup_anim(2, 2, 2, 6);
    }
    e_crawler1->m_animation_indices = e_crawler1->m_walking[Entity::RIGHT];
    e_crawler2->m_animation_indices = e_crawler2->m_walking[Entity::LEFT];

    // ————— FLYERS ————— //
    // create entities
    e_flyer1 = new FlyerEntity(this, 0.5f, 0.75f, 4.0f);
    e_flyer2 = new FlyerEntity(this, 0.5f, 0.75f, 4.0f);

    // setup basic attributes
    e_flyer1->set_position(glm::vec3(13.0f, 6.0f, 0.0f));
    e_flyer2->set_position(glm::vec3(19.5f, 6.0f, 0.0f));
    for (Entity* flyer : { e_flyer1, e_flyer2 }) {
        flyer->set_speed(5.5f);
        flyer->set_scale(glm::vec3(0.55f, 0.55f, 0.0f));
        flyer->set_sprite_scale(glm::vec3(0.84f, 0.63f, 0.0f));
        flyer->m_texture_id = Utility::load_texture(FLYER_FILEPATH);

        // setup flapping animation
        flyer->m_walking[Entity::LEFT] = new int[4] { 0, 2, 4 };
        flyer->m_walking[Entity::RIGHT] = new int[4] { 1, 3, 5 };
        flyer->m_animation_indices = e_flyer1->m_walking[Entity::LEFT];
        flyer->setup_anim(2, 3, 3, 4, true);
    }

    // ————— AUDIO ————— //
    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);

    m_state.jumpSfx = Mix_LoadWAV("assets/default_jump.wav");
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
}

void Level2::process_event(SDL_Event event) {
    // process event triggers
    switch (event.type) {
    case SDL_KEYDOWN:
        // process keydown triggers specifically
        switch (event.key.keysym.sym) {
        case SDLK_SPACE:
            if (e_player->m_collided_bottom) {
                e_player->m_is_jumping = true;
                Mix_PlayChannel(-1, m_state.jumpSfx, 0);
            }
            break;
        default:
            break;
        }
    }
}

void Level2::process_input()
{
    // reset forced-movement if no player input
    e_player->set_movement(glm::vec3(0.0f));
    e_player->set_rotation(0.0f);

    // event triggers are *NOT* handled in this function, unlike before
    // see process_event() for event handling

    // process held keys
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_LEFT]) {
        e_player->move_left();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    } else if (key_state[SDL_SCANCODE_RIGHT]) {
        e_player->move_right();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_Q]) {
        e_player->rotate_anticlockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    } else if (key_state[SDL_SCANCODE_E]) {
        e_player->rotate_clockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }

    // normalize forced-movement vector so you don't go faster diagonally
    if (glm::length(e_player->get_movement()) > 1.0f) {
        e_player->set_movement(glm::normalize(e_player->get_movement()));
    }
}

void Level2::update(float delta_time) {
    // update entities
    e_player->update(delta_time, NULL, 0, m_state.map);
    e_walker1->update(delta_time, NULL, 0, m_state.map);
    e_walker2->update(delta_time, NULL, 0, m_state.map);
    e_crawler1->update(delta_time, NULL, 0, m_state.map);
    e_crawler2->update(delta_time, NULL, 0, m_state.map);
    e_flyer1->update(delta_time, NULL, 0, m_state.map);
    e_flyer2->update(delta_time, NULL, 0, m_state.map);

    // move background
    Utility::move_background(e_player, e_background, m_state.map);
    e_background->update(delta_time, NULL, 0, m_state.map);

    // check for level transition
    if (e_player->get_position().x > 29.0f) m_changeScenes = true;
}

void Level2::render(ShaderProgram* program) {
    e_background->render(program);
    m_state.map->render(program);
    e_player->render(program);
    e_walker1->render(program);
    e_walker2->render(program);
    e_crawler1->render(program);
    e_crawler2->render(program);
    e_flyer1->render(program);
    e_flyer2->render(program);
}