#pragma once
#include <glm/glm.hpp>

struct Camera
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
};