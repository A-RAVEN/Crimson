#pragma once
#define NOMINMAX
#include <windows.h>
#include <string>

namespace library_loader
{
	template<typename TModInstance>
	class TModuleLoader
	{
	private:
		typedef TModInstance* (*FTP_NewModuleObject)();
		typedef void(*FPT_DeleteModuleObject)(TModInstance*);

		HINSTANCE hModuleLib = nullptr;
		FTP_NewModuleObject pNewInstanceFunc = nullptr;
		FPT_DeleteModuleObject pDeleteInstanceFunc = nullptr;
	public:
		TModuleLoader(
#if UNICODE
			std::wstring const& modulePath
#else
			std::string const& moduelPath
#endif
		)
		{
			hModuleLib = LoadLibrary(modulePath.c_str());
			if (hModuleLib != nullptr)
			{
				pNewInstanceFunc = reinterpret_cast<FTP_NewModuleObject>(GetProcAddress(hModuleLib, "NewModuleInstance"));
				pDeleteInstanceFunc = reinterpret_cast<FPT_DeleteModuleObject>(GetProcAddress(hModuleLib, "DeleteModuleInstance"));
			}
		}

		~TModuleLoader()
		{
			pNewInstanceFunc = nullptr;
			pDeleteInstanceFunc = nullptr;
			if (hModuleLib != nullptr)
			{
				FreeLibrary(hModuleLib);
				hModuleLib = nullptr;
			}
		}

		std::shared_ptr<TModInstance> New()
		{
			return std::shared_ptr<TModInstance>(NewModuleInstance(), [this](TModInstance* removingInstance) { DeleteModuleInstance(removingInstance); });
		}

		TModInstance* NewModuleInstance()
		{
			return pNewInstanceFunc();
		}

		void DeleteModuleInstance(TModInstance* moduleObject)
		{
			if (pDeleteInstanceFunc != nullptr)
			{
				pDeleteInstanceFunc(moduleObject);
			}
		}
	};
}