#include "PCH.h"
#include "Config.h"

namespace Arbiter::Config
{
    Settings& GetSingleton()
    {
        static Settings s;
        return s;
    }

    void Load()
    {
        auto& s = GetSingleton();
        char  owner[MAX_PATH] = { 0 };
        GetPrivateProfileStringA("General", "SpecialBarOwner", "", owner, MAX_PATH, "Data\\SKSE\\Plugins\\SpecialBarArbiter.ini");
        s.verboseLog = GetPrivateProfileIntA("General", "VerboseLog", 1, "Data\\SKSE\\Plugins\\SpecialBarArbiter.ini") != 0;

        // "ValhallaCombat" / "ValhallaCombat.dll" -> L"ValhallaCombat"
        std::string name(owner);
        if (auto pos = name.rfind(".dll"); pos != std::string::npos) name.erase(pos);
        s.specialBarOwner.clear();
        const int wlen = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
        if (wlen > 1) {
            std::wstring wname(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname.data(), wlen);
            s.specialBarOwner = wname;
        }
        logger::info("[SBA] SpecialBarOwner = '{}', VerboseLog = {}",
                     name.empty() ? "<empty: pass-through mode>" : name, s.verboseLog);
    }

    std::wstring GetCallerModuleName(const void* a_returnAddress)
    {
        std::wstring result;
        HMODULE hModule = nullptr;
        if (a_returnAddress &&
            GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(a_returnAddress), &hModule) && hModule) {
            wchar_t path[MAX_PATH] = { 0 };
            if (GetModuleFileNameW(hModule, path, MAX_PATH)) {
                result = path;
                if (auto pos = result.find_last_of(L'\\'); pos != std::wstring::npos) result.erase(0, pos + 1);
                // убираем .dll для удобства сравнения
                if (auto pos = result.rfind(L".dll"); pos != std::wstring::npos) result.erase(pos);
            }
        }
        return result;
    }

    static bool IEquals(const std::wstring& a, const std::wstring& b)
    {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i)
            if (std::towlower(a[i]) != std::towlower(b[i])) return false;
        return true;
    }

    bool IsCallerOwner(const void* a_returnAddress, std::wstring* a_callerModuleOut)
    {
        auto&  s     = GetSingleton();
        auto   caller = GetCallerModuleName(a_returnAddress);
        if (a_callerModuleOut) *a_callerModuleOut = caller;

        // Пустой владелец в конфиге -> режим "прозрачного прохода" (никого не ограничиваем)
        if (s.specialBarOwner.empty()) return true;

        return IEquals(caller, s.specialBarOwner);
    }
}
