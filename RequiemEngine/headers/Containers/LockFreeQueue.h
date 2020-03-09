#pragma once
#include <atomic>


template <typename T>
class LockFreeQueue
{
public:
	struct LockFreeNode
	{
		T m_Data;
		LockFreeNode* p_Next;
	};
	bool TryPush(T const& value)
	{
		LockFreeNode new_node = {value, nullptr};
		if(std::atomic_compare_exchange_weak)
	}
private:

};