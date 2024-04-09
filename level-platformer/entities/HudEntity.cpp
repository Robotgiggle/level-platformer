#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "../glm/mat4x4.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../ShaderProgram.h"
#include "../Scene.h"
#include "HudEntity.h"

HudEntity::HudEntity(Scene* scene, Entity* player, glm::vec3 relPos):
	Entity(scene), m_player(player), m_rel_pos(relPos)
{}

HudEntity::~HudEntity() {

}

void HudEntity::update(float delta_time, Entity* collidable_entities, int collidable_entity_count, Map* map) {
	float playerX = m_player->get_position().x;
	float rBound = map->get_right_bound();

	// calculate proper X position
	float xPos = glm::min(glm::max(playerX, 4.5f), rBound - 5.0f);

	// move into place
	set_position(glm::vec3(xPos, 3.25f, 0.0f) + m_rel_pos);

	Entity::update(delta_time, collidable_entities, collidable_entity_count, map);
}