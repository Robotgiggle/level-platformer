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
#include "../entities/WalkerEntity.h"
#include "../entities/CrawlerEntity.h"
#include "../entities/FlyerEntity.h"
#include "../entities/HudEntity.h"
#include "Level3.h"

// terrain map
const int LV3_WIDTH = 30,
          LV3_HEIGHT = 7;
const int LV3_DATA[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 18,
    0,  0,  0,  0,  0,  8, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  3,  4,  0,  0,  0,  8,  9,
    0,  0,  0,  0,  8, 12, 16,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 17,
    9,  9,  9,  9, 12, 17, 16,  0,  0,  2,  3,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 17,
   17, 17, 17, 17, 17, 17, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 17,
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
           PAUSE_FILEPATH[] = "assets/pause_screen.png",
           MAP_TILES_FILEPATH[] = "assets/map_tiles.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3",
           JUMP_FILEPATH[] = "assets/default_jump.wav",
           STOMP_FILEPATH[] = "assets/enemy_stomp.wav",
           DEATH_FILEPATH[] = "assets/death_slow.wav",
           PICKUP_FILEPATH[] = "assets/coin_pickup.wav";

// useful constants
const float ACC_OF_GRAVITY = -6.0f;

// constructor definition
Level3::Level3(int cap) : Scene(cap) {}

// other methods
void Level3::initialise() {
    // ————— BASICS ————— //
    Scene::initialise();
    m_state.nextSceneID = 4;
    m_unordered_render_start = 5;

    // ————— TERRAIN ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(LV3_WIDTH, LV3_HEIGHT, LV3_DATA, map_texture_id, 1.0f, 6, 4);

    // ————— PAUSE SCREEN ————— //
    // create entity
    e_pauseScreen = new Entity(this);

    // setup basic attributes
    e_pauseScreen->set_position(glm::vec3(4.5f, 3.25f, 0.0f));
    e_pauseScreen->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_pauseScreen->m_texture_id = Utility::load_texture(PAUSE_FILEPATH);

    e_pauseScreen->update(0.0f, NULL, 0, m_state.map);

    // ————— PLAYER ————— //
    // create entity
    e_player = new Entity(this);

    // setup basic attributes
    e_player->set_motion_type(Entity::SIDE_ON);
    e_player->set_position(glm::vec3(0.5f, 2.0f, 0.0f));
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

    // ————— BACKGROUND ————— //
    // create entity
    e_background = new HudEntity(this, e_player, glm::vec3(0.0f));

    // setup basic attributes
    e_background->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_background->m_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);

    // ————— HEALTHBAR ————— //
    // create entity
    e_healthbar = new HudEntity(this, e_player, glm::vec3(-3.5f, 3.25f, 0.0f));

    // setup basic attributes
    e_healthbar->set_sprite_scale(glm::vec3(2.5f, 0.6f, 0.0f));
    e_healthbar->m_texture_id = Utility::load_texture(HEALTHBAR_FILEPATH);

    // setup sprite variants
    e_healthbar->m_animation_indices = new int[4] { 0, 1, 2, 3 };
    e_healthbar->setup_anim(1, 4, 4, 0, 2);

    // ————— COINBAR ————— //
    // create entity
    e_coinbar = new HudEntity(this, e_player, glm::vec3(3.75f, 3.25f, 0.0f));

    // setup basic attributes
    e_coinbar->set_sprite_scale(glm::vec3(2.1f, 0.6f, 0.0f));
    e_coinbar->m_texture_id = Utility::load_texture(COINBAR_FILEPATH);

    // setup sprite variants
    e_coinbar->m_animation_indices = new int[8] { 0, 2, 4, 6, 1, 3, 5, 7 };
    e_coinbar->setup_anim(2, 4, 4, 0, 2);

    // ————— BONUS COIN ————— //
    // create entity
    e_coin = new Entity(this);

    // setup basic attributes
    e_coin->set_position(glm::vec3(15.5f, 6.0f, 0.0f));
    e_coin->set_scale(glm::vec3(0.6f, 0.6f, 0.0f));
    e_coin->set_sprite_scale(glm::vec3(0.6f, 0.6f, 0.0f));
    e_coin->m_texture_id = Utility::load_texture(COIN_FILEPATH);

    // setup spin animation
    e_coin->m_walking[0] = new int[4] { 0, 2 };
    e_coin->m_walking[1] = new int[4] { 1, 3 };
    e_coin->m_animation_indices = e_coin->m_walking[m_global_info->coins & 4];
    e_coin->setup_anim(2, 2, 2, 4, 1);

    // ————— WALKERS ————— //
    // create entity
    WalkerEntity* newWalker1 = spawn<WalkerEntity>(this, 1);
    WalkerEntity* newWalker2 = spawn<WalkerEntity>(this, 1);
    WalkerEntity* newWalker3 = spawn<WalkerEntity>(this, 1);

    // setup basic attributes
    newWalker1->set_position(glm::vec3(9.0f, 2.0f, 0.0f));
    newWalker2->set_position(glm::vec3(15.0f, 3.0f, 0.0f));
    newWalker3->set_position(glm::vec3(21.0f, 4.0f, 0.0f));
    for (Entity* walker : { newWalker1, newWalker2, newWalker3 }) {
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
    CrawlerEntity* newCrawler1 = spawn<CrawlerEntity>(this, 0, true);
    CrawlerEntity* newCrawler2 = spawn<CrawlerEntity>(this, 2, true);

    // setup basic attributes
    newCrawler1->set_position(glm::vec3(12.0f, 4.9f, 0.0f));
    newCrawler2->set_position(glm::vec3(11.0f, 3.1f, 0.0f));
    for (Entity* crawler : { newCrawler1, newCrawler2 }) {
        crawler->set_speed(1.87f);
        crawler->set_scale(glm::vec3(0.6f, 0.75f, 0.0f));
        crawler->set_sprite_scale(glm::vec3(0.7f, 0.8f, 0.0f));
        crawler->m_texture_id = Utility::load_texture(CRAWLER_FILEPATH);

        // setup walking animation
        crawler->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        crawler->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        crawler->setup_anim(2, 2, 2, 6);
    }
    newCrawler1->m_animation_indices = newCrawler1->m_walking[Entity::RIGHT];
    newCrawler2->m_animation_indices = newCrawler2->m_walking[Entity::LEFT];

    // ————— FLYERS ————— //
    // create entities
    FlyerEntity* newFlyer1 = spawn<FlyerEntity>(this, 0.5f, 0.75f, 4.0f);
    FlyerEntity* newFlyer2 = spawn<FlyerEntity>(this, 0.5f, 0.75f, 4.0f);
    FlyerEntity* newFlyer3 = spawn<FlyerEntity>(this, 0.5f, 0.75f, 4.0f);

    // setup basic attributes
    newFlyer1->set_position(glm::vec3(7.0f, 5.0f, 0.0f));
    newFlyer2->set_position(glm::vec3(20.0f, 5.0f, 0.0f));
    newFlyer3->set_position(glm::vec3(27.0f, 5.0f, 0.0f));
    for (Entity* flyer : { newFlyer1, newFlyer2, newFlyer3 }) {
        flyer->set_speed(4.8f);
        flyer->set_scale(glm::vec3(0.55f, 0.55f, 0.0f));
        flyer->set_sprite_scale(glm::vec3(0.84f, 0.63f, 0.0f));
        flyer->m_texture_id = Utility::load_texture(FLYER_FILEPATH);

        // setup flapping animation
        flyer->m_walking[Entity::LEFT] = new int[4] { 0, 2, 4 };
        flyer->m_walking[Entity::RIGHT] = new int[4] { 1, 3, 5 };
        flyer->m_animation_indices = flyer->m_walking[Entity::LEFT];
        flyer->setup_anim(2, 3, 3, 4, true);
    }

    // ————— AUDIO ————— //
    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);

    m_state.jumpSfx = Mix_LoadWAV(JUMP_FILEPATH);
    m_state.coinSfx = Mix_LoadWAV(PICKUP_FILEPATH);
    m_state.stompSfx = Mix_LoadWAV(STOMP_FILEPATH);
    m_state.deathSfx = Mix_LoadWAV(DEATH_FILEPATH);
    Mix_Volume(-1, MIX_MAX_VOLUME / 2);
}

void Level3::process_event(SDL_Event event) {
    // process event triggers
    switch (event.type) {
    case SDL_KEYDOWN:
        // process keydown triggers specifically
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            m_global_info->gamePaused = true;
            break;
        case SDLK_RETURN:
            m_global_info->gamePaused = false;
            break;
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

void Level3::process_input()
{
    // reset forced-movement if no player input
    e_player->set_movement(glm::vec3(0.0f));
    e_player->set_rotation(0.0f);

    // no movement if you're dead
    if (m_global_info->playerDead) return;

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

void Level3::update(float delta_time) {
    // update entities
    Scene::update(delta_time);

    // check for death fall
    if (e_player->get_position().y <= 0 && !m_global_info->playerDead) {
        Mix_PlayChannel(-1, m_state.deathSfx, 0);
        Utility::player_death(e_player, m_global_info);
        m_state.nextSceneID = 3;
    }

    // check for enemy collision
    if (!m_global_info->playerDead) {
        int status = Utility::enemy_collision(e_player, m_state.entities, m_entity_cap);
        if (status == 1) {
            // player died
            Mix_PlayChannel(-1, m_state.deathSfx, 0);
            Utility::player_death(e_player, m_global_info);
            m_state.nextSceneID = 3;
        } else if (status == 2) {
            // enemy stomped
            Mix_PlayChannel(-1, m_state.stompSfx, 0);
        }
    }

    // check for coin collision
    if (e_player->check_collision(e_coin) and !m_global_info->playerDead) {
        if (!(m_global_info->coins & 4)) m_timer = 2.5f;
        Mix_PlayChannel(-1, m_state.coinSfx, 0);
        m_global_info->coins |= 4;
        e_coin->set_active(false);
    }

    // update HUD elements
    e_healthbar->m_animation_index = m_global_info->lives;
    e_coinbar->m_animation_index = m_global_info->coins;

    // check for level transition
    glm::vec3 pos = e_player->get_position();
    if (pos.x > 29.0f and pos.y < 6.0f) m_global_info->changeScenes = true;

    // tick timer
    if (m_timer > 0.0f) m_timer -= delta_time;
}

void Level3::render(ShaderProgram* program) {
    e_background->render(program);
    Scene::render(program);
    e_player->render(program);
    e_healthbar->render(program);
    if (m_timer > 0.0f) e_coinbar->render(program);
    if (m_global_info->gamePaused) e_pauseScreen->render(program);
}