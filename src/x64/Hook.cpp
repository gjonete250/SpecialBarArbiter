#include "PCH.h"
#include "Hook.h"
#include "Shim.h"

namespace Arbiter::Hook
{
    namespace
    {
        constexpr std::size_t HOOK_BYTES = 14;  // FF 25 00 00 00 00 + 8-байтовый абсолютный адрес

        std::byte* g_originalBytes{ nullptr };   // копия оригинальных байтов (для вызова оригинала)
        std::byte* g_cave{ nullptr };            // пещера рядом с TrueHUD.dll
        std::uintptr_t g_target{ 0 };

        // Выделяет EXECUTE-страницу в пределах +/-2 ГБ от target (RIP-относительный переход)
        std::byte* AllocateCaveNear(std::uintptr_t a_target)
        {
            constexpr std::uintptr_t RANGE = 0x70000000;  // ~1.75 ГБ запас
            std::uintptr_t start = (a_target > RANGE) ? (a_target - RANGE) : 0x10000;
            start &= ~std::uintptr_t{ 0xFFFF };           // выравнивание на 64K

            for (std::uintptr_t addr = start; addr < a_target + RANGE; addr += 0x10000) {
                if (auto p = VirtualAlloc(reinterpret_cast<LPVOID>(addr), 0x1000,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
                    return static_cast<std::byte*>(p);
            }
            return nullptr;
        }

        void* __declspec(noinline) Thunk_RequestPluginAPI(const TRUEHUD_API::InterfaceVersion a_version)
        {
            // Адрес возврата = точка вызова в DLL клиента (мы вошли сюда через jmp)
            const void* caller = _ReturnAddress();
            void*       real   = CallOriginal(a_version);
            return SpecialBarShim::Attach(real, caller, a_version);
        }
    }

    void* CallOriginal(TRUEHUD_API::InterfaceVersion a_version)
    {
        // 1) выполняем сохранённые оригинальные байты в пещере...
        void* result = reinterpret_cast<void* (*)(const InterfaceVersion)>(reinterpret_cast<void*>(g_cave))(a_version);
        // 2) ...и прыгаем обратно в конец перехваченного кода (jmp rel32 записан в пещере)
        return result;
    }

    bool Install()
    {
        if (g_target) return true;  // уже установлен

        auto hTrueHUD = ::GetModuleHandleA("TrueHUD.dll");
        if (!hTrueHUD) {
            logger::warn("[SBA] TrueHUD.dll not loaded yet");
            return false;
        }

        auto target = reinterpret_cast<std::uintptr_t>(::GetProcAddress(hTrueHUD, "RequestPluginAPI"));
        if (!target) {
            logger::error("[SBA] Export 'RequestPluginAPI' not found in TrueHUD.dll");
            return false;
        }

        auto cave = AllocateCaveNear(target);
        if (!cave) {
            logger::error("[SBA] Failed to allocate code cave near TrueHUD.dll");
            return false;
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(reinterpret_cast<LPVOID>(target), HOOK_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            logger::error("[SBA] VirtualProtect failed");
            return false;
        }

        // копируем оригинальные байты в пещеру
        std::memcpy(cave, reinterpret_cast<void*>(target), HOOK_BYTES);
        g_originalBytes = cave;

        // в пещере после копии: jmp rel32 на target+HOOK_BYTES (возврат в оригинал)
        auto retAddr = target + HOOK_BYTES;
        auto jmpSite = reinterpret_cast<std::uintptr_t>(cave + HOOK_BYTES);
        auto rel     = static_cast<std::int32_t>(retAddr - (jmpSite + 5));
        std::byte patch[5] = { std::byte{ 0xE9 }, std::byte(0), std::byte(0), std::byte(0), std::byte(0) };
        std::memcpy(patch + 1, &rel, sizeof(rel));
        std::memcpy(cave + HOOK_BYTES, patch, 5);

        // патчим сам экспорт: FF 25 00 00 00 00 [8-байтовый адрес нашего Thunk]
        std::byte hook[HOOK_BYTES] = { 0 };
        hook[0] = std::byte{ 0xFF };
        hook[1] = std::byte{ 0x25 };
        // hook[2..5] = 0 (rip+0)
        auto thunkAddr = reinterpret_cast<std::uint64_t>(&Thunk_RequestPluginAPI);
        std::memcpy(hook + 6, &thunkAddr, sizeof(thunkAddr));
        std::memcpy(reinterpret_cast<void*>(target), hook, HOOK_BYTES);

        VirtualProtect(reinterpret_cast<LPVOID>(target), HOOK_BYTES, oldProtect, &oldProtect);
        // пещеру оставляем RWX (жертвуем минимальной безопасностью ради простоты MVP)
        g_target = target;

        logger::info("[SBA] Hook installed on TrueHUD.dll!RequestPluginAPI @ 0x{:X}", target);
        return true;
    }
}
