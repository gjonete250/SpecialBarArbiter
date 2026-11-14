#include "PCH.h"
#include "Hook.h"
#include "Shim.h"

namespace Arbiter::Hook
{
    namespace
    {
        constexpr std::size_t HOOK_BYTES = 14;  // FF 25 00 00 00 00 + 8-байтовый абсолютный адрес

        enum class HookMode
        {
            NotInstalled,
            PointerSwap,   // экспорт оказался трамплином jmp [ptr]: меняем сам указатель
            CodePatching   // экспорт - обычная функция: классический 14-байтовый патч + пещера
        };

        HookMode       g_mode{ HookMode::NotInstalled };
        void*          g_originalFunc{ nullptr };  // оригинальная функция (для PointerSwap)
        std::byte*     g_cave{ nullptr };          // пещера рядом с TrueHUD.dll (для CodePatching)
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

        // Находит адрес указателя, через который трамплин jmp [rip+disp32] уходит к цели.
        // Возвращает nullptr, если это не трамплин.
        std::byte* FindThunkPointerSlot(std::uintptr_t a_target)
        {
            const auto b0 = *reinterpret_cast<const std::uint8_t*>(a_target);
            const auto b1 = *reinterpret_cast<const std::uint8_t*>(a_target + 1);
            if (b0 != 0xFF || b1 != 0x25) return nullptr;

            const auto disp = *reinterpret_cast<const std::int32_t*>(a_target + 2);
            return reinterpret_cast<std::byte*>(a_target + 6 + disp);
        }

        __declspec(noinline) void* Thunk_RequestPluginAPI(const TRUEHUD_API::InterfaceVersion a_version)
        {
            // Адрес возврата = точка вызова в DLL клиента
            const void* caller = _ReturnAddress();
            void*       real   = CallOriginal(a_version);
            return SpecialBarShim::Attach(real, caller, a_version);
        }
    }

    void* CallOriginal(TRUEHUD_API::InterfaceVersion a_version)
    {
        switch (g_mode) {
        case HookMode::PointerSwap:
            {
                auto fn = reinterpret_cast<void* (*)(TRUEHUD_API::InterfaceVersion)>(g_originalFunc);
                return fn(a_version);
            }
        case HookMode::CodePatching:
            {
                // выполняем сохранённые оригинальные байты в пещере,
                // затем jmp rel32 возвращает в конец перехваченного кода
                auto fn = reinterpret_cast<void* (*)(TRUEHUD_API::InterfaceVersion)>(reinterpret_cast<void*>(g_cave));
                return fn(a_version);
            }
        default:
            return nullptr;
        }
    }

    bool Install()
    {
        if (g_mode != HookMode::NotInstalled) return true;  // уже установлен

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

        // --- Вариант А: экспорт - трамплин jmp [ptr] -> подменяем сам указатель ---
        if (auto ptrSlot = FindThunkPointerSlot(target)) {
            DWORD oldProtect = 0;
            if (!VirtualProtect(ptrSlot, 8, PAGE_READWRITE, &oldProtect)) {
                logger::error("[SBA] VirtualProtect (pointer slot) failed");
                return false;
            }

            g_originalFunc = *reinterpret_cast<void**>(ptrSlot);
            *reinterpret_cast<void**>(ptrSlot) = reinterpret_cast<void*>(&Thunk_RequestPluginAPI);

            VirtualProtect(ptrSlot, 8, oldProtect, &oldProtect);

            g_mode   = HookMode::PointerSwap;
            g_target = target;
            logger::info("[SBA] Hook installed (pointer swap) on TrueHUD.dll!RequestPluginAPI @ 0x{:X}, original fn @ 0x{:X}",
                         target, reinterpret_cast<std::uintptr_t>(g_originalFunc));
            return true;
        }

        // --- Вариант Б: обычная функция -> классический 14-байтовый патч + пещера ---
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
        g_originalFunc = cave;  // в этом режиме "оригинал" - скопированный пролог в пещере

        // в пещере после копии: jmp rel32 на target+HOOK_BYTES (возврат в оригинал)
        auto retAddr = target + HOOK_BYTES;
        auto jmpSite = reinterpret_cast<std::uintptr_t>(cave + HOOK_BYTES);
        auto rel     = static_cast<std::int32_t>(retAddr - (jmpSite + 5));
        std::byte patch[5] = { static_cast<std::byte>(0xE9), static_cast<std::byte>(0), static_cast<std::byte>(0), static_cast<std::byte>(0), static_cast<std::byte>(0) };
        std::memcpy(patch + 1, &rel, sizeof(rel));
        std::memcpy(cave + HOOK_BYTES, patch, 5);

        // патчим сам экспорт: FF 25 00 00 00 00 [8-байтовый адрес нашего Thunk]
        std::byte hook[HOOK_BYTES] = {};  // все байты = 0
        hook[0] = static_cast<std::byte>(0xFF);
        hook[1] = static_cast<std::byte>(0x25);
        // hook[2..5] = 0 (rip+0)
        auto thunkAddr = reinterpret_cast<std::uint64_t>(&Thunk_RequestPluginAPI);
        std::memcpy(hook + 6, &thunkAddr, sizeof(thunkAddr));
        std::memcpy(reinterpret_cast<void*>(target), hook, HOOK_BYTES);

        VirtualProtect(reinterpret_cast<LPVOID>(target), HOOK_BYTES, oldProtect, &oldProtect);

        g_mode   = HookMode::CodePatching;
        g_target = target;
        logger::info("[SBA] Hook installed (code patch) on TrueHUD.dll!RequestPluginAPI @ 0x{:X}", target);
        return true;
    }
}
