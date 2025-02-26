#include <iostream>
#include <windows.h>
#include <string>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "No DLL file provided." << std::endl;
		return 1;
	}

	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	std::wstring exeName = std::wstring(exePath);
	size_t pos = exeName.find_last_of(L"\\/");
	if (pos != std::wstring::npos) {
		exeName = exeName.substr(pos + 1);
	}

	pos = exeName.find_last_of(L".");
	if (pos != std::wstring::npos) {
		exeName = exeName.substr(0, pos);
	}

	HMODULE hModule = LoadLibraryA(argv[1]);
	if (hModule == NULL) {
		std::cerr << "Failed to load DLL: " << argv[1] << std::endl;
		return 1;
	}

	std::wcout << L"Loaded DLL: " << argv[1] << L" into process: " << exeName << std::endl;

	// Free the DLL module
	FreeLibrary(hModule);

	return 0;
}