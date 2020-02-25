#include "MeshQueue.h"

void MeshInstanceQueue::CmdDrawInstances(PGraphicsCommandBuffer command_buffer, uint32_t transform_batch_id)
{
	if (transform_batch_id < MeshQueue.size() && MeshQueue[transform_batch_id].Size() > 0)
	{
		size_t active_segment_num = MeshQueue[transform_batch_id].GetActiveSegmentNum();
		command_buffer->BindIndexBuffer(m_Resource->m_IndexBuffer, 0);
		for (size_t i = 0; i < active_segment_num; ++i)
		{
			size_t active_num = MeshQueue[transform_batch_id].GetSegmentActiveUsage(i);
			command_buffer->BindVertexInputeBuffer({ m_Resource->m_VertexBuffer, MeshQueue[transform_batch_id].GetBufferSegment(i) },
				{ 0, 0 });
			command_buffer->DrawIndexed(m_Resource->m_IndexSize, active_num);
		}
	}
}
