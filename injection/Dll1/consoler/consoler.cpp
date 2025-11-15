
// this is basic cout dll
//#include <windows.h>
//#include <iostream>
//
//int main()
//{
//    std::cout << "?? Trying to load the DLL..." << std::endl;
//
//    // Load the DLL (make sure Dll1.dll is in the same folder as the EXE)
//    HINSTANCE hDLL = LoadLibraryA("Dll1.dll");
//
//    if (hDLL != nullptr)
//    {
//        std::cout << "? DLL loaded successfully!" << std::endl;
//
//        // Unload the DLL
//        FreeLibrary(hDLL);
//        std::cout << "? DLL unloaded successfully!" << std::endl;
//    }
//    else
//    {
//        std::cout << "? Failed to load DLL!" << std::endl;
//    }
//
//    std::cout << "Press Enter to exit..." << std::endl;
//    std::cin.get(); // keeps window open
//    return 0;
//}


//this is for dll injection of function
//#include <windows.h>
//#include <iostream>
//
//typedef void (CALLBACK* LPFNSAYS)();  // Function pointer type for SayHello()
//
//int main()
//{
//    std::cout << "?? Trying to load the DLL..." << std::endl;
//
//    // Load the DLL (make sure Dll1.dll is in the same folder as the EXE)
//    HINSTANCE hDLL = LoadLibraryA("Dll1.dll");
//
//    if (hDLL != nullptr)
//    {
//        std::cout << "? DLL loaded successfully!" << std::endl;
//
//        // Get the address of the SayHello function
//        LPFNSAYS lpfnSayHello = (LPFNSAYS)GetProcAddress(hDLL, "SayHello");
//
//        if (lpfnSayHello != nullptr)
//        {
//            // Call the SayHello function
//            std::cout << "Calling SayHello() from DLL..." << std::endl;
//            lpfnSayHello();  // Call the function
//        }
//        else
//        {
//            std::cerr << "? Failed to find SayHello function in DLL!" << std::endl;
//        }
//
//        // Unload the DLL
//        FreeLibrary(hDLL);
//        std::cout << "? DLL unloaded successfully!" << std::endl;
//    }
//    else
//    {
//        std::cerr << "? Failed to load DLL!" << std::endl;
//    }
//
//    std::cout << "Press Enter to exit..." << std::endl;
//    std::cin.get(); // keeps window open
//    return 0;
//}


//#include<windows.h>
//#include<iostream>
//
//typedef void(CALLBACK* LPFNSAYS) ();
//
//int main() {
//	HINSTANCE hdll = LoadLibraryA("Dll1.dll");
//	if (hdll != nullptr) {
//		LPFNSAYS lpfunsays = (LPFNSAYS)GetProcAddress(hdll, "funnyFunction");
//		if (lpfunsays != nullptr) {
//			std::cout << "success\n";
//			lpfunsays();
//		}
//		else {
//			std::cerr << "error here in dll load\n";
//		}
//		FreeLibrary(hdll);
//		std::cout << "dll unload is sucessfull\n";
//	}
//	
//	
//}



#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>

#define TARGET_PROCESS L"notepad.exe"  // Case-insensitive usually
constexpr const WCHAR* DLL_PATH = L"C:\\Users\\test\\Pictures\\injection\\Dll1\\x64\\Debug\\Dll1.dll";

DWORD FindProcessIdByName(const WCHAR* processName);

int main() {
    DWORD pid = FindProcessIdByName(TARGET_PROCESS);
    if (pid == 0) {
        std::cerr << "Process not found: " << std::wstring(TARGET_PROCESS).c_str() << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "Found process PID: " << pid << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        system("pause");
        return 1;
    }

    // Use LoadLibraryW for Unicode path
    SIZE_T dllPathSize = (wcslen(DLL_PATH) + 1) * sizeof(WCHAR);

    LPVOID pRemoteBuffer = VirtualAllocEx(hProcess, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteBuffer) {
        std::cerr << "VirtualAllocEx failed. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    if (!WriteProcessMemory(hProcess, pRemoteBuffer, DLL_PATH, dllPathSize, NULL)) {
        std::cerr << "WriteProcessMemory failed. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    LPTHREAD_START_ROUTINE pLoadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");

    if (!pLoadLibraryW) {
        std::cerr << "Failed to get LoadLibraryW address." << std::endl;
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibraryW, pRemoteBuffer, 0, NULL);
    if (hThread) {
        std::cout << "DLL injection successful! Thread created." << std::endl;
        WaitForSingleObject(hThread, 5000); // Wait up to 5 sec
        CloseHandle(hThread);
    }
    else {
        std::cerr << "CreateRemoteThread failed. Error: " << GetLastError() << std::endl;
    }

    // Cleanup
   /* VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);*/
   /* CloseHandle(hProcess);
    system("pause");*/
    return 0;
}

DWORD FindProcessIdByName(const WCHAR* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (_wcsicmp(pe32.szExeFile, processName) == 0) {  // Case-insensitive
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}