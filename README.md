# ArenaShooter

A multiplayer first-person arena shooter built with **Unreal Engine 5** and C++, designed around the Gameplay Ability System (GAS). Two players duel in fast-paced deathmatch rounds with weapon pickups, shields, and Steam matchmaking.

## Features

### Gameplay
- **Duel game mode** — timed matches with warmup, kill tracking, respawn timers, and a win/lose/draw result screen
- **Weapon inventory & hotbar** — rifle and pistol with instant, client-predicted weapon switching
- **Per-weapon ammo** — depleting ammo pools that persist per weapon across switches
- **Pickups** — health, ammo, shield
- **Shield mechanic** — shields absorb a portion of incoming damage before health

### Technical highlights
- **Gameplay Ability System** throughout: firing, death, damage, and healing are abilities and gameplay effects; weapons grant their abilities via data-driven ability sets on equip
- **Client-predicted weapon switching** — slot-index + sequence-number replication design; switches feel instant on the owning client and reconcile authoritatively on the server
- **Ammo as a GAS attribute** — live ammo lives on the player's ability system component with predicted fire costs, hot-swapped against per-weapon persistent storage on switch
- **Dual-mesh character** — separate first-person arms and third-person body, each with its own animation blueprint and montage replication
- **Linked anim layers** — per-weapon animation layers swapped at runtime for both 1P and 3P meshes; hand IK keeps hands glued to weapon grips across aim offsets, gated by anim curves during equips
- **MVVM UI** — HUD built on UMG ViewModel: health, shields, ammo, hotbar, match timer, and player info are all event-driven with no widget polling
- **Sessions plugin** — custom OnlineSubsystem wrapper supporting Steam and LAN: host, browse, join, and clean session teardown on both voluntary quit and network failure

## Built with

- Unreal Engine 5.4
- C++ (gameplay framework, GAS, networking, UI managers) with Blueprints for content and widgets
- Plugins: GameplayAbilities, EnhancedInput, UMG ViewModel, OnlineSubsystemSteam

## Getting started

### Build & run
1. Clone the repository
2. Right-click `ArenaShooter.uproject` → **Generate Visual Studio project files**
3. Open `ArenaShooter.sln` and build the `ArenaShooter` target (Development Editor)
4. Launch `ArenaShooter.uproject`

### Testing multiplayer

**LAN (single machine):** set `DefaultPlatformService=Null` in `Config/DefaultEngine.ini`, then launch two standalone instances (e.g. run the packaged build twice, or `UnrealEditor.exe ArenaShooter.uproject -game -windowed` twice). Host from one instance, refresh and join from the other.

**Steam (two machines):** set `DefaultPlatformService=Steam`, package the project, and run it on two PCs with different Steam accounts logged in. Uses the Spacewar test AppId (480).

## Status

In active development. Planned: more weapons, additional pickup types, proper maps, ui and polish on animation and game feel.
