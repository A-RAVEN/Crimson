#include "GraphicsFrame.h"

void GraphicsFrame::AddInstance(InstanceInfo const& instance_info)
{
	auto find = m_InstanceList.find(instance_info.p_Mesh);
	if (find == m_InstanceList.end())
	{
		m_InstanceList.insert(std::make_pair(instance_info.p_Mesh, std::deque<InstanceInfo>{}));
		find = m_InstanceList.find(instance_info.p_Mesh);
	}
	find->second.push_back(instance_info);
}

void GraphicsFrame::RemoveInstance(InstanceInfo const& instance_info)
{
	//auto find = m_InstanceList.find(instance_info.p_Mesh);
	//if (find != m_InstanceList.end())
	//{
	//	find.sec
	//	m_InstanceList.insert(std::make_pair(instance_info.p_Mesh, std::deque<InstanceInfo>{}));
	//	find = m_InstanceList.find(instance_info.p_Mesh);
	//}
	//m_RemovedInstances.push_back(instance_info);
}

void GraphicsFrame::UpdateTransform(TransformInfo const& transform_info)
{
	m_TransoformUpdateInfo.push_back(transform_info);
}
