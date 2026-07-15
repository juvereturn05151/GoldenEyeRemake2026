# Step 2 — Create `UBondHealthComponent`
## Gears-of-War-Inspired Regenerating Health

This system uses a regenerating-health model inspired by cover shooters:

```text
Bond takes damage
→ Regeneration delay resets
→ Damage indicator becomes more intense
→ If Bond avoids damage for the delay duration
→ Health regenerates automatically
→ Damage indicator fades as health returns
```

Bond does not use health pickups for normal recovery.

The gameplay component owns health and regeneration rules.
The HUD only displays the current damage state.

---

## Responsibilities

`UBondHealthComponent` owns:

- Maximum health
- Current health
- Damage handling
- Regeneration delay
- Automatic regeneration
- Death state
- Health-change events
- Damage-state percentage for the HUD

It does not own:

- HUD widgets
- Screen effects
- Sound effects
- Camera effects
- Weapon damage calculations

---

# Part A — Create `UBondHealthComponent`

## 1. Create the class

In Unreal Editor:

1. Open **Tools → New C++ Class**.
2. Choose **Actor Component**.
3. Name it `BondHealthComponent`.
4. Close Unreal Editor before compiling in Visual Studio.

---

## 2. Health behavior

Recommended starting values:

```text
Max Health = 100
Regeneration Delay = 4 seconds
Regeneration Rate = 25 health per second
Low Health Threshold = 0.30
```

Example:

```text
Bond takes 40 damage
→ Current Health becomes 60
→ Regeneration timer resets
→ Bond avoids damage for 4 seconds
→ Health regenerates at 25 per second
→ Regeneration stops at 100
```

Any new damage must:

```text
Stop regeneration
→ Reset the regeneration delay
→ Increase the HUD damage effect
```

---

## 3. `BondHealthComponent.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BondHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FBondHealthChangedSignature,
    float,
    CurrentHealth,
    float,
    MaxHealth,
    float,
    HealthPercent
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FBondDamageTakenSignature,
    float,
    DamageAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FBondDeathSignature
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FBondRegenerationStateChangedSignature,
    bool,
    bIsRegenerating
);

UCLASS(
    ClassGroup = (Bond),
    meta = (BlueprintSpawnableComponent)
)
class GOLDENEYEFINAL_API UBondHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBondHealthComponent();

    UFUNCTION(BlueprintCallable, Category = "Bond|Health")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Bond|Health")
    void RestoreFullHealth();

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    float GetDamagePercent() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    bool IsDead() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    bool IsRegenerating() const;

    UFUNCTION(BlueprintPure, Category = "Bond|Health")
    bool IsLowHealth() const;

    UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
    FBondHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
    FBondDamageTakenSignature OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
    FBondDeathSignature OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
    FBondRegenerationStateChangedSignature
    OnRegenerationStateChanged;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Bond|Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Bond|Health"
    )
    float CurrentHealth = 100.0f;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Bond|Health|Regeneration",
        meta = (ClampMin = "0.0")
    )
    float RegenerationDelay = 4.0f;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Bond|Health|Regeneration",
        meta = (ClampMin = "0.0")
    )
    float RegenerationRate = 25.0f;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Bond|Health",
        meta = (ClampMin = "0.0", ClampMax = "1.0")
    )
    float LowHealthThreshold = 0.30f;

    bool bIsDead = false;
    bool bIsRegenerating = false;

    FTimerHandle RegenerationDelayTimer;
    FTimerHandle RegenerationTickTimer;

    void ScheduleRegeneration();
    void StartRegeneration();
    void RegenerateHealth();
    void StopRegeneration();
    void BroadcastHealthChanged();
};
```

---

## 4. `BondHealthComponent.cpp`

```cpp
#include "BondHealthComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

UBondHealthComponent::UBondHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBondHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bIsDead = false;
    bIsRegenerating = false;

    BroadcastHealthChanged();
}

void UBondHealthComponent::ApplyDamage(float DamageAmount)
{
    if (bIsDead || DamageAmount <= 0.0f)
    {
        return;
    }

    StopRegeneration();

    GetWorld()->GetTimerManager().ClearTimer(RegenerationDelayTimer);

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount,
        0.0f,
        MaxHealth
    );

    OnDamageTaken.Broadcast(DamageAmount);
    BroadcastHealthChanged();

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        OnDeath.Broadcast();
        return;
    }

    ScheduleRegeneration();
}

void UBondHealthComponent::ScheduleRegeneration()
{
    if (bIsDead ||CurrentHealth >= MaxHealth || RegenerationRate <= 0.0f)
    {
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(RegenerationDelayTimer, this, &UBondHealthComponent::StartRegeneration, RegenerationDelay, false);
}

void UBondHealthComponent::StartRegeneration()
{
    if (bIsDead ||CurrentHealth >= MaxHealth ||bIsRegenerating)
    {
        return;
    }

    bIsRegenerating = true;
    OnRegenerationStateChanged.Broadcast(true);

    constexpr float RegenerationTickInterval = 0.05f;

    GetWorld()->GetTimerManager().SetTimer(
        RegenerationTickTimer,
        this,
        &UBondHealthComponent::RegenerateHealth,
        RegenerationTickInterval,
        true
    );
}

void UBondHealthComponent::RegenerateHealth()
{
    if (bIsDead || CurrentHealth >= MaxHealth)
    {
        StopRegeneration();
        return;
    }

    const float TimerInterval =
        GetWorld()->GetTimerManager().GetTimerRate(
            RegenerationTickTimer
        );

    CurrentHealth = FMath::Clamp(
        CurrentHealth +
            (RegenerationRate * TimerInterval),
        0.0f,
        MaxHealth
    );

    BroadcastHealthChanged();

    if (CurrentHealth >= MaxHealth)
    {
        StopRegeneration();
    }
}

void UBondHealthComponent::StopRegeneration()
{
    GetWorld()->GetTimerManager().ClearTimer(RegenerationTickTimer);

    if (bIsRegenerating)
    {
        bIsRegenerating = false;
        OnRegenerationStateChanged.Broadcast(false);
    }
}

void UBondHealthComponent::RestoreFullHealth()
{
    if (bIsDead)
    {
        return;
    }

    StopRegeneration();

    GetWorld()->GetTimerManager().ClearTimer(RegenerationDelayTimer);

    CurrentHealth = MaxHealth;
    BroadcastHealthChanged();
}

float UBondHealthComponent::GetCurrentHealth() const
{
    return CurrentHealth;
}

float UBondHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

float UBondHealthComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float UBondHealthComponent::GetDamagePercent() const
{
    return 1.0f - GetHealthPercent();
}

bool UBondHealthComponent::IsDead() const
{
    return bIsDead;
}

bool UBondHealthComponent::IsRegenerating() const
{
    return bIsRegenerating;
}

bool UBondHealthComponent::IsLowHealth() const
{
    return !bIsDead && GetHealthPercent() <= LowHealthThreshold;
}

void UBondHealthComponent::BroadcastHealthChanged()
{
    OnHealthChanged.Broadcast(
        CurrentHealth,
        MaxHealth,
        GetHealthPercent()
    );
}
```

---

# Part B — Add the component to Bond

In `JamesBondCharacter.h`:

```cpp
class UBondHealthComponent;
```

Add:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UBondHealthComponent> HealthComponent;
```

In `JamesBondCharacter.cpp`:

```cpp
#include "BondHealthComponent.h"
```

In the constructor:

```cpp
HealthComponent =
    CreateDefaultSubobject<UBondHealthComponent>(
        TEXT("HealthComponent")
    );
```

Bond should not store duplicate health variables.

---

# Part C — Blueprint setup

Open `BP_JamesBond`.

Select:

```text
HealthComponent
```

Recommended starting values:

```text
Max Health = 100
Regeneration Delay = 4
Regeneration Rate = 25
Low Health Threshold = 0.30
```

The Blueprint Event Graph should remain empty.

---

# Part D — HUD design

Instead of relying mainly on a traditional health bar, use a central damage indicator.

Recommended widget:

```text
WBP_BondHUD
```

Suggested hierarchy:

```text
Canvas Panel
├── Crosshair
├── AmmoContainer
│   ├── CurrentAmmoText
│   └── ReserveAmmoText
├── DamageOverlay
│   └── DamageIndicatorImage
```

The damage image should be:

```text
Centered on the screen
Transparent when healthy
More visible as damage increases
Most intense near death
```

Do not copy copyrighted artwork from another game.

Create an original circular or iris-style damage indicator suitable for GoldenEye.

---

## HUD update function

Create this Blueprint function:

```text
UpdateDamageState(HealthPercent)
```

Logic:

```text
DamagePercent = 1 - HealthPercent
Set DamageOmenImage opacity using DamagePercent
Set scale or material intensity using DamagePercent
```

Recommended visual curve:

```text
Health 100% → Indicator invisible
Health 75%  → Very faint
Health 50%  → Clearly visible
Health 25%  → Strong
Health 10%  → Very strong and pulsing
```

A nonlinear mapping usually feels better:

```text
Visual Intensity = DamagePercent squared
```

This keeps the screen clean at minor damage and makes severe damage much more obvious.

---

## Low-health pulse

When:

```text
HealthPercent <= 0.30
```

the HUD may:

```text
Pulse the damage indicator
Play a heartbeat
Add subtle camera feedback
```

Stop the effect when health regenerates above the threshold.

Heartbeat audio should be handled by Bond audio or a dedicated UI/audio system, not by the health component.

---

# Part E — Event binding

Preferred flow:

```text
HealthComponent
→ OnHealthChanged
→ WBP_BondHUD.UpdateDamageState

HealthComponent
→ OnDamageTaken
→ Play brief damage flash

HealthComponent
→ OnRegenerationStateChanged
→ WBP_BondHUD.SetRegenerating

HealthComponent
→ OnDeath
→ PlayerController starts Bond death flow
```

The health component must not directly reference the widget.

Use `ABondPlayerController` to create the HUD and bind these delegates.

---

# Part F — Damage testing

Create a temporary debug input or damage volume.

Test:

```text
1. Apply 25 damage
2. Confirm health becomes 75
3. Confirm the damage indicator appears faintly
4. Wait less than 4 seconds
5. Confirm health does not regenerate yet
6. Wait beyond 4 seconds
7. Confirm health regenerates smoothly
8. Take damage during regeneration
9. Confirm regeneration stops and delay resets
10. Apply lethal damage
11. Confirm regeneration never starts after death
```

Remove temporary debug input after validation.

---

# Time-slow consideration

Recommended default behavior:

```text
Time slow active
→ Regeneration delay and regeneration are also slowed
```

This is the simplest behavior because Unreal timers follow world time.

Do not compensate for time dilation unless health should regenerate at normal real-world speed during slow motion.

---

# Validation

- Health automatically regenerates after a delay.
- Taking damage resets the delay.
- Damage interrupts active regeneration.
- Regeneration stops at maximum health.
- Death prevents regeneration.
- HUD intensity is driven by missing-health percentage.
- Health logic remains inside `UBondHealthComponent`.
- HUD logic remains outside the component.
- No body mesh is required.
- No health pickup is required for normal recovery.