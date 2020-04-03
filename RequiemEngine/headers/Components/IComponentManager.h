#pragma once
#include <vector>
#include <deque>
#include <headers/Components/IComponent.h>
#include <headers/Debug.h>
class IComponentManager
{
public:
	template <typename T>
	T* GetComponent(uint32_t id) {
		static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
		return static_cast<T*>(GetIComponent(id));
	}

	IComponent* GetComponent(uint32_t id) {
		return GetIComponent(id);
	}

	virtual size_t AllocateComponentId() = 0;
	virtual size_t FreeComponentId(size_t id) = 0;
protected:
	virtual IComponent* GetIComponent(uint32_t id) = 0;
};

template <typename T>
class BuiltInComponentManager : public IComponentManager
{
public:
	static_assert(std::is_base_of<IComponent, T>::value, "type T should be derived from class IComponent!");
	BuiltInComponentManager()
	{
	}

	static std::string GetComponentName() { return std::string(typeid(T).name()); }

	virtual size_t AllocateComponentId() override
	{
		if (!m_AvailableComponentIds.empty())
		{
			size_t return_val = m_AvailableComponentIds.front();
			m_AvailableComponentIds.pop_front();
			return return_val;
		}
		m_Components.push_back(T{});
		return m_Components.size() - 1;
	}

	virtual size_t FreeComponentId(size_t id) override
	{
		m_AvailableComponentIds.push_back(id);
	}
protected:
	virtual IComponent* GetIComponent(uint32_t id) override
	{
		RE_ASSERT(id < m_Components.size(), std::string("invalid component id ") + typeid(T).name());
		return &m_Components[id];
	}
private:
	std::vector<size_t> m_WorkingComponents;
	std::deque<size_t> m_AvailableComponentIds;
	std::vector<T> m_Components;
};