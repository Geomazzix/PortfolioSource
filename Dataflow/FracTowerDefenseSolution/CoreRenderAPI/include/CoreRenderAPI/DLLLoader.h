#pragma once
namespace TOR
{

#if defined(PLATFORM_WINDOWS)


	static HMODULE module;
	typedef HMODULE DLLModule;
	bool GetLastErrorString( char* buffer, DWORD length);
	FARPROC GetSymbol(DLLModule module, const char* symbol);
#elif defined(PLATFORM_SWITCH)			
	static nn::Result result;
	static const size_t MaxFileSize = 0x400000;
	static void* nro;
	static void* bss;
	static void* nrr;
	static size_t imageSize;
	static size_t bufferSize;
	static size_t nrrSize;
	static nn::ro::Module module;
	static nn::ro::RegistrationInfo info;
	typedef nn::ro::Module DLLModule;
	uintptr_t GetSymbol(DLLModule module, const char* symbol);
	size_t ReadAll(void* pOut, size_t bufferSize, const char* path);
#endif
	std::string GetDLLRootPath();
	const DLLModule& LoadTORModule(const char* dllName);
	void UnloadModule();
	size_t ReadAll(void* pOut, size_t bufferSize, const char* path);

}