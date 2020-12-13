#include <headers/Graphics/FillScreenRect.h>
#include <array>
#include <glm/glm.hpp>
#include <stdint.h>

void FillScreenRect::Init(PGPUDevice p_device)
{
	p_Device = p_device;
	std::array<glm::vec2, 4> positions = { glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(-1.0f, 1.0f) };
	std::array<int16_t, 6> indices = { 0, 1, 2, 0, 2, 3 };
	m_VertexBuffer = p_Device->CreateBuffer(positions.size() * sizeof(glm::vec2), { Crimson::EBufferUsage::E_BUFFER_USAGE_VERTEX }, Crimson::EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	m_IndiciesBuffer = p_Device->CreateBuffer(indices.size() * sizeof(int16_t), { Crimson::EBufferUsage::E_BUFFER_USAGE_INDICIES }, Crimson::EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);

	memcpy(m_VertexBuffer->GetMappedPointer(), positions.data(), m_VertexBuffer->GetSize());
	memcpy(m_IndiciesBuffer->GetMappedPointer(), indices.data(), m_IndiciesBuffer->GetSize());

	m_VertexBuffer->UnMapp();
	m_IndiciesBuffer->UnMapp();
}

void FillScreenRect::Draw(PGraphicsCommandBuffer cmdBuffer)
{
	cmdBuffer->BindVertexInputeBuffer({ m_VertexBuffer }, { m_VertexBuffer->GetRange() }, { sizeof(glm::vec2) });
	cmdBuffer->BindIndexBuffer(m_IndiciesBuffer, 0, EIndexType::E_INDEX_TYPE_16);
	cmdBuffer->DrawIndexed(6, 1);
}
