# GoldenEye Bond Character Setup Guide — First-Person Arms Only

This guide builds Bond as a first-person-only character.

There is no full-body player mesh in the design.

Because `AJamesBondCharacter` inherits from `ACharacter`, Unreal still provides the inherited component:

```text
Mesh (CharacterMesh0)
```

Leave that component empty, hidden, and without collision.

## Final architecture

```text
AJamesBondCharacter
CapsuleComponent
├── FirstPersonCamera
│   └── FirstPersonArms
│       └── WeaponRoot
├── BondAudioComponent
├── BondHealthComponent
├── BondWeaponComponent
├── BondTimeSlowComponent
├── BondInteractionComponent
└── CharacterMovement
```

The equipped gun is not a permanent component inside Bond.

At runtime:

```text
BondWeaponComponent
→ Spawns BP_Pistol
→ Sets Bond as owner
→ Attaches BP_Pistol to WeaponRoot
```

Weapon structure:

```text
ABondWeaponBase
└── BP_Pistol
    ├── Root
    ├── WeaponMesh
    ├── MuzzlePoint
```

## Responsibilities

```text
AJamesBondCharacter
- Movement
- Camera
- Enhanced Input
- Input forwarding
- Component ownership

FirstPersonArms
- Owner-only first-person presentation
- Fire and reload animations
- No collision

BondHealthComponent
- Health
- Damage
- Healing
- Death

BondWeaponComponent
- Spawn weapon
- Equip weapon
- Forward fire and reload commands

BondWeaponBase / BP_Pistol
- Ammo
- Fire rate
- Reload
- Weapon mesh
- Projectile (Using Raycast)
- Muzzle flash
- Weapon audio
- Recoil

BondTimeSlowComponent
- Time-slow meter
- Drain
- Regeneration
- Time dilation

BondInteractionComponent
- Interaction trace
- Current interactable
- Interaction command

BondPlayerController
- Create HUD widgets
- Bind gameplay events to UI
```

## Project rules

1. Compile after every small milestone.
2. Keep Unreal Editor closed during full Visual Studio builds.
3. Disable Live Coding while adding or changing reflected C++ classes.
4. Do not implement all systems in one Codex task.
5. Do not duplicate the previous Bond Blueprint.
6. Do not add a permanent gun component to Bond.
7. Do not place weapon audio or ammo variables on Bond.
8. Do not update UI using Tick.
9. Stop when a stage fails and fix it before continuing.

## Implementation order

1. Create the minimal Bond character.
2. Configure input and `BP_JamesBond`.
3. Create the health component.
4. Create `ABondWeaponBase`.
5. Create `UBondWeaponComponent`.
6. Create and configure `BP_Pistol`.
7. Add firing.
8. Add reload.
9. Create the time-slow component.
10. Create the Player Controller and HUD.
11. Add interaction last.
