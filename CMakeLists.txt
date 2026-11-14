cmake_minimum_required(VERSION 3.22)
project(SpecialBarArbiter VERSION 0.1.0 LANGUAGES CXX)

# Скачиваем аутентичный хедер TrueHUD API до поиска библиотеки
include(cmake/VendorTrueHUDAPI.cmake)

find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(
    SpecialBarArbiter
    SOURCES
        src/x64/main.cpp
        src/x64/Config.cpp
        src/x64/Shim.cpp
        src/x64/Hook.cpp
)

target_compile_features(SpecialBarArbiter PRIVATE cxx_std_23)
target_compile_options(SpecialBarArbiter PRIVATE
    /utf-8
    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/x64/PCH.h"
)
target_include_directories(SpecialBarArbiter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/x64
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
