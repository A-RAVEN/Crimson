#pragma once

namespace graphics_backend
{
	enum class UploadingResourceType
	{
		eMemoryDataThisFrame = 0,
		eMemoryDataLowPriority,
		eAddressDataThisFrame,
	};


}