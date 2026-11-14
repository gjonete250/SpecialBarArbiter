# Changelog

## v0.1.1 — «Peacemaker, hotfix edition» (2026-11-14)

### Fixed
- **Фантомный хвостовой `'\0'` в `SpecialBarOwner`** — корневая причина бага «владелец получает DENIED у собственного замка»: `MultiByteToWideChar` вызывался с флагом `-1`, и завершающий нуль попадал в строку (`'ValhallaCombat\0'`, 14 символов ≠ `'valhallaCombat'`, 13 символов). Теперь конвертация идёт по точной длине строки —DENIED из ниоткуда больше не возникает.
- **`[TargetControl]` — отдельный владелец target control.** В v0.1.0 `RequestTargetControl`/`SetTarget`/`SetSoftTarget` проверяли владельца *спец-полосок*, из-за чего TDM терял управление целью. Теперь у target control свой владелец (`[TargetControl] TargetControlOwner` в ini), пустое значение = прозрачный проход (TDM работает из коробки).
- **Trim пробелов/табов** для всех строковых значений ini — хвостовой пробел в `SpecialBarOwner=ValhallaCombat␣` больше не ломает сравнение.
- Обработка суффикса `.dll` в имени владельца (без учёта регистра) сохранена и вынесена в общий хелпер.

### Changed
- `VERSION 0.1.0` → `0.1.1` (CMakeLists + строка в логе запуска).

### Примечание
Логи «FlashActorValue swallowed … crash prevented» — это нормальная работа шима: чужие вызовы отрисовки глохнут, пока спец-полоски никем не заняты. В v0.1.1 при правильно настроенном владельце такие строки должны почти исчезнуть (полоски займутся — «forwarded»).

## v0.1.0 — «Peacemaker» (2026-11-13)

### Added
- Перехват `TrueHUD.dll!RequestPluginAPI` через Microsoft Detours (установка на kPostLoad с ретраем на kPostPostLoad).
- Шим `IVTrueHUD4` (совместим с V1–V3): выдача API только настроенному владельцу (`[General] SpecialBarOwner`), чужие вызовы отрисовки/цветов спец-полосок безопасно глохнут — устраняет краш `std::bad_function_call` в TrueHUD.dll.
- Конфиг `Data\SKSE\Plugins\SpecialBarArbiter.ini`: `SpecialBarOwner`, `VerboseLog`.
- Лог `Documents\My Games\Skyrim Special Edition\SKSE\SpecialBarArbiter.log`.
