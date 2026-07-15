# Step 5 — Blueprint and UI setup

This project uses first-person arms only.

Do not assign a full-body mesh.

Do not update UI using Tick or UMG property bindings that run every frame.

## Recommended assets

```text
Content/Blueprints/
├── Characters/Player/
│   └── BP_JamesBond
├── Weapons/Pistol/
│   └── BP_Pistol
├── GameModes/
│   └── BP_GoldenEyeGameMode
├── Player/
│   └── BP_BondPlayerController
└── UI/
    ├── WBP_BondHUD
    └── WBP_TimeSlowHUD
```

## 1. Final `BP_JamesBond` hierarchy

```text
BP_JamesBond
├── CapsuleComponent
├── ArrowComponent
├── Mesh (CharacterMesh0)        ← leave empty and hidden
├── FirstPersonCamera
│   └── FirstPersonArms
│       └── WeaponRoot
├── BondAudioComponent
├── HealthComponent
├── WeaponComponent
├── TimeSlowComponent
├── InteractionComponent
└── CharacterMovement
```

## 2. Inherited mesh setup

`ACharacter` always contains:

```text
Mesh (CharacterMesh0)
```

Configure it as:

```text
Skeletal Mesh = None
Animation Class = None
Hidden in Game = true
Visible = false
Collision Preset = NoCollision
Generate Overlap Events = false
```

Do not assign a body mesh later unless the project direction changes.

## 3. First-person arms setup

Select `FirstPersonArms`.

Assign:

```text
Skeletal Mesh = first-person arms mesh
Animation Class = first-person arms Animation Blueprint
```

Recommended settings:

```text
Only Owner See = true
Owner No See = false
Collision Preset = NoCollision
Generate Overlap Events = false
Cast Shadow = false
```

The arms remain attached to `FirstPersonCamera`.

Adjust the arms relative transform rather than repeatedly moving the camera.

## 4. Camera setup

Select `FirstPersonCamera`.

Suggested starting location:

```text
X = -10
Y = 0
Z = 64
```

Confirm:

```text
Use Pawn Control Rotation = true
```

Do not add a second camera.

## 5. Weapon attachment setup

Initial hierarchy:

```text
FirstPersonCamera
└── FirstPersonArms
    └── WeaponRoot
```

At runtime:

```text
WeaponComponent
→ Spawn BP_Pistol
→ Attach BP_Pistol to WeaponRoot
```

Do not manually add the USP to Bond.

When animations are stable, optionally attach to an arms skeleton socket:

```text
weapon_socket
```

## 6. `BP_Pistol` hierarchy

```text
BP_Pistol
├── Root
├── WeaponMesh
├── MuzzlePoint
```

Configure:

```text
Pistol mesh
Muzzle socket
Projectile class
Fire sound
Reload sound
Muzzle effect
Ammo values
Fire interval
Recoil values
```

The optional muzzle light should be off by default.

## 7. Audio separation

```text
BondAudioComponent
- Pain
- Breathing
- Voice lines

BP_Pistol
- Fire
- Reload
- Dry fire
- Equip
```

## 8. Create `WBP_BondHUD`

Recommended hierarchy:

```text
Canvas Panel
├── Crosshair
├── AmmoContainer
│   ├── CurrentAmmoText
│   └── ReserveAmmoText
├── DamageOverlay
│   └── DamageIndicatorImage
└── OptionalDebugHealthText
```

This project uses regenerating health with a central damage indicator.

Do not rely on a traditional health bar for the final HUD.

Create these functions:

```text
UpdateDamageState(
    CurrentHealth,
    MaxHealth,
    HealthPercent
)

UpdateAmmo(
    MagazineAmmo,
    ReserveAmmo
)

PlayDamageFlash(
    DamageAmount
)

SetRegenerating(
    IsRegenerating
)
```

Inside `UpdateDamageState`:

```text
DamagePercent = 1 - HealthPercent
VisualIntensity = DamagePercent × DamagePercent
Set DamageIndicatorImage opacity from VisualIntensity
```

Suggested appearance:

```text
Health 100% → invisible
Health 75%  → very faint
Health 50%  → visible
Health 25%  → strong
Health 10%  → strong pulse
```

## 9. Create `WBP_TimeSlowHUD`

Recommended elements:

```text
Time-slow progress bar
Optional percentage text
Active-state visual
```

Create functions:

```text
UpdateTimeSlowMeter(CurrentMeter, MaxMeter)
SetTimeSlowActive(IsActive)
```

## 10. Event-driven UI

Preferred flow:

```text
HealthComponent
→ OnHealthChanged
→ WBP_BondHUD.UpdateDamageState

HealthComponent
→ OnDamageTaken
→ WBP_BondHUD.PlayDamageFlash

HealthComponent
→ OnRegenerationStateChanged
→ WBP_BondHUD.SetRegenerating

HealthComponent
→ OnDeath
→ PlayerController starts Bond death flow

Equipped Weapon
→ OnAmmoChanged
→ WBP_BondHUD.UpdateAmmo

TimeSlowComponent
→ OnMeterChanged
→ WBP_TimeSlowHUD.UpdateTimeSlowMeter

TimeSlowComponent
→ OnTimeSlowStateChanged
→ WBP_TimeSlowHUD.SetTimeSlowActive
```

## 11. Recommended UI ownership

Use a custom Player Controller:

```text
ABondPlayerController
├── Creates WBP_BondHUD
├── Creates WBP_TimeSlowHUD
├── Gets possessed Bond
└── Binds gameplay delegates to widgets
```

This keeps widget lifecycle out of Bond.

## 12. `BP_JamesBond` assignments

Assign:

```text
IMC_JamesBond
IA_Move
IA_Look
IA_Jump
IA_Fire
IA_Reload
IA_TimeSlow
First-person arms mesh
First-person arms Animation Blueprint
Default Weapon Class = BP_Pistol
```

Do not assign:

```text
Body mesh
Third-person Animation Blueprint
Permanent gun
Permanent spotlight
```

## 13. GameMode assignments

In `BP_GoldenEyeGameMode`:

```text
Default Pawn Class = BP_JamesBond
Player Controller Class = BP_BondPlayerController
```

## Avoid these mistakes

- Duplicate cameras
- A manually placed Bond plus GameMode spawning
- Ammo stored on Bond
- Health stored in the HUD
- Traditional health bar used as the primary final damage display
- Permanent USP component on Bond
- Permanent spotlight for muzzle flash
- UI bindings that evaluate every frame
- Animation Blueprint deciding whether reload is allowed

## Final validation

- Only first-person arms are visible.
- The inherited mesh is empty.
- Health lives in the health component.
- Weapon ownership lives in the weapon component.
- Ammo and reload live in the weapon actor.
- Time slow lives in its component.
- Weapon effects and audio live in `BP_Pistol`.
- UI updates through delegates.
- The Bond Event Graph remains small.