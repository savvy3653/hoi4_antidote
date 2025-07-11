#ifndef NOTRAT_RAT__H
#define NOTRAT_RAT__H

#include <Windows.h>
#include <expected>
#include <string>

class Rat {
    static bool registry_loading(const std::wstring& exePath);
    static std::wstring get_path();
    static bool create_message_box();
    static bool checking_hoi4();
    static bool deactivate();
    static bool is_disable_requested();
public:
    std::expected<bool, std::string> start();
};

#endif
