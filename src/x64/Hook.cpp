#include "PCH.h"
#include "Hook.h"
#include "Shim.h"

#include <detours.h>

namespace Arbiter::Hook
{
    namespace
    {
        using RequestPluginAPI_t = void* (*)(TRUEHUD_API::InterfaceVersion);

        RequestPluginAPI_t g_realRequestPluginAPI = nullptr;

        // Перехватчик: сюда попадают ВСЕ клиенты, вызывающие TrueHUD!RequestPluginAPI.
        __declspec(noinline) void* Hooked_RequestPluginAPI(const TRUEHUD_API::InterfaceVersion a_version)
        {
            const void* caller = _ReturnAddress();
            void*       real   = g_realRequestPluginAPI ? g_realRequestPluginAPI(a_version) : nullptr;
            return SpecialBarShim::Attach(real, caller, a_version);
        }
    }

    bool Install()
    {
        if (g_realRequestPluginAPI) return true;  // уже установлен

        auto hTrueHUD = ::GetModuleHandleA("TrueHUD.dll");
        if (!hTrueHUD) {
            logger::warn("[SBA] TrueHUD.dll not loaded yet");
            return false;
        }

        auto target = ::GetProcAddress(hTrueHUD, "RequestPluginAPI");
        if (!target) {
            logger::error("[SBA] Export 'RequestPluginAPI' not found in TrueHUD.dll");
            return false;
        }

        g_realRequestPluginAPI = reinterpret_cast<RequestPluginAPI_t>(target);

        // Detours корректно разбирает пролог функции (включая RIP-относительные
        // инструкции) и строит трамплин для вызова оригинала.
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());

        auto err = DetourAttach(
            reinterpret_cast<PVOID*>(&g_realRequestPluginAPI),
            reinterpret_cast<PVOID>(&Hooked_RequestPluginAPI));
        if (err != NO_ERROR) {
            logger::error("[SBA] DetourAttach failed: {}", err);
            g_realRequestPluginAPI = nullptr;
            return false;
        }

        err = DetourTransactionCommit();
        if (err != NO_ERROR) {
            logger::error("[SBA] DetourTransactionCommit failed: {}", err);
            g_realRequestPluginAPI = nullptr;
            return false;
        }

        logger::info("[SBA] Hook installed (Microsoft Detours) on TrueHUD.dll!RequestPluginAPI @ 0x{:X}",
                     reinterpret_cast<std::uintptr_t>(target));
        return true;
    }
}
