# Special Bar Arbiter (SBA) v0.1 — "The Peacemaker"

<p align="center">
  <img src="banner_en.svg" alt="Special Bar Arbiter" width="100%"/>
</p>


Author: **gjonete** — I love Skyrim with mods 🙂
An SKSE plugin that ends the war between combat mods fighting over
TrueHUD's special bars.

> 🇷🇺 Русская версия: [README.md](README.md)

## Why it exists

TrueHUD grants control over the "special bar" (the stun/poise bar above
health) to exactly ONE plugin — first come, first served. As a result:
- ValhallaCombat, POISE, and Chocolate Poise cannot coexist;
- whichever mod loses the race triggers a `std::bad_function_call`
  crash inside TrueHUD.dll.

Arbiter intercepts API hand-out and grants the bars only to the owner
defined in the config; everyone else receives a polite refusal.
Unowned "flash the bar" calls are safely ignored. The crash becomes
technically impossible.

## Installation (ready-made DLL)

1. Download `SpecialBarArbiter.dll` (from Actions or a Release).
2. In MO2, create a mod folder named `SpecialBarArbiter` and put inside:
   - `SKSE/Plugins/SpecialBarArbiter.dll`
   - `SKSE/Plugins/SpecialBarArbiter.ini`
3. Launch the game.

Requirements: SKSE, TrueHUD, game build 1.6.1170+ (the NG build is
forward-compatible).

## Configuration

`Data/SKSE/Plugins/SpecialBarArbiter.ini`:

```ini
[General]
; DLL name of the plugin allowed to own the special bars (case-insensitive)
SpecialBarOwner=ValhallaCombat
; 1 = verbose log of every API request
VerboseLog=1
```

An empty `SpecialBarOwner` = transparent mode (no restrictions).

## Verifying it works

Log file: `Documents\My Games\Skyrim Special Edition\SKSE\SpecialBarArbiter.log`

- `Hook installed on TrueHUD.dll!RequestPluginAPI` — the hook is active;
- `RequestSpecialResourceBarsControl DENIED to '...'` — which mod asked
  for the bars and was refused;
- `FlashActorValue swallowed ... crash prevented` — a prevented crash.

## Building from source

The repository builds itself: GitHub Actions → Build SpecialBarArbiter.
Artifact: `SpecialBarArbiter.dll`. Details are in `build.yml`.

## License

MIT — see [LICENSE](LICENSE).
TrueHUD API © powerof3.
