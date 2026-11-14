#pragma once

// ---------- Win32 ----------
#include <Windows.h>

// ---------- spdlog ----------
#include <spdlog/sinks/basic_file_sink.h>

// ---------- Полный прелюд CommonLibSSE-NG ----------
// (std-литералы, пространство REL, stl-алиасы, базовые типы RE — всё в верном порядке)
#include "SKSE/Impl/PCH.h"

// ---------- То, что нам нужно сверх прелюда ----------
#include "RE/A/Actor.h"
#include "RE/A/ActorValue.h"
#include "RE/G/GFxMovieView.h"
#include "RE/G/GFxValue.h"
#include "RE/N/NiPoint3.h"
#include "RE/N/NiQuaternion.h"
#include "SKSE/SKSE.h"
#include "SKSE/Logger.h"

// ---------- intrinsics (_ReturnAddress) ----------
#include <intrin.h>

// ---------- TrueHUD API (скачивается при сборке) ----------
#include "API/TrueHUDAPI.h"
