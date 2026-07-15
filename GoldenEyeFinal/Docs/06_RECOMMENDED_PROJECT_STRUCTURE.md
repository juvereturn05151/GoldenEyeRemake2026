# Recommended project structure — First-Person Only

## C++ source

```text
Source/GoldenEyeFinal/
├── Characters/
│   ├── JamesBondCharacter.h
│   └── JamesBondCharacter.cpp
├── Components/
│   ├── BondHealthComponent.h
│   ├── BondHealthComponent.cpp
│   ├── BondWeaponComponent.h
│   ├── BondWeaponComponent.cpp
│   ├── BondTimeSlowComponent.h
│   ├── BondTimeSlowComponent.cpp
│   ├── BondInteractionComponent.h
│   └── BondInteractionComponent.cpp
├── Weapons/
│   ├── BondWeaponBase.h
│   └── BondWeaponBase.cpp
├── Projectiles/
│   ├── BondProjectileBase.h
│   └── BondProjectileBase.cpp
├── Player/
│   ├── BondPlayerController.h
│   └── BondPlayerController.cpp
└── UI/
    ├── BondHUD.h
    └── BondHUD.cpp
```

## Runtime structure

```text
AJamesBondCharacter
├── CapsuleComponent
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

The inherited `Mesh (CharacterMesh0)` remains empty and hidden.

The equipped weapon is spawned separately:

```text
BondWeaponComponent
→ BP_Pistol
```

## Content

```text
Content/
├── Blueprints/
│   ├── Characters/
│   │   └── Player/
│   │       └── BP_JamesBond
│   ├── Weapons/
│   │   └── USP/
│   │       ├── BP_Pistol
│   ├── GameModes/
│   │   └── BP_GoldenEyeGameMode
│   ├── Player/
│   │   └── BP_BondPlayerController
│   └── UI/
│       ├── WBP_BondHUD
│       └── WBP_TimeSlowHUD
├── Input/
│   └── JamesBond/
│       ├── IA_Move
│       ├── IA_Look
│       ├── IA_Jump
│       ├── IA_Fire
│       ├── IA_Reload
│       ├── IA_TimeSlow
│       └── IMC_JamesBond
├── Characters/
│   └── Bond/
│       └── FirstPerson/
│           ├── Arms/
│           ├── Animations/
│           └── Materials/
└── Weapons/
    └── USP/
        ├── Meshes/
        ├── Animations/
        ├── Effects/
        └── Audio/
```

There is no body-mesh content folder because the player uses first-person arms only.

## Ownership rules

```text
AJamesBondCharacter
- Movement
- Camera
- Input binding
- Input forwarding
- Component references

FirstPersonArms
- Owner-only visual presentation
- Fire and reload animations
- No collision

BondAudioComponent
- Pain
- Breathing
- Voice lines

BondHealthComponent
- Health and death

BondWeaponComponent
- Spawn and equip weapon
- Forward weapon commands

BP_Pistol / ABondWeaponBase
- Ammo
- Fire rate
- Reload
- Weapon mesh
- Muzzle
- Projectile
- Weapon sounds
- Weapon effects
- Recoil

BondTimeSlowComponent
- Time-slow meter and time dilation

BondInteractionComponent
- Interaction detection and command

BondPlayerController
- Widget creation
- UI event binding
```

## Suggested source-file size targets

```text
JamesBondCharacter.cpp       200–350 lines
BondHealthComponent.cpp      100–200 lines
BondWeaponComponent.cpp      150–250 lines
BondWeaponBase.cpp           250–450 lines
BondTimeSlowComponent.cpp    150–250 lines
BondInteractionComponent.cpp 150–250 lines
BondPlayerController.cpp     150–300 lines
```

Split by responsibility, not only by line count.

A system deserves its own class when it has its own:

- State
- Rules
- Events
- Lifecycle
- Testing needs
