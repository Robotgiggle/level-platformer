#pragma once
#include "../Entity.h"

class HudEntity : public Entity {
private:
	Entity* m_player;
	glm::vec3 m_rel_pos;
public:
	HudEntity(Scene* scene, Entity* player, glm::vec3 relPos);
	~HudEntity();

	// ————— CUSTOM UPDATE METHOD ————— //
	void update(float delta_time, Entity* collidable_entities, int collidable_entity_count, Map* map) override;

	// ————— GETTERS ————— //
	glm::vec3 const get_rel_pos() const { return m_rel_pos; };
	
	// ————— SETTERS ————— //
	void const set_rel_pos(glm::vec3 new_pos) { m_rel_pos = new_pos; };
};