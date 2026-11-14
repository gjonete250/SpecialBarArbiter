#pragma once

#include "PCH.h"

namespace Arbiter::Config
{
    struct Settings
    {
        std::wstring specialBarOwner;      // имя DLL-плагина-владельца спец-полосок, напр. L"ValhallaCombat"
        std::wstring targetControlOwner;   // имя DLL-плагина-владельца target control, напр. L"TrueDirectionalMovement" (пусто = прозрачный проход)
        bool         verboseLog{ true };
    };

    Settings& GetSingleton();
    void      Load();

    // Проверяет, принадлежит ли адрес вызова DLL-модулю владельца спец-полосок.
    // a_returnAddress — адрес возврата из client-кадра (_ReturnAddress()).
    bool IsCallerOwner(const void* a_returnAddress, std::wstring* a_callerModuleOut = nullptr);

    // То же, но для владельца target control (независимая секция [TargetControl] в ini).
    bool IsCallerTargetControlOwner(const void* a_returnAddress, std::wstring* a_callerModuleOut = nullptr);

    // Имя модуля (без пути), в котором находится адрес вызова. Пустая строка при неудаче.
    std::wstring GetCallerModuleName(const void* a_returnAddress);
}
