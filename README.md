# Special Bar Arbiter (SBA) v0.1 — «Миротворец»

Автор: **gjonete** — я люблю Skyrim с модами 🙂
SKSE-плагин, который прекращает войну комбо-модов за спец-полоски TrueHUD.

> 🇬🇧 English version: [README_EN.md](README_EN.md)

## Зачем он нужен

TrueHUD выдаёт управление «особой полоской» (стан/пауза над здоровьем)
только ОДНОМУ плагину — кто первый запросил, тот и хозяин. Из-за этого:
- ValhallaCombat, POISE, Chocolate Poise не могут сосуществовать;
- второй мод вызывает краш `std::bad_function_call` в TrueHUD.dll.

Arbiter перехватывает выдачу API и отдаёт полоски только владельцу из
конфига, остальным — вежливый отказ. Вызовы «мигнуть полоской» без
владельца безопасно игнорируются. Краш технически невозможен.

## Установка (готовый DLL)

1. Скачайте `SpecialBarArbiter.dll` (сборка из Actions или Release).
2. В MO2 создайте мод-папку `SpecialBarArbiter`, внутрь положите:
   - `SKSE/Plugins/SpecialBarArbiter.dll`
   - `SKSE/Plugins/SpecialBarArbiter.ini`
3. Запустите игру.

Требования: SKSE, TrueHUD, игра 1.6.1170+ (NG-сборка совместима и с новее).

## Конфиг

`Data/SKSE/Plugins/SpecialBarArbiter.ini`:

```ini
[General]
; DLL-имя плагина-владельца спец-полосок (регистр не важен)
SpecialBarOwner=ValhallaCombat
; 1 = подробный лог всех обращений к API
VerboseLog=1
```

Пустой `SpecialBarOwner` = прозрачный режим (никого не ограничивать).

## Проверка работы

Лог: `Documents\My Games\Skyrim Special Edition\SKSE\SpecialBarArbiter.log`

- `Hook installed on TrueHUD.dll!RequestPluginAPI` — перехват активен;
- `RequestSpecialResourceBarsControl DENIED to '...'` — кто хотел полоски
  и получил отказ;
- `FlashActorValue swallowed ... crash prevented` — предотвращённый краш.

## Сборка из исходников

Репозиторий собирает себя сам: GitHub Actions → Build SpecialBarArbiter.
Артефакт: `SpecialBarArbiter.dll`. Детали — в `build.yml`.

## Лицензия

MIT — см. файл [LICENSE](LICENSE).
TrueHUD API © powerof3.
