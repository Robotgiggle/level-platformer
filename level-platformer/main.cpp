/**
* Author: Ben Miller
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Scene.h"
#include "levels/Menu.h"
#include "levels/Level1.h"
#include "levels/Level2.h"
#include "levels/Level3.h"
#include "levels/EndScreen.h"

//  CONSTANTS  //

// window size
const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

// background color
const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

// viewport position & size
const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// shader filepaths
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// time constants
const float MILLISECONDS_IN_SECOND = 1000.0;
const float FIXED_TIMESTEP = 0.0166666f;

// scenes
Scene* const ALL_SCENES[] = {
    new Menu(10),
    new Level1(20),
    new Level2(20),
    new Level3(20),
    new EndScreen(10)
};

//  VARIABLES  //

// core globals
SDL_Window* g_displayWindow;
ShaderProgram g_shaderProgram;
glm::mat4 g_viewMatrix, g_projectionMatrix;
Scene* g_currentScene;

// global values visible to scenes (defined in Scene.h)
GlobalInfo g_globalInfo;

// times
float g_previousTicks = 0.0f;
float g_timeAccumulator = 0.0f;

//  GENERAL FUNCTIONS  //
void startup_scene(int index) {
    g_globalInfo.changeScenes = false;
    g_globalInfo.playerDead = false;
    g_currentScene = ALL_SCENES[index];
    g_currentScene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_displayWindow = SDL_CreateWindow("Multi-Level Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_displayWindow);
    SDL_GL_MakeCurrent(g_displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shaderProgram.load(V_SHADER_PATH, F_SHADER_PATH);

    g_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-4.5f, -3.25f, 0.0f));
    g_projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shaderProgram.set_projection_matrix(g_projectionMatrix);
    g_shaderProgram.set_view_matrix(g_viewMatrix);

    glUseProgram(g_shaderProgram.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    for (Scene* scene : ALL_SCENES) scene->m_global_info = &g_globalInfo;
    startup_scene(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // process event triggers, this loop needs to be done at the top level
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_globalInfo.gameIsRunning = false;
            break;
        default:
            g_currentScene->process_event(event);
            break;
        }
    }

    // process other stuff (like key holds)
    g_currentScene->process_input();
}

void update()
{
    //  DELTA TIME  //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previousTicks; // the delta time is the difference from the last frame
    g_previousTicks = ticks;

    //  FIXED TIMESTEP  //
    g_timeAccumulator += delta_time;
    if (g_timeAccumulator < FIXED_TIMESTEP) return;
    while (g_timeAccumulator >= FIXED_TIMESTEP)
    {
        // update scene, unless the game is paused
        if (!g_globalInfo.gamePaused) g_currentScene->update(FIXED_TIMESTEP);
        
        // if the player dies, move to the ending scene unless you're already there
        if (g_globalInfo.lives <= 0 && g_globalInfo.deathTimer <= 0 && g_currentScene != ALL_SCENES[4]) startup_scene(4);
        
        // if changeScene is set and the player isn't in the death animation, move to the next scene
        if (g_globalInfo.changeScenes && g_globalInfo.deathTimer <= 0) startup_scene(g_currentScene->m_state.nextSceneID);
        
        // if the player isn't alive, tick down the death timer
        if (g_globalInfo.playerDead) g_globalInfo.deathTimer -= FIXED_TIMESTEP;

        g_timeAccumulator -= FIXED_TIMESTEP;
    }

    //  CAMERA TRACKING  //
    if (g_currentScene->get_player()) {
        float playerX = g_currentScene->get_player()->get_position().x;
        float rBound = g_currentScene->m_state.map->get_right_bound();
        float xPos = -1.0f * glm::min(glm::max(playerX, 4.5f), rBound - 5.0f);
        
        g_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, -3.25f, 0.0f));
    }
}

void render()
{
    g_shaderProgram.set_view_matrix(g_viewMatrix);
    glClear(GL_COLOR_BUFFER_BIT);

    g_currentScene->render(&g_shaderProgram);

    SDL_GL_SwapWindow(g_displayWindow);
}

void shutdown() { 
    SDL_Quit();
    for (Scene* scene : ALL_SCENES) delete scene;
}

//  DRIVER GAME LOOP  /
int main(int argc, char* argv[])
{
    initialise();

    while (g_globalInfo.gameIsRunning)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}