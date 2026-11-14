#pragma once

// ---------- Стандартная библиотека (CommonLib ожидает их в предвключении) ----------
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ---------- Win32 (ДО CommonLib: IMAGE_SCN_*, HANDLE и прочее) ----------
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

// Совместимость: в NG логгер называется SKSE::log
namespace logger = SKSE::log;
