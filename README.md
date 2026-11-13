# Special Bar Arbiter (SBA) v0.1 — «Миротворец»

Автор: **gjonete** — я люблю Skyrim с модами 🙂
Идея и код: Special Bar Arbiter — SKSE-плагин, который прекращает войну
комбо-модов за спец-полоски TrueHUD.

## Зачем он нужен

TrueHUD выдаёт управление «особой полоской» (стан/пауза над здоровьем)
только ОДНОМУ плагину — кто первый запросил, тот и хозяин. Из-за этого:
- ValhallaCombat, POISE, Chocolate Poise не могут сосуществовать
- второй мод вызывает краш `std::bad_function_call` в TrueHUD.dll

Arbiter перехватывает выдачу API и отдаёт полоски только владельцу из
конфига, остальным — вежливый отказ. Вызовы «мигнуть полоской» без
владельца безопасно игнорируются. Краш технически невозможен.

## Установка (готовый DLL)

1. Скачайте `SpecialBarArbiter.dll` (сборка из Actions или Release)
2. В MO2 создайте мод-папку `SpecialBarArbiter`, внутрь положите:
   - `SKSE/Plugins/SpecialBarArbiter.dll`
   - `SKSE/Plugins/SpecialBarArbiter.ini`
3. Запустите игру

Требования: SKSE, TrueHUD (естественно), игра 1.6.1170+ (NG-сборка —
совместима и с новее).

## Конфиг

`Data/SKSE/Plugins/SpecialBarArbiter.ini`:
```ini
[General]
; DLL-имя плагина-владельца спец-полосок (регистр не важен)
SpecialBarOwner=ValhallaCombat
; 1 = подробный лог всех обращений к API
VerboseLog=1
