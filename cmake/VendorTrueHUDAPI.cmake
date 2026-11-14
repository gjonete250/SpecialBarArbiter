# VendorTrueHUDAPI.cmake
# Скачивает аутентичный заголовок API TrueHUD (powerof3) на этапе конфигурации,
# перебирая несколько источников. Это гарантирует, что раскладка виртуальных
# функций в нашем шиме совпадает с настоящей TrueHUD.dll.

set(SBA_TRUEHUD_API_PATH "${CMAKE_SOURCE_DIR}/include/API/TrueHUDAPI.h")

if(NOT EXISTS "${SBA_TRUEHUD_API_PATH}")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/API")

    set(SBA_CANDIDATE_URLS
        "https://raw.githubusercontent.com/powerof3/TrueHUD/dev/include/API/TrueHUDAPI.h"
        "https://raw.githubusercontent.com/powerof3/TrueHUD/main/include/API/TrueHUDAPI.h"
        "https://raw.githubusercontent.com/powerof3/TrueHUD/master/include/API/TrueHUDAPI.h"
        "https://raw.githubusercontent.com/powerof3/FloatingSubtitles/main/src/API/TrueHUDAPI.h"
        "https://raw.githubusercontent.com/powerof3/FloatingSubtitles/master/src/API/TrueHUDAPI.h"
    )

    set(SBA_FETCHED FALSE)
    foreach(SBA_URL IN LISTS SBA_CANDIDATE_URLS)
        message(STATUS "SBA: trying ${SBA_URL}")
        file(DOWNLOAD "${SBA_URL}" "${SBA_TRUEHUD_API_PATH}" STATUS SBA_STATUS TIMEOUT 30)
        list(GET SBA_STATUS 0 SBA_CODE)
        if(SBA_CODE EQUAL 0)
            message(STATUS "SBA: fetched TrueHUDAPI.h successfully")
            set(SBA_FETCHED TRUE)
            break()
        else()
            file(REMOVE "${SBA_TRUEHUD_API_PATH}")
            message(WARNING "SBA: download failed (${SBA_STATUS}), trying next URL")
        endif()
    endforeach()

    if(NOT SBA_FETCHED)
        message(FATAL_ERROR "SBA: could not fetch TrueHUDAPI.h from any known URL")
    endif()
endif()
