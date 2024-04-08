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
#include "Level1.h"

// terrain map
const int LV1_WIDTH = 30,
          LV1_HEIGHT = 7;
const int LV1_DATA[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 10,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  2,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 12, 16,  0,  0,  0,  0, 18,
    9,  9, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9,  9,  9,  9,  9,  9, 12, 15, 16,  0,  0,  0,  8,  9,
   15, 15, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 15, 15, 15, 15, 15, 15, 15, 15, 16,  0,  0,  0, 14, 15,
};

// sprite filepaths
const char SPRITESHEET_FILEPATH[] = "assets/player.png",
           BACKGROUND_FILEPATH[] = "assets/background.png",
           HEALTHBAR_FILEPATH[] = "assets/healthbar.png",
           COINBAR_FILEPATH[] = "assets/coinbar.png",
           WALKER_FILEPATH[] = "assets/walker.png",
           CRAWLER_FILEPATH[] = "assets/crawler.png",
           FLYER_FILEPATH[] = "assets/flyer.png",
           COIN_FILEPATH[] = "assets/coin.png",
           MAP_TILES_FILEPATH[] = "assets/map_tiles.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3",
           JUMP_FILEPATH[] = "assets/default_jump.wav",
           PICKUP_FILEPATH[] = "assets/coin_pickup.wav";

// useful constants
const float ACC_OF_GRAVITY = -6.0f;

// constructor definition
Level1::Level1(int cap) : Scene(cap) {}

// other methods
void Level1::initialise() {
    // ����� NEXT SCENE ����� //
    m_state.nextSceneID = 2;
    
    // ����� TERRAIN ����� //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(LV1_WIDTH, LV1_HEIGHT, LV1_DATA, map_texture_id, 1.0f, 6, 4);

    // ����� BACKGROUND ����� //
    // create entity
    e_background = new Entity(this);

    // setup basic attributes
    e_background->set_position(glm::vec3(4.5f, 3.25f, 0.0f));
    e_background->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_background->m_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);

    // ����� HEALTHBAR ����� //
    // create entity
    e_healthbar = new Entity(this);

    // setup basic attributes
    e_healthbar->set_position(glm::vec3(1.0f, 6.5f, 0.0f));
    e_healthbar->set_sprite_scale(glm::vec3(2.5f, 0.6f, 0.0f));
    e_healthbar->m_texture_id = Utility::load_texture(HEALTHBAR_FILEPATH);

    // setup sprite variants
    e_healthbar->m_animation_indices = new int[4] { 0, 1, 2, 3 };
    e_healthbar->setup_anim(1, 4, 4, 0, 2);

    // ����� COINBAR ����� //
    // create entity
    e_coinbar = new Entity(this);

    // setup basic attributes
    e_coinbar->set_position(glm::vec3(8.0f, 6.75f, 0.0f));
    e_coinbar->set_sprite_scale(glm::vec3(2.1f, 0.6f, 0.0f));
    e_coinbar->m_texture_id = Utility::load_texture(COINBAR_FILEPATH);

    // setup sprite variants
    e_coinbar->m_animation_indices = new int[8] { 0, 2, 4, 6, 1, 3, 5, 7 };
    e_coinbar->setup_anim(2, 4, 4, 0, 2);

    // ����� BONUS COIN ����� //
    // create entity
    e_coin = new Entity(this);

    // setup basic attributes
    e_coin->set_position(glm::vec3(17.0f, 5.0f, 0.0f));
    e_coin->set_scale(glm::vec3(0.6f, 0.6f, 0.0f));
    e_coin->set_sprite_scale(glm::vec3(0.6f, 0.6f, 0.0f));
    e_coin->m_texture_id = Utility::load_texture(COIN_FILEPATH);

    // setup spin animation
    e_coin->m_animation_indices = new int[4] { 0, 1 };
    e_coin->setup_anim(2, 1, 2, 4, 1);

    // ����� PLAYER ����� //
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
    e_player->m_jumping_power = 5.4f;

    // setup walking animation
    e_player->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
    e_player->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
    e_player->m_animation_indices = e_player->m_walking[Entity::RIGHT];
    e_player->setup_anim(2, 2, 2, 6);

    // ����� WALKER ����� //
    // create entity
    e_walker = new WalkerEntity(this,0);

    // setup basic attributes
    e_walker->set_motion_type(Entity::SIDE_ON);
    e_walker->set_position(glm::vec3(21.0f, 2.0f, 0.0f));
    e_walker->set_movement(glm::vec3(0.0f));
    e_walker->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_walker->set_speed(2.0f);
    e_walker->set_scale(glm::vec3(0.72f, 0.9f, 0.0f));
    e_walker->set_sprite_scale(glm::vec3(0.765f, 0.9f, 0.0f));
    e_walker->m_texture_id = Utility::load_texture(WALKER_FILEPATH);

    // setup walking animation
    e_walker->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
    e_walker->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
    e_walker->m_animation_indices = e_walker->m_walking[Entity::RIGHT];
    e_walker->setup_anim(2, 2, 2, 6);

    // ����� CRAWLERS ����� //
    // create entities
    e_crawler1 = new CrawlerEntity(this, 2, false);
    e_crawler2 = new CrawlerEntity(this, 0, false);

    // setup basic attributes
    e_crawler1->set_position(glm::vec3(5.0f, 1.1f, 0.0f));
    e_crawler2->set_position(glm::vec3(12.0f, 3.9f, 0.0f));
    for (Entity* crawler : { e_crawler1, e_crawler2 }) {
        crawler->set_speed(3.0f);
        crawler->set_scale(glm::vec3(0.6f, 0.75f, 0.0f));
        crawler->set_sprite_scale(glm::vec3(0.7f, 0.8f, 0.0f));
        crawler->m_texture_id = Utility::load_texture(CRAWLER_FILEPATH);

        // setup walking animation
        crawler->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        crawler->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        crawler->m_animation_indices = crawler->m_walking[Entity::LEFT];
        crawler->setup_anim(2, 2, 2, 6);
    }

    // ����� FLYERS ����� //
    // create entities
    e_flyer1 = new FlyerEntity(this, 0.5f, 0.75f, 4.0f);
    e_flyer2 = new FlyerEntity(this, 0.5f, 0.75f, 4.0f);

    // setup basic attributes
    e_flyer1->set_position(glm::vec3(22.0f, 5.0f, 0.0f));
    e_flyer2->set_position(glm::vec3(27.0f, 5.0f, 0.0f));
    for (Entity* flyer : { e_flyer1, e_flyer2 }) {
        flyer->set_speed(4.8f);
        flyer->set_scale(glm::vec3(0.55f, 0.55f, 0.0f));
        flyer->set_sprite_scale(glm::vec3(0.84f, 0.63f, 0.0f));
        flyer->m_texture_id = Utility::load_texture(FLYER_FILEPATH);

        // setup flapping animation
        flyer->m_walking[Entity::LEFT] = new int[4] { 0, 2, 4 };
        flyer->m_walking[Entity::RIGHT] = new int[4] { 1, 3, 5 };
        flyer->m_animation_indices = e_flyer1->m_walking[Entity::LEFT];
        flyer->setup_anim(2, 3, 3, 4, true);
    }

    // ����� AUDIO ����� //
    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);
    
    m_state.jumpSfx = Mix_LoadWAV(JUMP_FILEPATH);
    m_state.coinSfx = Mix_LoadWAV(PICKUP_FILEPATH);
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
}

void Level1::process_event(SDL_Event event) {
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

void Level1::process_input()
{
    // reset forced-movement if no player input
    e_player->set_movement(glm::vec3(0.0f));
    e_player->set_rotation(0.0f);

    // no movement if you're dead
    if (m_globalInfo->playerDead) return;

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

void Level1::update(float delta_time) {
    // update entities
    for (int i = 1; i < 8; i++) m_state.entities[i]->update(delta_time, NULL, 0, m_state.map);

    // check for death fall
    if (e_player->get_position().y <= 0 && !m_globalInfo->playerDead) {
        Utility::player_death(e_player, m_globalInfo);
        m_state.nextSceneID = 1;
    }

    // check for enemy collision
    for (int i = 2; i < 7; i++) {
        Entity* enemy = m_state.entities[i];
        if (e_player->check_collision(enemy) and !m_globalInfo->playerDead) {
            if ((e_player->get_velocity().y < 0 or enemy->get_velocity().y > 0)
                and e_player->get_position().y > 0.3f + enemy->get_position().y) {
                if ((i == 3 or i == 4) and (!enemy->get_angle())) {
                    // stomping a crawler kills you if the spike is pointing up
                    Utility::player_death(e_player, m_globalInfo);
                    m_state.nextSceneID = 1;
                    continue;
                }
                enemy->set_active(false);
                e_player->set_velocity(glm::vec3(0.0f, 4.0f, 0.0f));
            }
            else {
                Utility::player_death(e_player, m_globalInfo);
                m_state.nextSceneID = 1;
            }
        }
    }

    // check for coin collision
    if (e_player->check_collision(e_coin) and !m_globalInfo->playerDead) {
        Mix_PlayChannel(-1, m_state.coinSfx, 0);
        m_globalInfo->coins |= 1;
        e_coin->set_active(false);
        m_timer = 2.5f;
    }

    // move background
    Utility::move_background(e_player, e_background, m_state.map);
    e_background->update(delta_time, NULL, 0, m_state.map);

    // update and move healthbar
    e_healthbar->set_position(e_background->get_position() + glm::vec3(-3.5f, 3.25f, 0.0f));
    e_healthbar->m_animation_index = m_globalInfo->lives;
    e_healthbar->update(delta_time, NULL, 0, m_state.map);

    // update and move coinbar
    e_coinbar->set_position(e_background->get_position() + glm::vec3(3.75f, 3.25f, 0.0f));
    e_coinbar->m_animation_index = m_globalInfo->coins;
    e_coinbar->update(delta_time, NULL, 0, m_state.map);

    // check for level transition
    glm::vec3 pos = e_player->get_position();
    if (pos.x > 29.0f and pos.y < 3.5f) m_globalInfo->changeScenes = true;

    // tick timer
    if (m_timer > 0.0f) m_timer -= delta_time;
}

void Level1::render(ShaderProgram* program) {
    e_background->render(program);
    m_state.map->render(program);
    for (int i = 1; i < 9; i++) m_state.entities[i]->render(program);
    if (m_timer > 0.0f) e_coinbar->render(program);
}