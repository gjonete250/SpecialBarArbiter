#pragma once

#include "PCH.h"

namespace Arbiter::Hook
{
    // Запускает сторожевой поток, который ставит перехват, как только TrueHUD.dll
    // появится в памяти (раньше первой раздачи kPostLoad). Вызывается один раз
    // из SKSEPlugin_Load. Не блокирует поток вызова.
    void StartWatcher();

    // Синхронная попытка установки перехвата (используется как fallback
    // на kPostLoad/kPostPostLoad). Потокобезопасно; повторный вызов безопасен.
    // Возвращает true при успехе.
    bool Install();
}
