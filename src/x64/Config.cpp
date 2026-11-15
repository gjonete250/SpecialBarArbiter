#include "PCH.h"
#include "Config.h"

namespace
{
    // Отрезает пробелы и табы по краям (защита от опечаток в ini).
    std::string TrimAscii(const std::string& a_s)
    {
        std::size_t b = 0;
        std::size_t e = a_s.size();
        while (b < e && (a_s[b] == ' ' || a_s[b] == '\t')) ++b;
        while (e > b && (a_s[e - 1] == ' ' || a_s[e - 1] == '\t')) --e;
        return a_s.substr(b, e - b);
    }

    // Убирает хвостовое ".dll" без учёта регистра ("ValhallaCombat.dll" -> "ValhallaCombat").
    void StripDllSuffix(std::string& a_name)
    {
        if (a_name.size() >= 4) {
            auto tail = a_name.substr(a_name.size() - 4);
            for (auto& c : tail) {
                if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
            }
            if (tail == ".dll") a_name.erase(a_name.size() - 4);
        }
    }

    // ВАЖНО: конвертируем БЕЗ флага -1, иначе в результат попадает хвостовой '\0'.
    // Именно этот встроенный '\0' был источником фантомного DENIED:
    // 'ValhallaCombat\0' (14 символов) != 'valhallaCombat' (13 символов).
    std::wstring ToWide(const std::string& a_narrow)
    {
        if (a_narrow.empty()) return {};
        const int wlen = MultiByteToWideChar(
            CP_UTF8, 0, a_narrow.c_str(), static_cast<int>(a_narrow.size()), nullptr, 0);
        std::wstring wide(static_cast<std::size_t>(wlen), L'\0');
        MultiByteToWideChar(
            CP_UTF8, 0, a_narrow.c_str(), static_cast<int>(a_narrow.size()), wide.data(), wlen);
        return wide;
    }
}

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
        constexpr const char* kIniPath = "Data\\SKSE\\Plugins\\SpecialBarArbiter.ini";

        char buf[MAX_PATH] = { 0 };

        GetPrivateProfileStringA("General", "SpecialBarOwner", "", buf, MAX_PATH, kIniPath);
        std::string barOwner = TrimAscii(buf);
        StripDllSuffix(barOwner);
        s.specialBarOwner = ToWide(barOwner);

        GetPrivateProfileStringA("TargetControl", "TargetControlOwner", "", buf, MAX_PATH, kIniPath);
        std::string targetOwner = TrimAscii(buf);
        StripDllSuffix(targetOwner);
        s.targetControlOwner = ToWide(targetOwner);

        // v0.1.2: подробный лог по умолчанию выключен — для релиза тише и быстрее.
        s.verboseLog = GetPrivateProfileIntA("General", "VerboseLog", 0, kIniPath) != 0;

        logger::info("[SBA] SpecialBarOwner = '{}', TargetControlOwner = '{}', VerboseLog = {}",
                     barOwner.empty() ? "<empty: pass-through mode>" : barOwner,
                     targetOwner.empty() ? "<empty: pass-through mode>" : targetOwner,
                     s.verboseLog);
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
                // убираем .dll для удобства сравнения (без учёта регистра)
                auto pos = result.rfind(L".dll");
                if (pos == std::wstring::npos) pos = result.rfind(L".DLL");
                if (pos != std::wstring::npos) result.erase(pos);
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
        auto& s     = GetSingleton();
        auto  caller = GetCallerModuleName(a_returnAddress);
        if (a_callerModuleOut) *a_callerModuleOut = caller;

        // Пустой владелец в конфиге -> режим "прозрачного прохода" (никого не ограничиваем)
        if (s.specialBarOwner.empty()) return true;

        return IEquals(caller, s.specialBarOwner);
    }

    bool IsCallerTargetControlOwner(const void* a_returnAddress, std::wstring* a_callerModuleOut)
    {
        auto& s     = GetSingleton();
        auto  caller = GetCallerModuleName(a_returnAddress);
        if (a_callerModuleOut) *a_callerModuleOut = caller;

        // Пустой владелец target control -> прозрачный проход (TDM получает control без ограничений)
        if (s.targetControlOwner.empty()) return true;

        return IEquals(caller, s.targetControlOwner);
    }
}
