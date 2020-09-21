#pragma once
#include <headers/Components/IComponent.h>
#include <Generals.h>

using namespace Crimson;
class CameraComponent : public IComponent
{
public:
	PRenderPassInstance m_RenderPassInstance;

};