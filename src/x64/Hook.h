#pragma once

#include "PCH.h"

namespace Arbiter::Hook
{
    // Устанавливает 14-байтовый перехват на экспорт RequestPluginAPI из TrueHUD.dll.
    // Возвращает true при успехе. Безопасно вызывать повторно.
    bool Install();

    // Вызывается из Thunk: прозрачно выполняет оригинальный код экспорта.
    void* CallOriginal(TRUEHUD_API::InterfaceVersion a_version);
}
