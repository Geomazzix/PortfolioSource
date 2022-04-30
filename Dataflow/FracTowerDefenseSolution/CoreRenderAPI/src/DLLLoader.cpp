#include "CoreRenderAPI/pch.h"

#include "CoreRenderAPI/DLLLoader.h"

#include <cstdlib>







#if defined(PLATFORM_SWITCH)
#define TOR_API 
#elif defined(PLATFORM_WINDOWS)
#ifdef TOR_API_EXPORTS
#define TOR_API __declspec(dllexport)
#else
#define TOR_API __declspec(dllimport)
#endif
#endif

#if defined(PLATFORM_WINDOWS)
	bool TOR::GetLastErrorString(char* buffer, DWORD length)
	{
		auto err = GetLastError();

		if (!err)
			return false;

		// Assuming buffer is large enough for any error message.
		// Otherwise, use FORMAT_MESSAGE_ALLOCATE_BUFFER and LocalFree!
		auto ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			buffer,
			length,
			NULL);

		return ret > 0;
	}

	

FARPROC TOR::GetSymbol(DLLModule module, const char* symbol)
{
	return GetProcAddress(module, symbol);
}

#endif

#if defined (PLATFORM_SWITCH)
uintptr_t TOR::GetSymbol(TOR::DLLModule module, const char* symbol){
	uintptr_t functionPtr;
	nn::ro::LookupModuleSymbol(&functionPtr,&module,symbol);
	return functionPtr;
}
#endif
size_t TOR::ReadAll(void* pOut, size_t bufferSize, const char* path)
{
	#if defined(PLATFORM_SWITCH)
	nn::Result result;
	nn::fs::FileHandle file;
	result = nn::fs::OpenFile(&file, path, nn::fs::OpenMode_Read);
	NN_ASSERT(result.IsSuccess());

	int64_t fileSize;
	result = nn::fs::GetFileSize(&fileSize, file);
	NN_ASSERT(result.IsSuccess());
	NN_ASSERT_LESS(fileSize, static_cast<int64_t>(bufferSize));

	size_t readSize;
	result = nn::fs::ReadFile(&readSize, file, 0, pOut, bufferSize);
	NN_ASSERT(result.IsSuccess());
	NN_ASSERT_EQUAL(static_cast<int64_t>(readSize), fileSize);

	nn::fs::CloseFile(file);

	return readSize;
	#elif defined(PLATFORM_WINDOWS)
	return 0;
	#endif
	return 0;
	
}

std::string TOR::GetDLLRootPath()
{
	// Path is hardcoded as it doesn't have access to the engine's wildcard system
#if defined (CONFIG_DEBUG) // Debug
#if defined (PLATFORM_WINDOWS) // x64
	return "dll/OpenGLRenderer/";
#elif defined (PLATFORM_SWITCH) // NX64
	return "rom:/bin/Debug/NX64/";
#endif

#elif defined (CONFIG_DEVELOP) // Develop
#if defined (PLATFORM_WINDOWS) // x64
	return "dll/OpenGLRenderer/";
#elif defined (PLATFORM_SWITCH) // NX64
	return "rom:/bin/Develop/NX64/";
#endif

#else // Release
#if defined (PLATFORM_WINDOWS) // x64
	return "dll/OpenGLRenderer/";
#elif defined (PLATFORM_SWITCH) // NX64
	return "rom:/bin/Release/NX64/";
#endif
#endif
}

const TOR::DLLModule& TOR::LoadTORModule(const char* dllName)
{
	#if defined(PLATFORM_SWITCH)
	nn::ro::Initialize();
	// Load the NRR file.
	nrr = aligned_alloc(nn::os::MemoryPageSize, MaxFileSize);
	std::string nrrPath;

	nrrPath = GetDLLRootPath() + "dataSrc/.nrr/TDGame.nrr"; // hardcoded TDGame.nrr here, if unit test project needs to use dll loader this needs to be adressed.
	nrrSize = ReadAll(nrr, MaxFileSize, nrrPath.c_str());
	// Register the NRR file.
	result = nn::ro::RegisterModuleInfo(&info, nrr);
	NN_ASSERT(result.IsSuccess());
	nro = aligned_alloc(nn::os::MemoryPageSize, MaxFileSize);
	std::string dllPath = GetDLLRootPath() + "dataSrc/nro/" + std::string(dllName) + ".nro";
	imageSize = ReadAll(nro, MaxFileSize, dllPath.c_str());
	result = nn::ro::GetBufferSize(&bufferSize, nro);
	NN_ASSERT(result.IsSuccess());
	if (bufferSize != 0)
	{
		bss = aligned_alloc(nn::os::MemoryPageSize, bufferSize);
	}
	else
	{
		bss = 0;
	}
	
	// Load the NRO file (delayed resolution for the symbols).
	result = nn::ro::LoadModule(&module, nro, bss, bufferSize, nn::ro::BindFlag_Lazy);
	NN_ASSERT(result.IsSuccess());
	// You can now call module functions!
	return module;
	#elif defined(PLATFORM_WINDOWS)
	// try the development folder structure first
	std::string dllpath = GetDLLRootPath();
	
	dllpath += dllName;
	module = LoadLibraryA(dllpath.c_str());
	if (!module)
	{
		char errstr[1024] = "FormatMessage failed (unknown error code?)";
		// Print informative info:
		if (GetLastErrorString(errstr, sizeof(errstr)))
			printf("Failed to load %s: %s\tTrying local DLL\n", dllpath.c_str(), errstr);

		// see if the dll is perhaps in the current folder
		module = LoadLibraryA(dllName);
		if (!module)
		{
			GetLastErrorString(errstr, sizeof(errstr));
			printf("Unable to open %s: %s", dllName, errstr);
		}
	}
	return module;

	#endif
}

void TOR::UnloadModule()
{
	#if defined(PLATFORM_SWITCH)
	nn::ro::UnloadModule(&module);
	free(nro);
	free(bss);
	nn::ro::UnregisterModuleInfo(&info);
	nn::ro::Finalize();
	#elif defined(PLATFORM_WINDOWS)
	// goes automatically when a module goes out of scope?
	#endif
}
