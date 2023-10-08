#pragma once
#include <mutex>
#include <deque>
#include <SharedTools/header/DebugUtils.h>
#include <ThreadManager/header/ThreadManager.h>
#include <unordered_set>
#include "RenderBackendSettings.h"
#include "VulkanApplicationSubobjectBase.h"

namespace graphics_backend
{
	class ITickingResourceUpdator;

	class BaseTickingUpdateResource : public BaseApplicationSubobject
	{
	public:
		BaseTickingUpdateResource(CVulkanApplication& app);
		void SetOwningUpdator(ITickingResourceUpdator* owningUpdator);
		ITickingResourceUpdator* GetOwningUpdator() const;
		void ResetResource();
		bool IsValid() const;
		bool IsDirty() const;
		bool UploadingDone() const;
		virtual void TickUpload() = 0;
		void MarkDirtyThisFrame();
	protected:
		void MarkUploadingDoneThisFrame();
	private:
		bool b_Dirty = false;
		ITickingResourceUpdator* m_OwningUpdator = nullptr;
		FrameType m_SubmitFrame = INVALID_FRAMEID;
	};

	class ITickingResourceUpdator
	{
	public:
		virtual void EnqueueTickingResource(BaseTickingUpdateResource* resource) = 0;
	};

	template<typename T>
	class TTickingUpdateResourcePool :  public ITickingResourceUpdator
	{
	public:
		TTickingUpdateResourcePool() = delete;
		TTickingUpdateResourcePool(TTickingUpdateResourcePool const& other) = delete;
		TTickingUpdateResourcePool& operator=(TTickingUpdateResourcePool const&) = delete;
		TTickingUpdateResourcePool(TTickingUpdateResourcePool&& other) = delete;
		TTickingUpdateResourcePool& operator=(TTickingUpdateResourcePool&&) = delete;

		TTickingUpdateResourcePool(CVulkanApplication& owner) :
			m_Owner(owner)
		{
		}

		virtual ~TTickingUpdateResourcePool()
		{
			CA_ASSERT(IsEmpty(), std::string{"Vulkan Application Pointer Pool Is Not Released Before Destruct: "} + CA_CLASS_NAME(T));
		}

		template<typename...TArgs>
		T* Alloc(TArgs&&...Args)
		{
			std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);
			T* result = nullptr;
			if (m_EmptySpaces.empty())
			{
				m_Pool.emplace_back(m_Owner);
				result = &m_Pool.back();
			}
			else
			{
				result = m_EmptySpaces.front();
				m_EmptySpaces.pop_front();
			}
			result->SetOwningUpdator(this);
			result->Initialize(std::forward<TArgs>(Args)...);
			result->MarkDirtyThisFrame();
			return result;
		}

		void Release(T* releaseObj)
		{
			CA_ASSERT(releaseObj != nullptr, std::string{"Try Release nullptr: "} + CA_CLASS_NAME(T));
			std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);
			releaseObj->Release();
			releaseObj->ResetResource();
			m_EmptySpaces.push_back(releaseObj);
		}

		void ReleaseAll()
		{
			std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);

			std::unordered_set<T*> emptySet;
			for (T* emptyPtr : m_EmptySpaces)
			{
				emptySet.insert(emptyPtr);
			}

			for (size_t i = 0; i < m_Pool.size(); ++i)
			{
				if (emptySet.find(&m_Pool[i]) == emptySet.end())
				{
					m_Pool[i].Release();
				}
			}
			emptySet.clear();
			m_Pool.clear();
			m_EmptySpaces.clear();
			m_DirtyResources.clear();
		}

		template<typename...TArgs>
		std::shared_ptr<T> AllocShared(TArgs&&...Args)
		{
			return std::shared_ptr<T>(Alloc(std::forward<TArgs>(Args)...), [this](T* releaseObj) { Release(releaseObj); });
		}

		bool IsEmpty()
		{
			std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);
			return m_EmptySpaces.size() == m_Pool.size();
		}

		void TickUpload(thread_management::CTaskGraph* pWorkingGraph)
		{
			pWorkingGraph->SetupFunctor
			([this](thread_management::CTaskGraph* thisGraph) 
			{
				std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);
				auto dirtyResources = std::move(m_DirtyResources);
				m_DirtyResources.clear();
				thisGraph->NewTaskParallelFor()
				->JobCount(dirtyResources.size())
				->Functor
				([this, dirtyResources](size_t index)
				{
					if (dirtyResources[index]->IsValid())
					{
						auto pResource = dirtyResources[index];
						pResource->TickUpload();
						if (pResource->IsDirty())
						{
							EnqueueTickingResource(pResource);
						}
					}
				});
			});
		}

		virtual void EnqueueTickingResource(BaseTickingUpdateResource* resource) override
		{
			CA_ASSERT(resource != nullptr && resource->GetOwningUpdator() == this, "Invalid Ticking Resource Enqueue");
			T* pResource = dynamic_cast<T*>(resource);
			std::lock_guard<std::recursive_mutex> lockGuard(m_Mutex);
			m_DirtyResources.push_back(pResource);
		}

	private:
		CVulkanApplication& m_Owner;
		std::recursive_mutex m_Mutex;
		std::deque<T> m_Pool;
		std::deque<T*> m_EmptySpaces;
		std::deque<T*> m_DirtyResources;
	};
}