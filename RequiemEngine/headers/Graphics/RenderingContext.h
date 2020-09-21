#pragma once
#include <string>
class RenderingContext
{
public:
	void DrawLayer(std::string layerName);
	void DrawFillScreenRect();
	void BindPipeline();
};