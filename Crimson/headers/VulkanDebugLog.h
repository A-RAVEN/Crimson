#pragma once
#include "VulkanHeader.h"
#include <string>
#include <iostream>
#include <sstream>
#include <assert.h>

namespace Crimson
{
	namespace VulkanDebug
	{

#define CHECK_VKRESULT( vkresult , _error_string ) {VulkanDebug::CheckVKResultWithLine(vkresult, _error_string, __LINE__, __FILE__);} 

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

		static inline void CheckVKResultWithLine(VkResult result, std::string const& error_string, int line, std::string const& file)
		{
#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
			if (result != VK_SUCCESS)
			{
				std::string out = "Vulkan Runtime Error:\n" + error_string + "\nLine: " + std::to_string(line) + "\nFile: " + file + "\n";
				std::cerr << out << std::endl;
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

		//new Vulkan Debug Callback
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData)
		{
				char prefix[64];
				char* message = (char*)malloc(strlen(callbackData->pMessage) + 500);
				assert(message);
				if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
					strcpy(prefix, "VERBOSE : ");
				}
				else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
					strcpy(prefix, "INFO : ");
				}
				else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
					strcpy(prefix, "WARNING : ");
				}
				else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
					strcpy(prefix, "ERROR : ");
				}
				if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
					strcat(prefix, "GENERAL");
				}
				else {
					if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
						strcat(prefix, "PERF");
					}
				}
				sprintf(message,
					"%s - Message ID Number %d, Message ID String :\n%s",
					prefix,
					callbackData->messageIdNumber,
					callbackData->pMessageIdName,
					callbackData->pMessage);
				if (callbackData->objectCount > 0) {
					//char tmp_message[500];
					//sprintf(tmp_message, "\n Objects - %d\n", callbackData->objectCount);
					//strcat(message, tmp_message);
					//for (uint32_t object = 0; object < callbackData->objectCount; ++object) {
					//	sprintf(tmp_message,
					//		" Object[%d] - Type %s, Value %p, Name \"%s\"\n",
					//		Object,
					//		DebugAnnotObjectToString(
					//			callbackData->pObjects[object].objectType),
					//			(void*)(callbackData->pObjects[object].objectHandle),
					//		callbackData->pObjects[object].pObjectName);
					//	strcat(message, tmp_message);
					//}
				}
				if (callbackData->cmdBufLabelCount > 0) {
					//char tmp_message[500];
					//sprintf(tmp_message,
					//	"\n Command Buffer Labels - %d\n",
					//	callbackData->cmdBufLabelCount);
					//strcat(message, tmp_message);
					//for (uint32_t label = 0; label < callbackData->cmdBufLabelCount; ++label) {
					//	sprintf(tmp_message,
					//		" Label[%d] - %s { %f, %f, %f, %f}\n",
					//		Label,
					//		callbackData->pCmdBufLabels[label].pLabelName,
					//		callbackData->pCmdBufLabels[label].color[0],
					//		callbackData->pCmdBufLabels[label].color[1],
					//		callbackData->pCmdBufLabels[label].color[2],
					//		callbackData->pCmdBufLabels[label].color[3]);
					//	strcat(message, tmp_message);
					//}
				}
				printf("%s\n", message);
				fflush(stdout);
				free(message);
				// Don't bail out, but keep going.
				return false;
		}
	}
}