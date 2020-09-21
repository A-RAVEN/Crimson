#pragma once
#include <GPUDevice.h>
#include <Generals.h>

using namespace Crimson;
class FillScreenRect
{
public:
	void Init(PGPUDevice p_device);
	void Draw(PGraphicsCommandBuffer cmdBuffer);
	std::vector<EDataType> GetDataType() { return { EDataType::EVEC2 }; }
private:
	PGPUDevice p_Device;
	PGPUBuffer m_VertexBuffer;
	PGPUBuffer m_IndiciesBuffer;
};