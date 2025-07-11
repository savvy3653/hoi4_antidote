#include "rat_.h"

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#include <iostream>
#include <fstream>
#include <thread>
#include <boost/dll/runtime_symbol_info.hpp>
#include <TlHelp32.h>
#include <chrono>

#ifdef UNICODE
std::wstring hoiExe = L"hoi4.exe";
#else
std::string hoiExe = "hoi4.exe";
#endif

bool Rat::registry_loading(const std::wstring& exePath) {
    HKEY hKey = NULL;
    LONG createStatus = RegCreateKeyW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
    if (createStatus != ERROR_SUCCESS) {
        return false;
    }

    LONG status = RegSetValueExW(
            hKey,
            L"notrat",
            0, REG_SZ,
            const_cast<BYTE*>(reinterpret_cast<const BYTE*>(exePath.c_str())),
            (exePath.size()+1) * sizeof(wchar_t));
    if (status != ERROR_SUCCESS) {
        return false;
    }

    return true;
}

std::wstring Rat::get_path() {
    std::wstring path = L"\"";
    path += boost::dll::program_location().wstring();
    path += L"\"";
    return path;
}

bool Rat::create_message_box() {
    int msgboxID = MessageBoxW(
            NULL,
            L"Failed to load DLC: \"Together for Victory\"\n"
            L"Reason: checksum mismatch.\n"
            L"File: dlc_together_for_victory.pack\n\n"
            L"Please verify the DLC integrity via Steam or reinstall it.",
            L"hoi4 DLC Error",
            MB_ICONERROR | MB_OK
            );
    if (msgboxID == 0) {
        return false;
    }
    return true;
}

bool Rat::checking_hoi4() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe { sizeof(pe) };
    if (!Process32First(hProcessSnap, &pe)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        if (pe.szExeFile == hoiExe) {
            const auto proc = OpenProcess(PROCESS_TERMINATE, false, pe.th32ProcessID);
            if (proc != NULL && proc != INVALID_HANDLE_VALUE) {
                TerminateProcess(proc, 1);
                CloseHandle(proc);
            }
            CloseHandle(hProcessSnap);
            create_message_box();
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe));

    CloseHandle(hProcessSnap);
    return false;
}

bool Rat::deactivate() {
    // deleting from registry
    HKEY hKey = NULL;
    LONG openStatus = RegOpenKeyExW(HKEY_CURRENT_USER,
                                      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                      0, KEY_SET_VALUE | KEY_WOW64_64KEY,
                                      &hKey);
    if (openStatus != ERROR_SUCCESS) {
        return false;
    }
    openStatus = RegDeleteValueW(hKey, L"notrat");
    RegCloseKey(hKey);


    // disabling process
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe { sizeof(pe) };
    if (!Process32First(hProcessSnap, &pe)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        if (pe.szExeFile == "notrat.exe") {
            const auto proc = OpenProcess(PROCESS_TERMINATE, false, pe.th32ProcessID);
            if (proc != NULL && proc != INVALID_HANDLE_VALUE) {
                TerminateProcess(proc, 1);
                CloseHandle(proc);
            }
            CloseHandle(hProcessSnap);
            create_message_box();
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe));

    CloseHandle(hProcessSnap);
    return false;
}

bool Rat::is_disable_requested() {
    return  (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
            (GetAsyncKeyState(VK_F12) & 0x8000);
}

std::expected<bool, std::string> Rat::start() {
    if (!ShowWindow(GetConsoleWindow(), SW_HIDE)) {
        return std::unexpected("Failed to hide window.");
    }

    // Loading notRat to registry
    const std::wstring exePath = get_path();

    if (!registry_loading(exePath)) {
        return std::unexpected("Registry loading failed.");
    }

    // Checking if hoi4 is loaded (main process)
    auto monitoring = [this]() -> std::expected<bool, std::string> {
        using namespace std::chrono_literals;
        while (true) {
            bool found = checking_hoi4();
            if (is_disable_requested()) {
                if (!deactivate()) {
                    return std::unexpected("Deactivation failed.");
                }
            }
            std::this_thread::sleep_for(100ms);
        }
    };

    std::thread t(monitoring);
    t.join();

    return true;
}