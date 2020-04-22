#include <headers/System/MeshRenderSystem.h>
#include <headers/Components/MeshRenderComp.h>
#include <headers/Components/CameraComponent.h>
#include <headers/Entity/World.h>

void MeshRenderSystem::Configure(World& world)
{
	p_MeshRendererEntitySet = world.RegisterFilter<MeshRenderComp>();
	p_CameraEntitySet = world.RegisterFilter<CameraComponent>();
	ConfigureListeningBits({ world.GetBits<MeshRenderComp>(), world.GetBits<CameraComponent>() });
}

void MeshRenderSystem::Execute(World const& world)
{

}

void MeshRenderSystem::OnEntityCreated(World const& world, EntityId entityId)
{
}

void MeshRenderSystem::OnEntityDestroyed(World const& world, EntityId entityId)
{

}
