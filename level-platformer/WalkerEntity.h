#pragma once
#include "Entity.h"

class WalkerEntity : public Entity {
private:
	enum AIState { WALK_LEFT, WALK_RIGHT };
	AIState m_ai_state;
	glm::vec3 m_edge_check_offset;
	glm::vec3 m_wall_check_offset;
public:
	WalkerEntity(Scene* scene, int dir);
	~WalkerEntity();

	// ————— CUSTOM UPDATE METHOD ————— //
	void update(float delta_time, Entity* collidable_entities, int collidable_entity_count, Map* map) override;

	// ————— GETTERS ————— //
	AIState const get_ai_state() const { return m_ai_state; };

	// ————— SETTERS ————— //
	void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
};
