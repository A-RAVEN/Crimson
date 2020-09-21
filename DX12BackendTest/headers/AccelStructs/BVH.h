#pragma once
#include<headers/AccelStructs/AABB.h>
#include <vector>
#include <deque>
#include <set>
#include <array>

class BVHVolume
{
public:
	AABB box;
	uint64_t m_UserData;
	BVHVolume() : box(), m_UserData(0), p_Parent((std::numeric_limits<size_t>::max)()), p_Children{ (std::numeric_limits<size_t>::max)(), (std::numeric_limits<size_t>::max)() }, b_Insert(false), b_Removed(false), b_Active(false){}
	bool isLeaf() { return p_Children[0] == -1 && p_Children[1] == -1; }
	bool isFull(){return p_Children[0] != -1 && p_Children[1] != -1;}
private:
	friend class BVH;
	size_t p_Parent;
	std::array<size_t, 2> p_Children;
	void Reset() {
		p_Parent = (std::numeric_limits<size_t>::max)(); p_Children = { (std::numeric_limits<size_t>::max)(), (std::numeric_limits<size_t>::max)() }; m_UserData = 0;
		b_Insert = b_Removed = b_Active = false;
	}

	bool b_Insert;
	bool b_Removed;
	bool b_Active;
};

class BVH
{
public:
	BVH() : m_Root(-1) {};

	size_t AllocVolume();
	void FreeVolume(size_t handle);
	void VolumeDirty(size_t handle);
	BVHVolume& operator[](size_t handle);

	void UpdateTree();

	std::set<size_t> const& GetActiveVolumes() {
		return m_ActiveVolumes;
	}
private:
	void FreeVolumeImpl(size_t handle);
	void RemoveNodeMainThread(size_t remove_node);
	void InsertNodeMainThread(size_t insert_node);
	void Parenting(size_t parent, size_t child, size_t child_id);

	std::vector<BVHVolume> m_VolumePool;
	std::deque<size_t> m_AvailableVolumes;

	std::deque<size_t> m_InsertedVolumes;
	std::deque<size_t> m_RemovedVolumes;
	std::deque<size_t> m_FreedVolumes;
	std::set<size_t> m_ActiveVolumes;

	size_t m_Root;
};