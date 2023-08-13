#pragma once
#define CA_LIBRARY_API __declspec(dllexport)

#define CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(TInterface, TImplemented)\
extern "C"\
{\
	CA_LIBRARY_API TInterface* NewModuleInstance()\
	{\
		return new TImplemented();\
	}\
	CA_LIBRARY_API void DeleteModuleInstance(TInterface* instance)\
	{\
		delete static_cast<TImplemented*>(instance);\
	}\
}