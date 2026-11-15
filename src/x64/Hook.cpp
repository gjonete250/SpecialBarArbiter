#include "PCH.h"
#include "Hook.h"
#include "Shim.h"

#include <detours/detours.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace Arbiter::Hook
{
    namespace
    {
        using RequestPluginAPI_t = void* (*)(TRUEHUD_API::InterfaceVersion);

        RequestPluginAPI_t g_realRequestPluginAPI = nullptr;
        std::mutex         g_installMutex;
        std::atomic<bool>  g_hookDone{ false };

        // Перехватчик: сюда попадают ВСЕ клиенты, вызывающие TrueHUD!RequestPluginAPI.
        __declspec(noinline) void* Hooked_RequestPluginAPI(const TRUEHUD_API::InterfaceVersion a_version)
        {
            const void* caller = _ReturnAddress();
            void*       real   = g_realRequestPluginAPI ? g_realRequestPluginAPI(a_version) : nullptr;
            return SpecialBarShim::Attach(real, caller, a_version);
        }

        // Пытается установить перехват. ВЫЗЫВАТЬ ПОД g_installMutex.
        // true  = установлен (или уже был / ретраи бессмысленны);
        // false = TrueHUD.dll ещё не в памяти — можно повторить попытку позже.
        bool TryInstallLocked()
        {
            if (g_hookDone.load(std::memory_order_acquire)) return true;

            auto hTrueHUD = ::GetModuleHandleA("TrueHUD.dll");
            if (!hTrueHUD) return false;

            auto target = ::GetProcAddress(hTrueHUD, "RequestPluginAPI");
            if (!target) {
                // Модуль загружен, но экспорта нет: ретраи ничего не изменят.
                logger::error("[SBA] Export 'RequestPluginAPI' not found in TrueHUD.dll");
                g_hookDone.store(true, std::memory_order_release);
                return true;
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
                DetourTransactionAbort();   // v0.1.2: транзакция обязана быть закрыта при ошибке
                g_realRequestPluginAPI = nullptr;
                return false;
            }

            err = DetourTransactionCommit();
            if (err != NO_ERROR) {
                logger::error("[SBA] DetourTransactionCommit failed: {}", err);
                DetourTransactionAbort();
                g_realRequestPluginAPI = nullptr;
                return false;
            }

            g_hookDone.store(true, std::memory_order_release);
            logger::info("[SBA] Hook installed (Microsoft Detours) on TrueHUD.dll!RequestPluginAPI @ 0x{:X}",
                         reinterpret_cast<std::uintptr_t>(target));
            return true;
        }

        // Сторожевой поток («Bouncer»): ловит момент появления TrueHUD.dll в памяти
        // и ставит перехват ДО первой раздачи kPostLoad. Раньше моды с номером меньше
        // нашего в порядке загрузки успевали взять настоящий API в обход шима —
        // теперь перехват встаёт первым, кто бы ни запросил API.
        void WatcherLoop()
        {
            // ~20 секунд жизни: если TrueHUD так и не появился — отступаем,
            // остаётся штатный ретрай на kPostLoad/kPostPostLoad.
            for (int attempt = 0; attempt < 20000; ++attempt) {
                {
                    std::lock_guard lock(g_installMutex);
                    if (TryInstallLocked()) return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            logger::warn("[SBA] watcher: TrueHUD.dll not found within 20s, stopping (fallback retries remain active)");
        }
    }

    void StartWatcher()
    {
        std::thread(WatcherLoop).detach();
    }

    bool Install()
    {
        std::lock_guard lock(g_installMutex);
        return TryInstallLocked();
    }
}
