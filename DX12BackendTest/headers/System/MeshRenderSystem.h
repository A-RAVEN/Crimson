#pragma once
#include <headers/System/System.h>
#include <set>

class MeshRenderSystem : public ISystem
{
public:
	virtual void Configure(World& world) override;
	virtual void Execute(World const& world) override;
	virtual void OnEntityCreated(World const& world, EntityId entityId) override;
	virtual void OnEntityDestroyed(World const& world, EntityId entityId) override;
private:
	std::set<EntityId> const* p_MeshRendererEntitySet;
	std::set<EntityId> const* p_CameraEntitySet;
};