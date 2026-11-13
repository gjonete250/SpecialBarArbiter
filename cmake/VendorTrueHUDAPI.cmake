# VendorTrueHUDAPI.cmake
# Скачивает аутентичный заголовок API TrueHUD (powerof3) на этапе конфигурации.
# Это гарантирует, что раскладка виртуальных функций в нашем шиме
# побайтово совпадает с настоящей TrueHUD.dll.

set(SBA_TRUEHUD_API_URL
    "https://raw.githubusercontent.com/powerof3/TrueHUD/main/include/API/TrueHUDAPI.h")
set(SBA_TRUEHUD_API_PATH "${CMAKE_SOURCE_DIR}/include/API/TrueHUDAPI.h")

if(NOT EXISTS "${SBA_TRUEHUD_API_PATH}")
    message(STATUS "SBA: fetching TrueHUD API header...")
    file(DOWNLOAD
        "${SBA_TRUEHUD_API_URL}"
        "${SBA_TRUEHUD_API_PATH}"
        STATUS SBA_DL_STATUS)
    list(GET SBA_DL_STATUS 0 SBA_DL_CODE)
    if(NOT SBA_DL_CODE EQUAL 0)
        message(FATAL_ERROR "SBA: failed to download TrueHUDAPI.h: ${SBA_DL_STATUS}")
    endif()
endif()

# подключаем include/ к таргетам шаблона
if(TARGET "${PROJECT_NAME}")
    target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/include")
endif()
