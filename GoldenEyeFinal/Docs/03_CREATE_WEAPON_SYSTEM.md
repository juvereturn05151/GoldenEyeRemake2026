# Step 3 — Create the weapon system

This stage creates:

```text
ABondWeaponBase
UBondWeaponComponent
BP_Pistol
```

The weapon actor owns ammo, firing, reload, audio, and effects.

The weapon component owns the equipped weapon.

The USP must not be a permanent component of `BP_JamesBond`.

## Final runtime hierarchy

```text
FirstPersonCamera
└── FirstPersonArms
    └── WeaponRoot
        └── Spawned BP_Pistol
```

There is no body mesh.

## Part A — Create `ABondWeaponBase`

Create a new C++ class derived from `Actor`.

Name it:

```text
BondWeaponBase
```

### Responsibilities

The weapon base owns:

- Root component
- Weapon mesh
- Muzzle transform
- Optional shell-ejection transform
- Magazine ammo
- Reserve ammo
- Fire interval
- Reload state
- Projectile class
- Fire sound
- Reload sound
- Muzzle effect
- Optional temporary muzzle light

Suggested Blueprint hierarchy:

```text
BP_Pistol
├── Root
├── WeaponMesh
├── MuzzlePoint
```

A muzzle light must be disabled by default and enabled only briefly when firing.

Do not use a permanently enabled spotlight.

### Suggested interface

```cpp
UFUNCTION(BlueprintCallable)
virtual void StartFire();

UFUNCTION(BlueprintCallable)
virtual void StopFire();

UFUNCTION(BlueprintCallable)
virtual void StartReload();

UFUNCTION(BlueprintCallable)
virtual void CompleteReload();

UFUNCTION(BlueprintPure)
bool CanFire() const;

UFUNCTION(BlueprintPure)
bool CanReload() const;
```

Suggested delegate:

```cpp
OnAmmoChanged(MagazineAmmo, ReserveAmmo)
```

For the first compile, firing may only:

```text
Check CanFire
→ Decrease ammo
→ Broadcast OnAmmoChanged
→ Print a log message
```

Add projectile spawning later.

## Part B — Create `UBondWeaponComponent`

Create an Actor Component named:

```text
BondWeaponComponent
```

### Responsibilities

- Default weapon class
- Equipped weapon reference
- Spawn default weapon
- Equip weapon
- Unequip weapon
- Forward fire
- Forward reload
- Attach the weapon to WeaponRoot

It must not own:

- Magazine capacity
- Reserve ammo
- Fire rate
- Reload duration
- Muzzle effects
- Weapon sounds

### Suggested interface

```cpp
UFUNCTION(BlueprintCallable)
void SpawnDefaultWeapon();

UFUNCTION(BlueprintCallable)
void EquipWeapon(ABondWeaponBase* NewWeapon);

UFUNCTION(BlueprintCallable)
void StartFire();

UFUNCTION(BlueprintCallable)
void StopFire();

UFUNCTION(BlueprintCallable)
void Reload();

UFUNCTION(BlueprintPure)
ABondWeaponBase* GetEquippedWeapon() const;
```

Properties:

```cpp
UPROPERTY(EditDefaultsOnly)
TSubclassOf<ABondWeaponBase> DefaultWeaponClass;

UPROPERTY(VisibleInstanceOnly)
TObjectPtr<ABondWeaponBase> EquippedWeapon;
```

## Part C — Add the weapon component to Bond

In `JamesBondCharacter.h`:

```cpp
class UBondWeaponComponent;
```

Add:

```cpp
UPROPERTY(
	VisibleAnywhere,
	BlueprintReadOnly,
	Category = "Bond|Components",
	meta = (AllowPrivateAccess = "true")
)
TObjectPtr<UBondWeaponComponent> WeaponComponent;
```

In the constructor:

```cpp
WeaponComponent =CreateDefaultSubobject<UBondWeaponComponent>(TEXT("WeaponComponent"));
```

Bond should expose:

```cpp
USceneComponent* GetWeaponRoot() const;
```

## Part D — Attachment flow

```text
Bond begins play
→ WeaponComponent checks DefaultWeaponClass
→ Spawns BP_Pistol
→ Sets Bond as owner
→ Attaches BP_Pistol to WeaponRoot
→ Uses Snap to Target
```

For the early prototype, attach to `WeaponRoot`.

Later, after first-person animations are stable, you may attach to a socket on the arms skeleton:

```text
weapon_socket
```

## Part E — Add input

Create:

```text
IA_Fire
IA_Reload
```

Settings:

```text
IA_Fire = Digital
IA_Reload = Digital
```

Mappings:

```text
Left Mouse Button = IA_Fire
R = IA_Reload
```

Bond input handlers should only forward commands:

```cpp
void AJamesBondCharacter::HandleFireStarted()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartFire();
	}
}

void AJamesBondCharacter::HandleFireCompleted()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
	}
}

void AJamesBondCharacter::HandleReload()
{
	if (WeaponComponent)
	{
		WeaponComponent->Reload();
	}
}
```

Bond must not subtract ammo itself.

## Part F — Create `BP_Pistol`

Create a Blueprint child of `ABondWeaponBase`.

Name it:

```text
BP_Pistol
```

Recommended folder:

```text
Content/Blueprints/Weapons/Pistol/
```

Configure:

```text
Weapon Mesh = PistolUSP mesh
Magazine Capacity = 8
Starting Reserve Ammo = 32
Fire Interval = 0.25
Muzzle Socket Name = Muzzle
```

Add or configure:

```text
MuzzlePoint
```

Preferred socket names:

```text
Muzzle
```

## Part G — Configure `BP_JamesBond`

Open `BP_JamesBond`.

Select `WeaponComponent`.

Assign:

```text
Default Weapon Class = BP_Pistol
```

Do not manually add `BP_Pistol` to the component tree.

Do not add a permanent `Gun` mesh under `FirstPersonArms`.

## Audio ownership

```text
BondAudioComponent
- Pain
- Breathing
- Voice lines

BP_Pistol
- Fire
- Dry fire
- Reload
- Equip
```

## Muzzle flash ownership

```text
BP_Pistol fires
→ Spawn Niagara effect at Muzzle
→ Play fire sound
→ perform hitscan
```

## Compile checkpoints

Compile after:

1. Creating `ABondWeaponBase`.
2. Creating `UBondWeaponComponent`.
3. Adding the weapon component to Bond.
4. Adding fire and reload input.
5. Creating and assigning `BP_Pistol`.
6. Verifying attachment.
7. Implementing ammo decrease.
8. Implementing reload.

## Validation

- One USP spawns.
- USP attaches to WeaponRoot.
- Ammo belongs to the weapon.
- Reload belongs to the weapon.
- Weapon sounds belong to the weapon.
- Muzzle effects originate from the weapon.
- No permanent gun component exists on Bond.
- No body mesh is used.
