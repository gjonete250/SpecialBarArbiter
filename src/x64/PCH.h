#pragma once

// ---------- Стандартная библиотека (CommonLib ожидает их в предвключении) ----------
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
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

// ---------- CommonLibSSE-NG ----------
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

// ---------- TrueHUD API (скачивается при сборке: cmake/VendorTrueHUDAPI.cmake) ----------
#include "API/TrueHUDAPI.h"
