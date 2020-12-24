#pragma once


#ifdef MC_PLATFORM_WINDOWS
	#ifdef MC_BUILD_DLL
		#define MIMIC_API __declspec(dllexport)
	#else
		#define MIMIC_API __declspec(dllimport)
	#endif
#else 
	#error Mimic only supports Windows

#endif // MC_PLATFORM_WINDOWS
