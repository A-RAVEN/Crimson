#pragma once
#include "VulkanHeader.h"
#include <string>
#include <iostream>
#include <sstream>

namespace Crimson
{
	namespace VulkanDebug
	{
		//Vulkan Result Checker
		static inline void CheckVKResult(VkResult result, std::string const& error_string)
		{
#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
			if (result != VK_SUCCESS)
			{
				std::cerr << "Vulkan Runtime Error:\n" << error_string << std::endl;
			}
#ifdef _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
		}
		//Vulkan Debug Callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL
			VulkanDebugCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT obj_type,
				uint64_t src_obj,
				size_t location,
				int32_t msg_code,
				const char* layer_prefix,
				const char* msg,
				void* user_data
			)
		{
			std::ostringstream stream;
			stream << "VULKAN_DEBUG_";
			if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			{
				stream << "INFO: ";
			}
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			{
				stream << "WARNING: ";
			}
			if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			{
				stream << "PERFORM_WARNING: ";
			}
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			{
				stream << "ERROR: ";
			}
			if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			{
				stream << "DEBUG: ";
			}

			stream << "(" << layer_prefix << ")";
			stream << "ObjectID: " << src_obj << std::endl;
			stream << msg << std::endl;
#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
			std::cerr << stream.str().c_str() << std::endl;
#ifdef _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
			return false;
		}
	}
}