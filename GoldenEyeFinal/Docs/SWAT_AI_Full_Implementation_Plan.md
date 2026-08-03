# SWAT AI Full Implementation Plan

**Project:** GoldenEyeFinal  
**Author:** Ju-ve Chankasemporn  
**Goal:** Build a complete SWAT NPC that can patrol, perceive Bond, approach, fire visible projectiles, reload, search, investigate, react to hits, die correctly, and integrate with time slow.

---

## 1. Target Gameplay Loop

```text
Idle / Patrol
→ See Bond
→ Chase until in range
→ Stop and face Bond
→ Fire projectile bursts
→ Reload
→ Lose sight
→ Move to last known location
→ Search
→ Return to patrol
```

Do not add cover, flanking, grenades, or squad tactics until this full loop works.

---

## 2. Final Architecture

```text
BP_SWATEnemy / ASWATEnemyCharacter
├── NPCHealthComponent
├── SWATWeaponComponent
├── SWATCombatComponent
├── CharacterMovement
└── ABP_SWAT

BP_SWATAIController / ASWATAIController
├── AI Perception
├── Blackboard updates
└── Behavior Tree startup

BB_SWAT
└── AI memory and current state

BT_SWAT
└── Chooses movement, combat, search, investigation, and patrol

BP_SWATProjectile
└── Visible projectile movement and damage
```

---

## 3. Responsibility Split

### Codex should implement

- `ASWATAIController`
- AI Perception configuration
- Blackboard synchronization
- `USWATWeaponComponent`
- Projectile spawning
- Ammo and reload
- `USWATCombatComponent`
- Burst timing
- Behavior Tree C++ tasks and services
- Debug getters and logs
- Safe interaction with hit reaction and death

### You should implement

- `BP_SWATAIController`
- `BB_SWAT`
- `BT_SWAT`
- Blueprint assignments
- NavMesh and test level
- Perception tuning
- Projectile visuals
- Weapon muzzle socket
- Behavior Tree decorators
- Animation montage wiring
- Combat timing and accuracy tuning
- Playtesting

---

# Stage 1 — Create the AI Controller and Perception

## Codex task

Give Codex this prompt:

```text
Create ASWATAIController for the existing SWAT enemy.

Inspect first:
- ASWATEnemyCharacter
- existing hit-reaction and death states
- current GameMode and AI setup
- GoldenEyeFinal.Build.cs

Requirements:

1. Derive from AAIController.
2. Use GOLDENEYEFINAL_API.
3. Add:
   - UAIPerceptionComponent
   - UAISenseConfig_Sight
   - UAISenseConfig_Hearing

4. Expose Blueprint-editable perception settings:
   - SightRadius
   - LoseSightRadius
   - PeripheralVisionHalfAngleDegrees
   - HearingRange
   - SightMaxAge
   - HearingMaxAge
   - bDebugPerception

5. Maintain:
   - TargetActor
   - LastKnownLocation
   - LastHeardLocation
   - bHasLineOfSight
   - bShouldInvestigate

6. Sight behavior:
   - only consider the player pawn as the combat target
   - when sight succeeds:
       set TargetActor
       set bHasLineOfSight true
       update LastKnownLocation
   - when sight is lost:
       set bHasLineOfSight false
       preserve LastKnownLocation
       do not continue tracking the player's exact location

7. Hearing behavior:
   - update LastHeardLocation
   - set bShouldInvestigate true
   - do not grant line of sight

8. Add a BehaviorTree asset property.
9. Start the Behavior Tree in OnPossess.
10. Do not move or fire directly from the controller.
11. Do not use Tick for perception polling.
12. Compile and report all modified files.
```

## Your task

Create:

```text
BP_SWATAIController
```

Assign it in `BP_SWATEnemy`:

```text
AI Controller Class = BP_SWATAIController
Auto Possess AI = Placed in World or Spawned
```

Start with:

```text
Sight Radius = 2000
Lose Sight Radius = 2400
Peripheral Vision = 60
Hearing Range = 1800
Sight Max Age = 3
Hearing Max Age = 5
```

### Test

```text
Bond visible
→ target acquired

Bond hides
→ line of sight false
→ last known location remains

Bond fires behind wall
→ sound location recorded
→ no immediate shooting
```

---

# Stage 2 — Create the Blackboard

Create:

```text
BB_SWAT
```

Add these exact keys:

```text
TargetActor
Type: Object
Base Class: Actor

LastKnownLocation
Type: Vector

LastHeardLocation
Type: Vector

HasLineOfSight
Type: Bool

ShouldInvestigate
Type: Bool

IsDead
Type: Bool

IsHitReacting
Type: Bool

NeedsReload
Type: Bool

IsReloading
Type: Bool

IsFiring
Type: Bool

IsInCombat
Type: Bool

DistanceToTarget
Type: Float

IsTooFar
Type: Bool

IsTooClose
Type: Bool

IsInPreferredRange
Type: Bool

HomeLocation
Type: Vector

PatrolLocation
Type: Vector

IsSearching
Type: Bool
```

## Codex task

```text
Update ASWATAIController to synchronize with BB_SWAT.

Use these exact Blackboard key names:

TargetActor
LastKnownLocation
LastHeardLocation
HasLineOfSight
ShouldInvestigate
IsDead
IsHitReacting
NeedsReload
IsReloading
IsFiring
IsInCombat
DistanceToTarget
IsTooFar
IsTooClose
IsInPreferredRange
HomeLocation
IsSearching

Requirements:

- define FName constants for all key names
- initialize HomeLocation when possessing the SWAT pawn
- sight gained:
    set TargetActor
    update LastKnownLocation
    set HasLineOfSight true
    set IsInCombat true
- sight lost:
    set HasLineOfSight false
    preserve LastKnownLocation
- hearing:
    update LastHeardLocation
    set ShouldInvestigate true
- synchronize death and hit-reaction state from ASWATEnemyCharacter
- do not make movement or firing decisions
- compile and report changes
```

---

# Stage 3 — Build the First Behavior Tree Shell

Create:

```text
BT_SWAT
```

Assign:

```text
Blackboard Asset = BB_SWAT
```

Initial structure:

```text
Root
└── Selector
    ├── Dead
    ├── Hit Reaction
    ├── Combat
    ├── Search
    ├── Investigate
    └── Idle
```

Priority is left to right.

## Dead branch

Decorator:

```text
IsDead == true
```

Task:

```text
Wait
```

## Hit Reaction branch

Decorator:

```text
IsHitReacting == true
```

Task:

```text
Wait 0.1
```

## Idle branch

For now:

```text
Wait 1.0
```

Do not add patrol yet.

---

# Stage 4 — Create the Projectile Weapon Component

## Codex task

```text
Create USWATWeaponComponent for ASWATEnemyCharacter.

Requirements:

- derive from UActorComponent
- use GOLDENEYEFINAL_API
- no Tick
- projectile weapon only, no hitscan

Expose:
- ProjectileClass
- MagazineCapacity
- StartingReserveAmmo
- ReloadDuration
- MuzzleSocketName
- BaseSpreadDegrees
- TargetAimHeightOffset

Runtime state:
- CurrentMagazineAmmo
- CurrentReserveAmmo
- bIsReloading
- weapon mesh reference

Functions:
- SetWeaponMesh(USkeletalMeshComponent* InWeaponMesh)
- FireProjectileAt(AActor* Target)
- CanFire()
- StartReload()
- CompleteReload()
- CancelReload()
- StopWeapon()
- GetMagazineAmmo()
- GetReserveAmmo()
- IsReloading()
- NeedsReload()

FireProjectileAt must:
- validate owner, target, projectile class, weapon mesh, and muzzle socket
- reject firing if dead, reloading, or empty
- get muzzle transform
- calculate direction from muzzle to target aim point
- apply random cone spread
- spawn one projectile
- set enemy as owner and instigator
- reduce ammo only when spawn succeeds

Reload must:
- use game-time timer
- transfer only required rounds
- never make reserve ammo negative
- stop on death

Delegates:
- OnAmmoChanged
- OnProjectileFired
- OnReloadStarted
- OnReloadFinished

Compile and report files.
```

## Your task

In `BP_SWATEnemy`:

1. Add or assign the rifle mesh.
2. Attach it to the hand socket.
3. Create or verify:

```text
Muzzle
```

4. On BeginPlay:

```text
SWATWeaponComponent.SetWeaponMesh(WeaponMesh)
```

5. Assign:

```text
Projectile Class = BP_SWATProjectile
Magazine Capacity = 12
Reserve Ammo = 36
Reload Duration = 1.8
Base Spread = 2.5
Muzzle Socket Name = Muzzle
```

Manually test one shot before continuing.

---

# Stage 5 — Create the Visible Projectile

## Codex task

```text
Create ASWATProjectile.

Requirements:

- derive from AActor
- use GOLDENEYEFINAL_API
- USphereComponent root
- UStaticMeshComponent visual
- UProjectileMovementComponent
- no manual movement Tick

Expose:
- Damage
- InitialSpeed
- MaxSpeed
- LifeSpanSeconds
- CollisionRadius

Behavior:
- gravity scale 0
- rotation follows velocity
- ignore owner and instigator
- collide with Bond and world geometry
- apply damage once
- destroy after impact
- remain at CustomTimeDilation 1.0
- naturally respect global time dilation

Do not home toward Bond.
Compile and report files.
```

## Your task

Create:

```text
BP_SWATProjectile
```

Suggested test values:

```text
Speed = 2500
Damage = 10
Life Span = 5
```

Add:

- visible mesh
- emissive material
- optional Niagara trail

Test:

```text
Normal time → projectile moves normally
Time slow → projectile visibly slows
```

---

# Stage 6 — Add Combat Distance State

Use:

```text
Too far
Preferred range
Too close
```

## Codex task

```text
Create UBTService_SWATUpdateCombatState.

Requirements:

- derive from UBTService
- read TargetActor from Blackboard
- calculate distance from SWAT to target
- write:
    DistanceToTarget
    IsTooFar
    IsTooClose
    IsInPreferredRange

Expose:
- TooCloseDistance = 400
- PreferredMaximumDistance = 1500

States must be mutually exclusive.

If target is invalid:
- reset all range Booleans

Do not move or fire.
Use a service interval around 0.2 seconds.
Compile and report files.
```

## Your task

Attach this service to the Combat branch.

Expected:

```text
Distance < 400
→ IsTooClose

400–1500
→ IsInPreferredRange

>1500
→ IsTooFar
```

---

# Stage 7 — Create Burst Combat

## Codex task

```text
Create USWATCombatComponent for controlled projectile bursts.

Requirements:

- derive from UActorComponent
- use GOLDENEYEFINAL_API
- use USWATWeaponComponent
- no Tick

Expose:
- AimPreparationTime
- MinimumBurstShots
- MaximumBurstShots
- TimeBetweenShots
- BurstCooldown

Functions:
- StartCombatBurst(AActor* Target)
- StopCombatBurst()
- CanStartBurst()
- IsBurstActive()
- SetHasLineOfSight(bool)

Sequence:
1. validate target, owner, health, weapon, and line of sight
2. wait AimPreparationTime
3. fire 3–5 projectiles
4. wait TimeBetweenShots between each shot
5. finish burst
6. wait BurstCooldown

Stop when:
- target becomes invalid
- line of sight is lost
- owner dies
- owner is hit-reacting
- weapon starts reloading
- magazine becomes empty

If magazine is empty:
- stop burst
- request reload

Use game-time timers so time slow affects firing cadence.

Delegates:
- OnBurstStarted
- OnShotFired
- OnBurstFinished

Do not spawn projectiles directly.
Do not perform perception.
Compile and report files.
```

Initial values:

```text
Aim Preparation = 0.35
Burst Shots = 3–5
Time Between Shots = 0.12
Burst Cooldown = 0.8
```

---

# Stage 8 — Create the Fire Burst Behavior Tree Task

## Codex task

```text
Create UBTTask_SWATFireBurst.

Requirements:

- read TargetActor from Blackboard
- get ASWATEnemyCharacter
- get USWATCombatComponent
- call StartCombatBurst(TargetActor)
- remain InProgress while burst is active
- complete only when OnBurstFinished broadcasts
- fail if burst cannot start
- stop burst and unbind delegates on abort
- do not bind repeatedly
- compile and report files
```

## Your Combat branch

```text
Combat
Decorator:
TargetActor Is Set
IsDead == false

Service:
SWATUpdateCombatState

Selector:
├── Reload
├── Move Closer
└── Fire Burst
```

### Move Closer

Decorators:

```text
IsTooFar == true
HasLineOfSight == true
```

Task:

```text
Move To TargetActor
```

Acceptance radius:

```text
900–1200
```

### Fire Burst

Decorators:

```text
HasLineOfSight == true
IsInPreferredRange == true
NeedsReload == false
IsHitReacting == false
```

Tasks:

```text
Rotate to Face TargetActor
→ BTTask_SWATFireBurst
```

For the first version, allow firing when too close.

---

# Stage 9 — Add Reload Behavior

## Codex task

```text
Create UBTTask_SWATReload.

Requirements:

- get ASWATEnemyCharacter
- get USWATWeaponComponent
- call StartReload
- remain InProgress while reloading
- complete on OnReloadFinished
- fail if reload cannot start
- clean up delegates on finish and abort
- compile and report files
```

## Your reload branch

Place it first inside Combat:

```text
Decorator:
NeedsReload == true

Task:
BTTask_SWATReload
```

Blueprint animation:

```text
OnReloadStarted
→ Play AM_SWAT_Reload
```

The weapon component remains authoritative for ammo transfer.

---

# Stage 10 — Connect Fire Animation

In `BP_SWATEnemy`:

```text
SWATWeaponComponent.OnProjectileFired
→ Play Anim Montage AM_SWAT_Fire
```

Correct order:

```text
Projectile successfully spawned
→ fire montage plays
```

---

# Stage 11 — Add Last-Known-Location Search

## Codex task

```text
Create UBTTask_SWATLookAround.

Requirements:

- rotate the controlled SWAT pawn through configurable yaw offsets
- expose:
    NumberOfLooks = 3
    DegreesPerLook = 60
    DelayBetweenLooks = 0.6
- do not move the pawn's location
- use game-time timing
- abort cleanly
- complete after the final look
- compile and report files
```

## Your Search branch

Decorators:

```text
TargetActor Is Set
HasLineOfSight == false
IsDead == false
```

Tasks:

```text
Set IsSearching true
→ Move To LastKnownLocation
→ Look Around
→ Clear TargetActor
→ Set IsInCombat false
→ Set IsSearching false
```

Important:

```text
LastKnownLocation must not follow Bond through walls.
```

---

# Stage 12 — Add Hearing Investigation

## Your Investigate branch

Decorators:

```text
ShouldInvestigate == true
TargetActor Is Not Set
IsDead == false
```

Tasks:

```text
Move To LastHeardLocation
→ Look Around
→ Set ShouldInvestigate false
```

## Codex task

```text
Inspect Bond's current firing function and report an AI noise event when a
weapon is successfully fired.

Requirements:

- use Unreal AI hearing/noise support
- Bond is the noise instigator
- use configurable loudness and range
- do not change firing behavior
- hearing must not grant line of sight
- compile and report modified files
```

---

# Stage 13 — Add Patrol

Do this only after combat and search work.

Create patrol points:

```text
TargetPoint_SWAT_01
TargetPoint_SWAT_02
TargetPoint_SWAT_03
```

## Codex task

```text
Create a minimal patrol-point system.

Requirements:

- instance-editable array of AActor patrol points
- current patrol index
- GetCurrentPatrolPoint()
- AdvancePatrolPoint()
- loop when reaching the end
- do not call MoveTo in C++
- compile and report files
```

## Your Patrol branch

```text
Get patrol point
→ Move To PatrolLocation
→ Wait 1–2 seconds
→ Advance patrol point
```

---

# Stage 14 — Handle Hit-Reaction Interruption

Expected behavior:

```text
Hit reaction begins
→ stop burst
→ stop AI movement
→ Blackboard IsHitReacting = true

Hit reaction finishes
→ Blackboard IsHitReacting = false
→ Behavior Tree reevaluates
```

## Codex task

```text
Review the SWAT AI interaction with the existing hit-reaction system.

Requirements:

- active burst stops when hit reaction begins
- AI movement stops during hit reaction
- reload behavior is not duplicated
- Behavior Tree cannot start fire or movement while IsHitReacting is true
- after hit reaction ends, Behavior Tree chooses the next action
- death during hit reaction permanently disables movement and combat
- compile and report changes
```

---

# Stage 15 — Final Behavior Tree

```text
Root
└── Selector
    ├── Dead
    │   └── Wait
    │
    ├── Hit Reaction
    │   └── Wait
    │
    ├── Combat
    │   └── Selector
    │       ├── Reload
    │       ├── Move Closer
    │       ├── Fire Burst
    │       └── Wait
    │
    ├── Search
    │   ├── Move To LastKnownLocation
    │   ├── Look Around
    │   └── Clear Target
    │
    ├── Investigate
    │   ├── Move To LastHeardLocation
    │   ├── Look Around
    │   └── Clear Investigation
    │
    └── Patrol
        ├── Move To PatrolLocation
        ├── Wait
        └── Advance Patrol
```

---

# Stage 16 — Testing Order

## Test 1: Perception

```text
See Bond
Lose Bond
Hear Bond
```

## Test 2: Movement

```text
Too far
→ approach

Preferred range
→ stop
```

## Test 3: Firing

```text
Aim delay
→ finite projectile burst
→ cooldown
```

## Test 4: Reload

```text
Magazine reaches zero
→ stop firing
→ reload
→ resume combat
```

## Test 5: Line-of-sight loss

```text
Bond hides
→ no shooting through wall
→ move to last known location
→ search
```

## Test 6: Hit reaction

```text
Shot lands
→ burst stops
→ movement stops
→ montage plays
→ AI resumes
```

## Test 7: Death

```text
Lethal hit
→ movement permanently stops
→ combat permanently stops
→ death montage plays
```

## Test 8: Time slow

```text
SWAT movement slows
Projectile slows
Fire cadence slows
Reload slows
Animations slow
Bond remains responsive
```

---

# Initial Tuning Values

```text
Sight Radius = 2000
Lose Sight Radius = 2400
Preferred Range = 400–1500
Projectile Speed = 2500
Projectile Damage = 10
Aim Delay = 0.35
Burst = 3–5 shots
Shot Interval = 0.12
Burst Cooldown = 0.8
Magazine = 12
Reserve Ammo = 36
Reload = 1.8 seconds
```

---

# Definition of Complete

The SWAT AI is complete when one enemy can:

```text
Patrol
→ hear a shot
→ investigate
→ see Bond
→ approach
→ stop in range
→ fire visible projectile bursts
→ reload
→ stop firing when Bond hides
→ search the last known location
→ react to non-lethal hits
→ resume behavior
→ die permanently from lethal damage
→ behave correctly during time slow
```

---

# Recommended Immediate Next Step

Implement only these first:

```text
1. ASWATAIController
2. BB_SWAT
3. Empty BT_SWAT shell
```

Do not start projectile burst logic until perception and Blackboard values are visibly correct.
