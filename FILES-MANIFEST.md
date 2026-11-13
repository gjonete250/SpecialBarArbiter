# Куда класть файлы в репозитории (созданном из шаблона powerof3/SSEPluginTemplate)

Репозиторий: https://github.com/powerof3/SSEPluginTemplate → «Use this template»

| Файл из этой папки          | Куда положить в репозитории            |
|-----------------------------|----------------------------------------|
| src/x64/PCH.h               | src/x64/PCH.h (заменить шаблонный)     |
| src/x64/main.cpp            | src/x64/main.cpp (заменить шаблонный)  |
| src/x64/Config.h            | src/x64/Config.h (новый)               |
| src/x64/Config.cpp          | src/x64/Config.cpp (новый)             |
| src/x64/Shim.h              | src/x64/Shim.h (новый)                 |
| src/x64/Shim.cpp            | src/x64/Shim.cpp (новый)               |
| src/x64/Hook.h              | src/x64/Hook.h (новый)                 |
| src/x64/Hook.cpp            | src/x64/Hook.cpp (новый)               |
| cmake/VendorTrueHUDAPI.cmake| cmake/VendorTrueHUDAPI.cmake (новый)   |
| Data/SKSE/Plugins/SpecialBarArbiter.ini | не загружать в репо (положите в мод позже) |

## Одна правка в CMakeLists.txt

Найдите строку `project(...)` (в начале файла) и сразу после неё добавьте:

```cmake
include(cmake/VendorTrueHUDAPI.cmake)
```

## Дальше

1. GitHub: вкладка Actions → выберите workflow → Run workflow → дождитесь зелёной галки (~15 мин)
2. В завершившемся запуске — раздел Artifacts → скачайте архив
3. Внутри: `SkyrimSE-Debug/SpecialBarArbiter.dll` (или Release-вариант — берите Release)
4. Установка: в MO2 создайте новый мод (например, папка `SpecialBarArbiter`), внутрь:
   - `SKSE/Plugins/SpecialBarArbiter.dll`
   - `SKSE/Plugins/SpecialBarArbiter.ini`
5. Порядок загрузки DLL неважен — хук ставится в kPostLoad

## Лог работы

`Documents\My Games\Skyrim Special Edition\SKSE\SpecialBarArbiter.log` —
смотрите, кто запрашивал API, кому отказано, какие вызовы «проглочены» (crash prevented).
