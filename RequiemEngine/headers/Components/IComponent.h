#pragma once
#include <stdint.h>

class IComponent
{
public:
	friend class World;
	uint32_t GetEntityId() { return m_Entity; }
private:
	uint32_t m_Entity;
};