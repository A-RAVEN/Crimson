#pragma once

namespace thread_management
{
	class IGraphComponent
	{
	public:
		virtual void AddDependent(IGraphComponent* dependent) = 0;
		virtual void AddSuccessor(IGraphComponent* successor) = 0;
		virtual void TryDecCounter() = 0;
	};
}