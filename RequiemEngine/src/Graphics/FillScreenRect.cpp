#include <headers/Graphics/FillScreenRect.h>

void FillScreenRect::Init(PGPUDevice p_device)
{
	p_Device = p_device;
}

void FillScreenRect::Draw(PGraphicsCommandBuffer cmdBuffer)
{
	cmdBuffer->BindVertexInputeBuffer({ m_VertexBuffer }, { 0 });
	cmdBuffer->BindIndexBuffer(m_IndiciesBuffer, 0, EIndexType::E_INDEX_TYPE_16);
	cmdBuffer->DrawIndexed(6, 1);
}
