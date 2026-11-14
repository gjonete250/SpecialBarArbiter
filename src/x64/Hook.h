#pragma once

#include "PCH.h"

namespace Arbiter::Hook
{
    // Устанавливает перехват (Microsoft Detours) на экспорт RequestPluginAPI из TrueHUD.dll.
    // Возвращает true при успехе. Безопасно вызывать повторно.
    bool Install();
}
