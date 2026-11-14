#pragma once

// ---------- Win32 ----------
#include <Windows.h>

// ---------- spdlog ----------
#include <spdlog/sinks/basic_file_sink.h>

// ---------- Полный прелюд CommonLibSSE-NG ----------
// (std-литералы, пространство REL/stl, базовые типы RE — в верном порядке)
#include "SKSE/Impl/PCH.h"

// ---------- Всё, что нам нужно сверх прелюда ----------
#include "RE/A/Actor.h"          // Actor, ActorValue, ActorHandle
#include "RE/G/GFxMovieView.h"   // GPtr, GFxMovieView (нужно TrueHUD API)

#include "SKSE/SKSE.h"
#include "SKSE/Logger.h"

// ---------- intrinsics (_ReturnAddress) ----------
#include <intrin.h>

// ---------- TrueHUD API (скачивается при сборке) ----------
#include "API/TrueHUDAPI.h"
