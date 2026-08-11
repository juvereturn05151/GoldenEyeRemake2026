# Boris Escort Mission System – GoldenEye

## Goal

Create an escort/sequence mission involving an NPC named **Boris**.

The intended mission flow is:

1. Boris starts idle.
2. Boris sees Bond.
3. Boris plays a **Hands Up** animation.
4. When the animation finishes, Boris moves to **Point A**.
5. At Point A, Boris stops and says an annoying dialogue line.
6. Boris waits there until Bond shoots him.
7. If the shot does not kill Boris:
   - Boris plays a **Hurt Reaction** animation.
   - When the hurt animation finishes, Boris moves to the Computer.
8. Boris reaches the Computer.
9. Boris plays an **Activate Computer** animation.
10. When activation finishes:
   - The Computer becomes activated.
   - A boolean on the Computer is set to `true`.
   - The mission objective completes.
11. If Boris dies before the mission completes:
   - Play Boris's death animation.
   - Trigger Game Over.
12. If Boris dies after the mission completes:
   - Play death animation.
   - Do not trigger Game Over.

---

# Core Mission Flow

```text
Idle
 ↓
Sees Bond
 ↓
HandsUp
 ↓ animation finishes
MoveToPointA
 ↓ arrives
WaitingAtPointA
 ↓ dialogue
Wait for Bond to shoot Boris
 ↓
HurtReaction
 ↓ animation finishes
MoveToComputer
 ↓ arrives
ActivateComputer
 ↓ activation animation finishes
Computer Activated
 ↓
MissionComplete
```

Death handling:

```text
Boris Dies
   ↓
Was Mission Completed?
   ├─ No  → Play Death Anim → Game Over
   └─ Yes → Play Death Anim → Normal Death
```

---

# Recommended State Machine

Create a mission state enum similar to:

```cpp
enum class EBorisMissionState : uint8
{
    Idle,
    HandsUp,
    MovingToPointA,
    WaitingAtPointA,
    HurtReacting,
    MovingToComputer,
    ActivatingComputer,
    Completed,
    Dead
};
```

Also keep a separate boolean:

```cpp
bool bMissionCompleted = false;
```

This is important because once Boris dies, his current state becomes `Dead`.

Do not rely only on `CurrentState` to determine whether the mission was already complete.

---

# Recommended Architecture

```text
ABorisCharacter
    ├─ Mission State
    ├─ Health / Damage handling
    ├─ Animation montage triggers
    ├─ Point A reference
    ├─ Computer reference
    ├─ Mission completion flag
    └─ Mission delegates

ABorisAIController
    ├─ Detect Bond
    ├─ Move Boris to Point A
    └─ Move Boris to Computer

Behavior Tree
    ├─ Idle
    ├─ Moving
    └─ Waiting

Computer Actor
    ├─ bool bActivated
    ├─ ActivateComputer()
    └─ IsActivated()

Mission System
    ├─ Receives Boris mission completion
    └─ Completes objective

GameMode / Failure System
    └─ Boris dies before completion → Game Over
```

---

# PART 1 – What to Give Codex

Copy and paste the following prompt into Codex.

```text
Implement a Boris escort/sequence mission system for my Unreal Engine GoldenEye project.

Existing architecture:
- Player character is AJamesBondCharacter.
- Existing AI systems already use Unreal AIController / Behavior Tree patterns.
- Existing mission objectives are active simultaneously.
- Mission completion is event-driven.
- Do not redesign unrelated systems.

Create or extend a Boris NPC implementation with a clear mission state machine.

==================================================
MISSION STATES
==================================================

Add an enum similar to:

EBorisMissionState
{
    Idle,
    HandsUp,
    MovingToPointA,
    WaitingAtPointA,
    HurtReacting,
    MovingToComputer,
    ActivatingComputer,
    Completed,
    Dead
}

Track:

- CurrentMissionState
- bool bMissionCompleted

Do not use CurrentMissionState alone to determine mission-failure eligibility after Boris dies.

==================================================
REFERENCES
==================================================

Expose editable references for:

- AActor* PointA
- AActor* ComputerTarget

These should be assignable in Unreal Editor / Blueprint.

Do not hardcode level actor names.

==================================================
PLAYER DETECTION
==================================================

Boris starts in:

Idle

When Boris detects AJamesBondCharacter:

- Do not move immediately.
- Stop movement if needed.
- Set state to HandsUp.
- Play a Hands Up montage.
- Ignore duplicate player-detection events while Boris is already progressing through the mission.

When the Hands Up montage finishes:

- Transition to MovingToPointA.
- Command Boris AI to move to PointA.

Use animation completion / AnimNotify flow.
Do not use an arbitrary hardcoded delay if avoidable.

==================================================
POINT A
==================================================

When Boris reaches PointA:

- Stop movement.
- Set state to WaitingAtPointA.
- Broadcast an event/delegate such as OnReachedPointA.
- Boris should remain there indefinitely until the player damages him.

Do not automatically continue after a timer.

The dialogue itself will be handled in Blueprint/UI/audio.

==================================================
DAMAGE / PROVOKE BEHAVIOR
==================================================

While Boris is in WaitingAtPointA:

If Boris receives non-lethal damage:

- Stop movement.
- Set state to HurtReacting.
- Play Hurt Reaction montage.
- Do not immediately move to the Computer.

When the Hurt montage finishes:

- Set state to MovingToComputer.
- Move Boris to ComputerTarget.

If Boris takes damage in unrelated states:
- Keep normal damage behavior.
- Do not restart mission transitions.
- Do not accidentally trigger the Point A provoke flow outside WaitingAtPointA.

==================================================
DEATH
==================================================

If Boris health reaches zero:

- Stop movement.
- Stop/disable normal mission movement logic.
- Set state to Dead.
- Play Death montage.

If bMissionCompleted == false:
- Broadcast an event/delegate such as OnBorisDiedBeforeMissionComplete.
- This event will be connected to Game Over outside the Boris class.

If bMissionCompleted == true:
- Do not trigger mission failure.
- Boris may die normally after the mission.

Prevent duplicate death handling.

==================================================
COMPUTER ARRIVAL
==================================================

When Boris reaches ComputerTarget:

- Stop movement.
- Face the Computer if practical.
- Set state to ActivatingComputer.
- Play Activate Computer montage.

Do not mark the Computer activated immediately on arrival.

Wait until the activation animation reaches completion.

==================================================
ACTIVATE ANIMATION
==================================================

Use animation completion / AnimNotify behavior.

When the Activate Computer animation finishes:

- Call an activation function on the ComputerTarget.
- Set the Computer's activation boolean to true.
- Set bMissionCompleted = true.
- Set CurrentMissionState = Completed.
- Broadcast OnBorisMissionCompleted.

IMPORTANT:
Set bMissionCompleted = true before or at the same moment as broadcasting mission completion.
Do not broadcast completion first and set the flag later.

==================================================
COMPUTER ACTOR
==================================================

If a suitable reusable Computer actor does not already exist, create one.

It should have:

bool bActivated = false;

Functions similar to:

ActivateComputer()
IsActivated()

ActivateComputer() should:

- Set bActivated = true.
- Prevent duplicate activation if appropriate.
- Broadcast an optional OnComputerActivated event.

Do not put the next mission's logic inside Boris.

The next mission should be able to query:

Computer->IsActivated()

==================================================
ANIMATION MONTAGES
==================================================

Expose montage references for:

- HandsUpMontage
- HurtMontage
- ActivateComputerMontage
- DeathMontage

Do not hardcode animation assets in C++.

Prefer to assign these in BP_Boris.

Use AnimNotifies or montage-ended callbacks to transition gameplay state.

Recommended flow:

Player Seen
→ HandsUpMontage
→ AnimNotify_HandsUpFinished
→ MoveToPointA

Shot at Point A
→ HurtMontage
→ AnimNotify_HurtFinished
→ MoveToComputer

Reached Computer
→ ActivateComputerMontage
→ AnimNotify_ActivateFinished
→ Computer.ActivateComputer()
→ Mission Complete

Death
→ DeathMontage

==================================================
MOVEMENT / ANIMATION CONFLICTS
==================================================

Do not start AI movement while Hands Up or Hurt Reaction montages are still playing.

The intended rule is:

Animation finishes first
→ movement starts second

This avoids movement and montage conflicts.

==================================================
BLUEPRINT EVENTS / DELEGATES
==================================================

Expose BlueprintAssignable events/delegates for useful mission moments, such as:

- OnBorisSawPlayer
- OnHandsUpStarted
- OnReachedPointA
- OnBorisProvoked
- OnReachedComputer
- OnComputerActivationStarted
- OnBorisMissionCompleted
- OnBorisDiedBeforeMissionComplete

Do not implement dialogue UI directly in C++.

Do not implement Game Over UI directly in Boris.

Do not implement mission objective completion directly if the existing Mission System should own that.

==================================================
DEBUG LOGS
==================================================

Add useful UE_LOG messages:

BORIS: Player detected
BORIS: Hands up started
BORIS: Hands up finished
BORIS: Moving to Point A
BORIS: Reached Point A
BORIS: Waiting to be provoked
BORIS: Took damage at Point A
BORIS: Hurt reaction started
BORIS: Hurt reaction finished
BORIS: Moving to Computer
BORIS: Reached Computer
BORIS: Activation animation started
BORIS: Computer activated
BORIS: Mission complete
BORIS: Died before mission completion
BORIS: Died after mission completion

Avoid unnecessary log spam every frame.

==================================================
DO NOT MODIFY
==================================================

Do not modify unrelated:

- Bond weapon systems
- Bond health systems
- Time slow
- SWAT AI
- Photo system
- Copy system
- Other mission objectives

Keep Boris mission logic isolated and reusable.

==================================================
AFTER IMPLEMENTATION
==================================================

After finishing, tell me:

1. Which files were created.
2. Which existing files were modified.
3. What I need to assign in BP_Boris.
4. How to assign PointA.
5. How to assign ComputerTarget.
6. How the animation transitions work.
7. Which AnimNotifies I need to create.
8. How to connect mission completion.
9. How to connect early Boris death to Game Over.
10. How the Computer activation boolean can be read by the next mission.
```

---

# PART 2 – What You Should Do in Unreal Editor

Do this only after Codex finishes and the project compiles.

---

# Step 1 – Build the Project

After Codex finishes:

1. Review the changed files.
2. Build the project.
3. Fix compilation errors before continuing.

Expected:

```text
Build succeeded
```

Do not continue until Unreal recognizes the Boris changes.

---

# Step 2 – Create or Open BP_Boris

If Boris is a new C++ character:

```text
ABorisCharacter
      ↓
BP_Boris
```

If you already have a Boris Blueprint, reparent or update it only if necessary.

Open:

```text
BP_Boris
```

---

# Step 3 – Assign Boris Animations

Prepare these animation assets:

```text
Boris_Idle
Boris_Walk
Boris_Run
Boris_HandsUp
Boris_Hurt
Boris_ActivateComputer
Boris_Death
```

Recommended setup:

```text
AnimBP State Machine
├─ Idle
├─ Walk
└─ Run

Montages
├─ Hands Up
├─ Hurt
├─ Activate Computer
└─ Death
```

Assign the montage properties in `BP_Boris`:

```text
Hands Up Montage
Hurt Montage
Activate Computer Montage
Death Montage
```

---

# Step 4 – Create the Montages

Create:

```text
AM_Boris_HandsUp
AM_Boris_Hurt
AM_Boris_ActivateComputer
AM_Boris_Death
```

Make sure they use a montage slot that your Boris AnimBP supports.

For example:

```text
DefaultSlot
```

or your own Boris montage slot.

---

# Step 5 – Add AnimNotifies

Recommended notifies:

```text
AnimNotify_HandsUpFinished
AnimNotify_HurtFinished
AnimNotify_ActivateFinished
```

Place them near the end of the matching animations.

Flow:

```text
AM_Boris_HandsUp
       ↓
AnimNotify_HandsUpFinished
       ↓
Move to Point A
```

```text
AM_Boris_Hurt
       ↓
AnimNotify_HurtFinished
       ↓
Move to Computer
```

```text
AM_Boris_ActivateComputer
       ↓
AnimNotify_ActivateFinished
       ↓
Activate Computer
       ↓
Mission Complete
```

You do not need a DeathFinished notify unless your death cleanup logic specifically requires one.

---

# Step 6 – Test Hands Up Only

Before setting up the whole mission:

1. Place Boris in the level.
2. Start the game.
3. Let Boris detect Bond.

Expected:

```text
BORIS: Player detected
BORIS: Hands up started
```

Boris should:

```text
Idle
 ↓
See Bond
 ↓
Hands Up Animation
```

He should not start moving before the animation finishes.

---

# CHECKPOINT 1

Confirm:

```text
See Bond
→ Hands Up
→ No movement yet
```

If Boris moves during Hands Up, fix that before continuing.

---

# Step 7 – Create Point A

Place a Target Point in the level.

Name it something like:

```text
TP_Boris_PointA
```

Position it where Boris should stop and annoy the player.

Example:

```text
Boris Start
    ↓

[ corridor ]

    ↓

TP_Boris_PointA
```

---

# Step 8 – Assign Point A

Select:

```text
BP_Boris
```

In Details, assign:

```text
Point A → TP_Boris_PointA
```

---

# Step 9 – Test Hands Up → Point A

Run the game.

Expected:

```text
Boris sees Bond
↓
Hands Up animation
↓
Hands Up finished
↓
Boris walks to Point A
↓
Boris stops
```

Output Log should look similar to:

```text
BORIS: Player detected
BORIS: Hands up started
BORIS: Hands up finished
BORIS: Moving to Point A
BORIS: Reached Point A
BORIS: Waiting to be provoked
```

---

# CHECKPOINT 2

Confirm:

```text
Hands Up
→ Walk to Point A
→ Stop
```

---

# Step 10 – Add the Annoying Dialogue

Use:

```text
OnReachedPointA
```

in Blueprint.

Connect it to:

```text
Play Dialogue Audio
Show Subtitle
```

Example placeholder:

```text
"I'm not doing anything until you make me!"
```

The exact line can change later.

Important:

Boris should stay at Point A indefinitely after the dialogue.

Do not continue automatically.

---

# Step 11 – Test Shooting Boris

At Point A:

1. Shoot Boris once.
2. Make sure the shot is non-lethal.

Expected:

```text
WaitingAtPointA
      ↓
Shot
      ↓
Hurt Reaction
```

Output:

```text
BORIS: Took damage at Point A
BORIS: Hurt reaction started
```

Boris should not move while the Hurt animation is playing.

---

# CHECKPOINT 3

Confirm:

```text
Shoot Boris
→ Hurt Animation
→ No movement during Hurt
```

---

# Step 12 – Test Hurt → Computer Movement

When:

```text
AnimNotify_HurtFinished
```

fires:

Expected:

```text
BORIS: Hurt reaction finished
BORIS: Moving to Computer
```

Boris should then start walking toward the Computer.

---

# Step 13 – Prepare the Computer Actor

Use a reusable Computer actor.

Recommended class / Blueprint:

```text
AActivatableComputer
       ↓
BP_MainComputer
```

Required state:

```cpp
bool bActivated = false;
```

Functions:

```text
ActivateComputer()
IsActivated()
```

Initial value:

```text
bActivated = false
```

---

# Step 14 – Place the Computer

Place:

```text
BP_MainComputer
```

in the level.

Position it where Boris should perform the activation animation.

---

# Step 15 – Assign the Computer Target

Select:

```text
BP_Boris
```

Assign:

```text
Computer Target → BP_MainComputer
```

---

# Step 16 – Test Arrival at Computer

Run the sequence.

Expected:

```text
Shot Boris
↓
Hurt
↓
Move to Computer
↓
Reach Computer
↓
Stop
```

Output:

```text
BORIS: Reached Computer
```

---

# Step 17 – Test Activate Animation

When Boris reaches the Computer:

Expected:

```text
BORIS: Activation animation started
```

Boris should:

```text
Stop Movement
↓
Face Computer
↓
Play Activate Animation
```

The Computer should NOT activate immediately.

---

# CHECKPOINT 4

Confirm:

```text
Reach Computer
→ Activate Animation
→ bActivated still false during animation
```

---

# Step 18 – Activate the Computer at Animation Completion

When:

```text
AnimNotify_ActivateFinished
```

fires:

Expected:

```text
Computer.ActivateComputer()
```

Then:

```text
bActivated = true
```

Output:

```text
BORIS: Computer activated
BORIS: Mission complete
```

---

# Step 19 – Verify the Computer Boolean

Select or debug:

```text
BP_MainComputer
```

Before Boris activates it:

```text
bActivated = false
```

After activation:

```text
bActivated = true
```

This boolean will be used by the next mission.

For example:

```text
Can Download Data?
    ↓
Computer.IsActivated()
    ├─ false → No
    └─ true  → Yes
```

---

# Step 20 – Connect Mission Completion

Connect:

```text
OnBorisMissionCompleted
        ↓
Mission System
        ↓
Complete Objective
        ↓
Get personnel to activate computer
```

The Boris C++ class should not hardcode the objective name.

---

# Step 21 – Test Boris Death Before Completion

Restart the mission.

Kill Boris before he activates the Computer.

Expected:

```text
Health <= 0
↓
Death Montage
↓
OnBorisDiedBeforeMissionComplete
↓
Game Over
```

Output:

```text
BORIS: Died before mission completion
```

Connect the event to your Game Over flow.

---

# CHECKPOINT 5

Confirm:

```text
Boris dies early
→ Death animation
→ Game Over
```

---

# Step 22 – Test Boris Death After Completion

Complete the Boris mission.

Verify:

```text
bMissionCompleted = true
```

Then kill Boris.

Expected:

```text
Death Animation
```

But:

```text
NO GAME OVER
```

Output:

```text
BORIS: Died after mission completion
```

---

# FINAL MISSION TEST

Run this full sequence:

```text
Boris Idle
    ↓
Boris sees Bond
    ↓
Hands Up Animation
    ↓
Hands Up finishes
    ↓
Move to Point A
    ↓
Stop
    ↓
Annoying Dialogue
    ↓
Wait
    ↓
Bond shoots Boris
    ↓
Hurt Animation
    ↓
Hurt finishes
    ↓
Move to Computer
    ↓
Reach Computer
    ↓
Activate Animation
    ↓
Animation finishes
    ↓
Computer bActivated = true
    ↓
Mission Complete
```

Then test both death cases:

```text
Death before mission complete
→ Game Over
```

```text
Death after mission complete
→ Allowed
```

---

# Important Animation Rule

Do not start movement while a reaction montage is still playing.

Correct:

```text
Hands Up Animation
      ↓
Animation Finished
      ↓
Move
```

Correct:

```text
Hurt Animation
      ↓
Animation Finished
      ↓
Move
```

Wrong:

```text
Play Hurt Animation
      +
Start MoveTo immediately
```

This can cause locomotion and montage conflicts.

---

# Important Mission-Completion Rule

When Boris activates the Computer:

Do this order:

```text
Computer.ActivateComputer()
↓
bMissionCompleted = true
↓
CurrentState = Completed
↓
Broadcast OnBorisMissionCompleted
```

Do not do:

```text
Broadcast completion
↓
Later set bMissionCompleted
```

This avoids a case where Boris dies immediately after activation but the game still thinks the mission failed.

---

# What Codex Should Handle

Codex should handle:

- Boris mission-state enum
- State transitions
- Player detection hook
- Point A movement
- Computer movement
- Damage-state handling
- Non-lethal provoke transition
- Death handling
- Mission-completed boolean
- Animation montage references
- Montage / AnimNotify transition functions
- Computer activation function
- Computer activation boolean
- BlueprintAssignable delegates
- Debug logs
- Preventing duplicate transitions
- Preventing movement during reaction animations

---

# What You Should Handle in Unreal Editor

You should handle:

- Creating / opening `BP_Boris`
- Assigning Boris skeletal mesh
- Assigning AnimBP
- Creating the Hands Up montage
- Creating the Hurt montage
- Creating the Activate Computer montage
- Creating the Death montage
- Adding AnimNotifies
- Creating `TP_Boris_PointA`
- Assigning Point A
- Placing the Computer
- Assigning ComputerTarget
- Creating annoying dialogue
- Creating subtitle/audio presentation
- Connecting mission completion
- Connecting early Boris death to Game Over
- Testing animation timing
- Testing lethal vs non-lethal shots

---

# Recommended Checkpoints

## Checkpoint 1

```text
Boris sees Bond
→ Hands Up animation
```

## Checkpoint 2

```text
Hands Up finishes
→ Boris moves to Point A
```

## Checkpoint 3

```text
Boris reaches Point A
→ dialogue
→ waits
```

## Checkpoint 4

```text
Shoot Boris
→ Hurt animation
```

## Checkpoint 5

```text
Hurt finishes
→ Boris moves to Computer
```

## Checkpoint 6

```text
Reach Computer
→ Activate animation
```

## Checkpoint 7

```text
Activate finishes
→ Computer bActivated = true
```

## Checkpoint 8

```text
Mission completes
```

## Checkpoint 9

```text
Kill Boris before completion
→ Death animation
→ Game Over
```

## Checkpoint 10

```text
Kill Boris after completion
→ Death animation
→ No Game Over
```

---

# Debugging Advice

If Boris does not move to Point A:

- Check PointA is assigned.
- Check HandsUpFinished notify fires.
- Check AIController possesses Boris.
- Check NavMesh.
- Check MoveTo request result.

If Boris moves while Hands Up is playing:

- Make sure movement only starts after the HandsUpFinished notify.

If shooting Boris does nothing:

- Check Boris is actually in `WaitingAtPointA`.
- Check damage reaches Boris.
- Check the shot is non-lethal.
- Check the Hurt montage is assigned.

If Boris moves during Hurt:

- Make sure MoveToComputer starts only after HurtFinished.

If Computer never activates:

- Check ComputerTarget is assigned.
- Check Activate montage is assigned.
- Check ActivateFinished notify fires.
- Check ActivateComputer() is being called.

If killing Boris does not cause Game Over:

- Check `bMissionCompleted`.
- Check death event fires only once.
- Check `OnBorisDiedBeforeMissionComplete` is connected.

---

# Future Use

The same structure can later support more scripted NPC sequences.

For example:

```text
Detect Player
→ Reaction Animation
→ Move
→ Dialogue
→ Wait for Event
→ Reaction
→ Move
→ Interaction Animation
→ Mission Event
```

Keeping mission state, AI movement, animation timing, and mission events separate will make future NPC sequences much easier to expand.
