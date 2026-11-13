# Special Bar Arbiter v0.1 — сборка за 6 шагов (без Visual Studio!)

SKSE-плагин «миротворец» для TrueHUD: решает конфликт спец-полосок
(ValhallaCombat vs POISE vs другие) и убирает краш `std::bad_function_call`.

Игра: Skyrim SE/AE 1.6.1170+ (CommonLibSSE-NG — одна DLL на все версии).

## Шаг 1. Пустой репозиторий на GitHub
New repository → имя `SpecialBarArbiter` → Public → Create.

## Шаг 2. Загрузить файлы проекта
В репозитории: Add file → Upload files → загрузите всё содержимое этой папки
(папки создаются сами, если в имени файла указать путь, например
`src/x64/main.cpp`).

Нужные файлы:
```
.github/workflows/build.yml
CMakeLists.txt
CMakePresets.json
vcpkg.json
vcpkg-configuration.json
cmake/VendorTrueHUDAPI.cmake
src/x64/PCH.h
src/x64/main.cpp
src/x64/Config.h
src/x64/Config.cpp
src/x64/Hook.h
src/x64/Hook.cpp
src/x64/Shim.h
src/x64/Shim.cpp
```
Имя файла при загрузке пишете как `src/x64/main.cpp` — GitHub сам создаст папки.

## Шаг 3. Собрать
Вкладка **Actions** (сверху) → появится workflow «Build SpecialBarArbiter» →
кликните → справа **Run workflow** → зелёная кнопка. Ждите ~20–30 минут
(первая сборка качает библиотеки).

## Шаг 4. Скачать DLL
В завершившемся запуске (зелёная галка) внизу будет **Artifacts: SpecialBarArbiter**
— скачайте, распакуйте.

## Шаг 5. Установить в MO2
Создайте в MO2 новый мод-папку (например, `SpecialBarArbiter`), внутрь:
```
SKSE\Plugins\SpecialBarArbiter.dll
SKSE\Plugins\SpecialBarArbiter.ini
```
Файл ini лежит в этом проекте: `Data/SKSE/Plugins/SpecialBarArbiter.ini`.

## Шаг 6. Проверить
Запустите игру. Лог:
`Documents\My Games\Skyrim Special Edition\SKSE\SpecialBarArbiter.log`

В нём увидите:
- `Hook installed on TrueHUD.dll!RequestPluginAPI` — перехват работает
- строки вида `RequestSpecialResourceBarsControl DENIED to 'XXX'` — видно,
  какой мод хотел полоски и получил отказ (владелец = ValhallaCombat)

## Конфиг

`Data/SKSE/Plugins/SpecialBarArbiter.ini`:
```ini
[General]
SpecialBarOwner=ValhallaCombat
VerboseLog=1
```
`SpecialBarOwner` — имя DLL без пути (регистр не важен). Пусто = прозрачный
режим (никого не ограничивать).

## Если сборка упала
Пришлите мне красный текст ошибки из лога Actions — правлю, обычно за 1–2
итерации.
