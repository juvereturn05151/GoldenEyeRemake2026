# Step 4 — Create `UBondTimeSlowComponent`

This component owns the complete Max Payne-style time-slow system.

This system is independent of meshes and works with the first-person-arms-only character.

## Responsibilities

- Current meter
- Maximum meter
- Drain rate
- Regeneration rate
- Activation
- Deactivation
- Global time dilation
- Meter-change events
- State-change events

## 1. Create the component

Create an Actor Component named:

```text
BondTimeSlowComponent
```

## 2. Suggested API

```cpp
UFUNCTION(BlueprintCallable)
void StartTimeSlow();

UFUNCTION(BlueprintCallable)
void StopTimeSlow();

UFUNCTION(BlueprintPure)
bool CanStartTimeSlow() const;

UFUNCTION(BlueprintPure)
bool IsTimeSlowActive() const;

UFUNCTION(BlueprintPure)
float GetMeterPercent() const;
```

Suggested settings:

```cpp
float MaxMeter = 100.0f;
float CurrentMeter = 100.0f;
float DrainPerRealSecond = 20.0f;
float RegenerationPerRealSecond = 10.0f;
float TimeDilation = 0.25f;
float RegenerationDelay = 1.0f;
bool bIsActive = false;
```

## 3. Tick policy

This component may tick because drain and regeneration are continuous.

Important: global time dilation changes ordinary Delta Seconds.

Use real-time delta or compensate for time dilation so the meter drains at the intended real-world rate.

## 4. Events

Create:

```text
OnMeterChanged(CurrentMeter, MaxMeter)
OnTimeSlowStateChanged(IsActive)
```

Do not let the component create or update widgets directly.

## 5. Add the component to Bond

In the constructor:

```cpp
TimeSlowComponent =CreateDefaultSubobject<UBondTimeSlowComponent>(TEXT("TimeSlowComponent"));
```

Create input:

```text
IA_TimeSlow = Digital
```

Mapping:

```text
Right Mouse Button = IA_TimeSlow
```

Bind:

```text
Started → StartTimeSlow
Completed → StopTimeSlow
Canceled → StopTimeSlow
```

Bond should only forward input.

## 6. Blueprint setup

Open `BP_JamesBond`.

Select `TimeSlowComponent`.

Suggested values:

```text
Max Meter = 100
Drain Per Real Second = 20
Regeneration Per Real Second = 10
Regeneration Delay = 1
Time Dilation = 0.25
```

Assign `IA_TimeSlow` under Class Defaults.

## 7. Test before UI

Verify:

- Right mouse activates time slow.
- Releasing right mouse stops it.
- Reaching zero forces it to stop.
- Meter regenerates only while inactive.
- Gameplay returns to normal speed.
- Drain is consistent in real time.
- First-person arm animation still behaves acceptably during time slow.

## Validation

- Time-slow variables do not exist on Bond.
- The component does not own widgets.
- Bond only forwards input.
- No body mesh is required.
