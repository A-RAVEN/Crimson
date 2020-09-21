#include <headers/AccelStructs/BVH.h>
#include <numeric>

constexpr size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();

size_t BVH::AllocVolume()
{
	size_t return_val = 0;
	if (m_AvailableVolumes.empty())
	{
		m_VolumePool.push_back(BVHVolume());
		return_val = m_VolumePool.size() - 1;
	}
	else
	{
		return_val = m_AvailableVolumes.front();
		m_VolumePool[return_val].Reset();
		m_AvailableVolumes.pop_front();
	}
	m_VolumePool[return_val].b_Insert = true;
	m_VolumePool[return_val].b_Active = true;
	m_InsertedVolumes.push_back(return_val);
	m_ActiveVolumes.insert(return_val);
	return return_val;
}

void BVH::FreeVolume(size_t handle)
{
	BVHVolume* volume = &m_VolumePool[handle];

	if (volume->b_Active)
	{
		volume->b_Active = false;
		m_FreedVolumes.push_back(handle);
	}
	if (!volume->b_Removed)
	{
		volume->b_Removed = true;
		m_RemovedVolumes.push_back(handle);
	}
	//if (m_ActiveVolumes.find(handle) != m_ActiveVolumes.end())
	//{
	//	m_ActiveVolumes.erase(handle);
	//	m_AvailableVolumes.push_back(handle);
	//}
}

void BVH::FreeVolumeImpl(size_t handle)
{
	m_ActiveVolumes.erase(handle);
	m_AvailableVolumes.push_back(handle);
}

void BVH::VolumeDirty(size_t handle)
{
	//if (m_ActiveVolumes.find(handle) != m_ActiveVolumes.end())
	//{
		BVHVolume& vol = operator[](handle);
		if (!vol.b_Active) { return; }
		if (!vol.b_Insert)
		{
			vol.b_Insert = true;
			m_InsertedVolumes.push_back(handle);
		}
		if (!vol.b_Removed)
		{
			vol.b_Removed = true;
			m_RemovedVolumes.push_back(handle);
		}
	//}
}

BVHVolume& BVH::operator[](size_t handle)
{
	return m_VolumePool[handle];
}

void BVH::UpdateTree()
{
	//remove nodes
	for (size_t ri : m_RemovedVolumes)
	{
		RemoveNodeMainThread(ri);
	}
	m_RemovedVolumes.clear();

	for (size_t fi : m_FreedVolumes)
	{
		FreeVolumeImpl(fi);
	}
	m_FreedVolumes.clear();

	//insert nodes
	for (size_t ii : m_InsertedVolumes)
	{
		InsertNodeMainThread(ii);
	}
	m_InsertedVolumes.clear();
}


void BVH::RemoveNodeMainThread(size_t remove_node)
{
	BVHVolume* remove_vol = &m_VolumePool[remove_node];
	remove_vol->b_Removed = false;
	if (remove_node == m_Root)
	{
		m_Root = SIZE_T_MAX;
		return;
	}
	size_t parent_id = remove_vol->p_Parent;
	BVHVolume* parent_vol = &m_VolumePool[parent_id];
	{
		size_t other_children = parent_vol->p_Children[0];
		if (other_children == remove_node)
		{
			other_children = parent_vol->p_Children[1];
		}
		if (parent_vol->p_Parent == SIZE_T_MAX)
		{
			m_Root = other_children;
			m_VolumePool[other_children].p_Parent = SIZE_T_MAX;
			FreeVolume(parent_id);
			parent_vol = nullptr;
			parent_id = SIZE_T_MAX;
		}
		else
		{
			size_t new_parent_id = parent_vol->p_Parent;
			BVHVolume* new_parent = &m_VolumePool[new_parent_id];
			size_t slot = new_parent->p_Children[0] == parent_id ? 0 : 1;
			Parenting(new_parent_id, other_children, slot);
			FreeVolume(parent_id);
			parent_vol = new_parent;
			parent_id = new_parent_id;
		}
	}


	while (parent_vol != nullptr)
	{
		if (parent_vol->isFull())
		{
			BVHVolume* left_child = &m_VolumePool[parent_vol->p_Children[0]];
			BVHVolume* right_child = &m_VolumePool[parent_vol->p_Children[1]];
			parent_vol->box = AABB::Union(left_child->box, right_child->box);
			if (parent_vol->p_Parent != SIZE_T_MAX)
			{
				parent_id = parent_vol->p_Parent;
				parent_vol = &m_VolumePool[parent_id];
			}
			else
			{
				parent_id = SIZE_T_MAX;
				parent_vol = nullptr;
			}
		}
		else
		{
			parent_id = SIZE_T_MAX;
			parent_vol = nullptr;
		}
	}
}

void BVH::InsertNodeMainThread(size_t insert_node)
{
	BVHVolume* inserted_vol = &m_VolumePool[insert_node];
	if (!inserted_vol->b_Active) { return; }
	inserted_vol->b_Insert = false;
	if (m_Root == SIZE_T_MAX)
	{
		m_Root = insert_node;
		return;
	}

	size_t current_node = m_Root;
	BVHVolume* current_vol = &m_VolumePool[current_node];

	while(!current_vol->isLeaf())
	{
		//if one child is empty, set volume as child
		if (current_vol->p_Children[0] == SIZE_T_MAX)
		{
			Parenting(current_node, insert_node, 0);
			break;
		}
		else if (current_vol->p_Children[1] == SIZE_T_MAX)
		{
			Parenting(current_node, insert_node, 1);
			break;
		}
		//else traverse child
		else
		{
			size_t best_child = SIZE_T_MAX;
			float min_surface = std::numeric_limits<float>::infinity();
			for (size_t child_id : current_vol->p_Children)
			{
				BVHVolume* child_volume = &m_VolumePool[child_id];
				AABB union_b = AABB::Union(child_volume->box, inserted_vol->box);
				float area = union_b.Area();
				if (area < min_surface)
				{
					best_child = child_id;
					min_surface = area;
				};
			}
			if (best_child != SIZE_T_MAX)
			{
				current_node = best_child;
				current_vol = &m_VolumePool[best_child];
			}
			else
			{
				break;
			}
		}
	}
	if (current_vol->isLeaf())
	{
		size_t new_vol_id = AllocVolume();
		//vector may be re allocated, retarget pointers
		current_vol = &m_VolumePool[current_node];
		inserted_vol = &m_VolumePool[insert_node];
		BVHVolume& new_vol = operator[](new_vol_id);
		size_t old_parent = current_vol->p_Parent;
		//current vol has parent
		if (old_parent != SIZE_T_MAX)
		{
			size_t old_child_id = 0;
			if (current_node == m_VolumePool[old_parent].p_Children[1])
			{
				old_child_id = 1;
			}
			Parenting(old_parent, new_vol_id, old_child_id);
		}
		//current vol is root
		else
		{
			m_Root = new_vol_id;
		}
		Parenting(new_vol_id, current_node, 0);
		Parenting(new_vol_id, insert_node, 1);
	}

	size_t traverse_parent = inserted_vol->p_Parent;
	while (traverse_parent != SIZE_T_MAX)
	{
		BVHVolume* traverse_vol = &m_VolumePool[traverse_parent];
		if (traverse_vol->isFull())
		{
			traverse_vol->box = AABB::Union(m_VolumePool[traverse_vol->p_Children[0]].box, m_VolumePool[traverse_vol->p_Children[1]].box);
		}
		else
		{
			for (size_t child_id : traverse_vol->p_Children)
			{
				if (child_id != SIZE_T_MAX)
				{
					traverse_vol->box = m_VolumePool[child_id].box;
					break;
				}
			}
		}
		traverse_parent = traverse_vol->p_Parent;
	}
}

void BVH::Parenting(size_t parent, size_t child, size_t child_id)
{
	m_VolumePool[parent].p_Children[child_id] = child;
	m_VolumePool[child].p_Parent = parent;
}
