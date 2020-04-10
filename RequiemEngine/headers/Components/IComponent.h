#pragma once
#include <stdint.h>

class IComponent
{
public:
	friend class World;
	virtual void ResetComponent() = 0;
	uint32_t GetEntityId() { return m_Entity; }
	uint32_t GetCompId() { return m_CompId; }
	virtual ~IComponent(){}
private:
	uint32_t m_Entity;
	uint32_t m_CompId;
};