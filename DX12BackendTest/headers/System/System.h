#pragma once
#include <headers/Entity/Entity.h>
class World;
class ISystem
{
public:
	void ConfigureListeningBits(std::vector<ComponentBits> const& bits);
	bool TestingBits(ComponentBits const& bits);
	virtual void Configure(World &world) = 0;
	virtual void Execute(World const& world) = 0;
	virtual void OnEntityCreated(World const& world, EntityId entityId) = 0;
	virtual void OnEntityDestroyed(World const& world, EntityId entityId) = 0;
private:
	std::vector<ComponentBits> m_ListeningBits;
};