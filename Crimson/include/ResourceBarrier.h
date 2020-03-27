#pragma once
#include "Generals.h"
#include <set>
namespace Crimson
{
	class ResourceBarrierBatch
	{
	public:
		std::set<PGPUImage> m_ReferencedImages;
		std::set<PGPUBuffer> m_ReferencedBuffers;
		//src stages
		
		//dst stages

		//src access types

		//dst access types

		//src image layout

		//dst image layout
	};
}