# Fake Photo System – GoldenEye / Bond Objective

## Goal

Create a fake photography mechanic for the objective:

> **Photograph Main Video Screen**

The intended flow is:

1. Bond enters a photo trigger zone.
2. The game checks whether Bond is looking at the Main Video Screen.
3. If the target is visible and Bond is facing it, show:
   - **Press P to Take Photo**
4. Pressing `P` triggers:
   - Camera flash
   - Camera shutter sound
   - Photo objective completion
5. The photo cannot be taken twice.

---

# Recommended Implementation Order

Do not build everything at once.

Use these checkpoints:

1. PhotoZone detects Bond.
2. Looking at the target is detected.
3. Pressing `P` successfully takes the photo.
4. Prompt, flash, and sound work.
5. Mission objective completes.

If one checkpoint fails, fix it before moving to the next one.

---

# Step 1 – Ask Codex to Create the C++ Foundation

Give Codex this prompt:

```text
Implement a reusable fake photography interaction system for my Unreal Engine GoldenEye project.

Existing classes:
- AJamesBondCharacter
- ABondPlayerController

Create a new C++ actor called APhotoOpportunity.

Requirements:
- Add a UBoxComponent called TriggerBox.
- Add an editable AActor* PhotoTarget.
- Detect when AJamesBondCharacter enters and leaves the trigger.
- While Bond is inside, determine whether Bond is looking at PhotoTarget.
- Use the player's camera location and forward direction.
- Use a configurable dot-product threshold, default 0.85.
- Perform a line trace from the camera toward PhotoTarget to verify the target is visible.
- Ignore Bond himself in the trace.

A photo is valid only if:
1. Bond is inside the trigger.
2. Bond is facing the target.
3. The target is visible.
4. The photo has not already been taken.

Add:
- TryTakePhoto()

Add BlueprintAssignable delegates:
- OnPhotoAvailable
- OnPhotoUnavailable
- OnPhotoTaken

Do NOT:
- Implement UI.
- Implement screenshots.
- Implement mission completion.
- Bind keyboard input directly inside APhotoOpportunity.

Modify AJamesBondCharacter to support:
- APhotoOpportunity* CurrentPhotoOpportunity
- SetPhotoOpportunity()
- ClearPhotoOpportunity()
- TakePhoto()

TakePhoto() should call TryTakePhoto() on the current opportunity.

Add useful UE_LOG messages for:
- Enter photo zone
- Leave photo zone
- Photo available
- Photo unavailable
- Photo attempt failed
- Photo taken successfully

Do not modify unrelated:
- Weapon systems
- Health systems
- AI
- Time slow
- Mission systems

After finishing, tell me exactly which files were created or changed.
```

---

# Step 2 – Build the Project

After Codex finishes:

1. Close Unreal if necessary.
2. Build the project.
3. Make sure compilation succeeds.

Expected result:

```text
Build succeeded
```

Do not continue until Unreal recognizes the new C++ class.

---

# Step 3 – Find the New C++ Class

Open Unreal.

Look inside:

```text
C++ Classes
    YourProject
        PhotoOpportunity
```

Confirm that `PhotoOpportunity` exists.

---

# Step 4 – Create BP_PhotoZone

Right-click:

```text
PhotoOpportunity
```

Choose:

```text
Create Blueprint class based on PhotoOpportunity
```

Name it:

```text
BP_PhotoZone
```

Open the Blueprint.

You should see the inherited:

```text
TriggerBox
```

component.

---

# Step 5 – Place BP_PhotoZone in the Level

Drag:

```text
BP_PhotoZone
```

into the Bunker level.

Place it near the Main Video Screen where Bond should stand to take the photograph.

Example:

```text
        MAIN VIDEO SCREEN
       ┌───────────────┐
       │               │
       │    SCREEN     │
       │               │
       └───────────────┘


            Bond
             ↓

       ┌─────────────┐
       │ PHOTO ZONE  │
       └─────────────┘
```

Resize the trigger box so the player has a reasonable area to stand in.

Do not make it too small.

---

# Step 6 – Assign the Main Video Screen

Select the placed:

```text
BP_PhotoZone
```

In the Details panel, find:

```text
Photo Target
```

Use the eyedropper and select the Main Video Screen actor.

Now the PhotoZone knows which actor Bond must photograph.

---

# Step 7 – Test Detection Only

Do not create UI yet.

Play the game.

Walk Bond into the PhotoZone.

Check the Output Log.

Expected:

```text
Entered Photo Zone
```

Look toward the Main Video Screen.

Expected:

```text
Photo Available
```

Turn away.

Expected:

```text
Photo Unavailable
```

Leave the zone.

Expected:

```text
Left Photo Zone
```

Desired behavior:

```text
Outside zone
❌

Inside + looking away
❌

Inside + looking at screen
✅
```

---

# Step 8 – Tune the Facing Threshold

The dot-product threshold controls how directly Bond must look at the target.

Example values:

```text
0.90 = strict
0.85 = reasonable
0.75 = generous
```

Recommended starting value:

```text
0.80–0.85
```

For a GoldenEye-style interaction, slightly generous is usually better.

---

# Step 9 – Create the Take Photo Input

Create a new Enhanced Input Action:

```text
IA_TakePhoto
```

Set it to:

```text
Digital / Boolean
```

Open Bond's Input Mapping Context.

For example:

```text
IMC_Bond
```

Add:

```text
IA_TakePhoto → P
```

Save.

---

# Step 10 – Ask Codex to Bind IA_TakePhoto

Give Codex this prompt:

```text
The C++ PhotoOpportunity system is working.

I created an Enhanced Input Action called IA_TakePhoto.

Please bind the Take Photo input in AJamesBondCharacter using the same Enhanced Input architecture already used by this character.

Pressing the input should call the existing TakePhoto() function.

Expose the input action as an editable UInputAction* property if appropriate, following the same pattern as the existing Bond inputs.

Do not modify any other input behavior.
```

Build the project again.

---

# Step 11 – Assign IA_TakePhoto to Bond

Open:

```text
BP_JamesBond
```

Find the new input property, likely something similar to:

```text
Take Photo Action
```

Assign:

```text
IA_TakePhoto
```

Test again.

Stand in the PhotoZone.

Look at the Main Video Screen.

Press:

```text
P
```

Expected Output Log:

```text
PHOTO TAKEN
```

At this point the core gameplay mechanic works.

---

# CHECKPOINT 3

Confirm:

- Bond must be inside the zone.
- Bond must be looking at the screen.
- Bond must have line of sight.
- Pressing `P` succeeds.
- Pressing `P` outside the conditions fails.

Do not continue until this works.

---

# Step 12 – Create the Interaction Prompt Widget

Create:

```text
WBP_InteractionPrompt
```

Do not call it `WBP_PhotoPrompt`.

Keep it reusable for other interactions.

Example:

```text
┌──────────────────────────┐
│   [ P ]  Take Photo      │
└──────────────────────────┘
```

Use a TextBlock.

Later the same widget can support:

```text
[P] Take Photo
[E] Use Computer
[E] Copy Key
```

---

# Step 13 – Add the Prompt Through the HUD / PlayerController

Recommended architecture:

```text
PhotoZone
    ↓
Interaction available
    ↓
PlayerController / HUD
    ↓
Show prompt
```

Avoid this architecture:

```text
PhotoZone
    ↓
Creates its own random UI
```

The HUD / PlayerController should own UI presentation.

---

# Step 14 – Connect OnPhotoAvailable

For the first version, Blueprint wiring is fine.

Open:

```text
BP_PhotoZone
```

Use:

```text
OnPhotoAvailable
        ↓
Show Interaction Prompt
        ↓
"Press P to Take Photo"
```

Then:

```text
OnPhotoUnavailable
        ↓
Hide Interaction Prompt
```

Test:

```text
Enter zone while looking away
→ no prompt

Look at screen
→ Press P to Take Photo

Look away
→ prompt disappears
```

---

# Step 15 – Create the Flash Widget

Create:

```text
WBP_PhotoFlash
```

Add a fullscreen white Image.

Set anchors to:

```text
Full Screen
```

Create a widget animation called:

```text
Flash
```

Suggested opacity animation:

```text
0.00 sec → 0 opacity
0.03 sec → 1 opacity
0.20 sec → 0 opacity
```

The effect should feel like a quick camera flash.

---

# Step 16 – Connect OnPhotoTaken

Inside:

```text
BP_PhotoZone
```

Connect:

```text
OnPhotoTaken
       ↓
Hide interaction prompt
       ↓
Play photo flash
```

Test:

```text
Enter zone
→ Look at screen
→ Prompt appears
→ Press P
→ FLASH
```

---

# Step 17 – Add Camera Shutter Sound

Import or choose a short camera shutter sound.

Connect:

```text
OnPhotoTaken
     ├── Hide Prompt
     ├── Play Flash
     └── Play Sound 2D
```

Now the fake photography mechanic should feel convincing even though no real screenshot is being created.

---

# Step 18 – Verify the Photo Cannot Be Taken Twice

The C++ class should contain something similar to:

```cpp
bool bPhotoTaken;
```

After success:

```cpp
bPhotoTaken = true;
```

Test:

1. Take the photo.
2. Press `P` again.
3. Leave the zone.
4. Enter again.
5. Look at the target.

Expected:

- No second photo.
- No interaction prompt.
- No repeated objective completion.

---

# Step 19 – Connect to the Mission System

Only do this after the photography mechanic itself works.

Desired flow:

```text
OnPhotoTaken
    ├─ Flash
    ├─ Shutter
    ├─ Hide Prompt
    │
    └─ MissionSystem
           ↓
      CompleteObjective
           ↓
Photograph Main Video Screen ✓
```

The Photo system should only report:

```text
The photo was successfully taken.
```

The Mission System should decide which objective that completes.

Do not tightly couple the Photo system to a specific mission.

---

# Final Architecture

```text
AJamesBondCharacter
        │
        │ P Input
        ▼
CurrentPhotoOpportunity
        │
        ▼
APhotoOpportunity
    ├─ TriggerBox
    ├─ PhotoTarget
    ├─ Facing Check
    ├─ Line-of-Sight Check
    └─ TryTakePhoto()
        │
        ▼
OnPhotoTaken
    ├───────────────┐
    ▼               ▼
Photo UI        Mission System
    │               │
    ├─ Flash         └─ Complete Objective
    └─ Shutter
```

---

# Debugging Checklist

## Checkpoint 1 – Trigger

```text
Bond enters PhotoZone
→ "Entered Photo Zone"
```

If this fails:

- Check Box Collision.
- Check collision presets.
- Make sure Bond generates overlap events.

---

## Checkpoint 2 – Target Detection

```text
Bond looks at Main Video Screen
→ "Photo Available"
```

If this fails:

- Confirm `PhotoTarget` is assigned.
- Check facing threshold.
- Check camera forward vector.
- Check line trace.
- Check whether another object blocks the trace.

Do not work on UI until this works.

---

## Checkpoint 3 – Input

```text
Bond presses P
→ "PHOTO TAKEN"
```

If this fails:

- Check `IA_TakePhoto`.
- Check `IMC_Bond`.
- Check input binding.
- Check `TakePhoto()`.
- Check `CurrentPhotoOpportunity`.

Do not work on mission completion until this works.

---

## Checkpoint 4 – Presentation

```text
Photo Available
→ Prompt

P
→ Flash
→ Shutter
```

If gameplay works but the UI does not, the problem is isolated to UI.

---

## Checkpoint 5 – Mission Completion

```text
OnPhotoTaken
→ CompleteObjective("PhotographMainVideoScreen")
```

Only integrate this after everything above is stable.

---

# Things Codex Should Handle

Codex should handle:

- `APhotoOpportunity`
- Box trigger logic
- Target reference
- Facing check
- Dot-product logic
- Line trace
- `TryTakePhoto()`
- Delegates
- Bond's current PhotoOpportunity reference
- Take Photo input binding
- Debug logs

---

# Things You Should Handle in Unreal Editor

You should handle:

- Creating `BP_PhotoZone`
- Placing it in the level
- Resizing the trigger box
- Assigning Main Video Screen as `PhotoTarget`
- Creating `IA_TakePhoto`
- Assigning the input action in `BP_JamesBond`
- Creating `WBP_InteractionPrompt`
- Creating `WBP_PhotoFlash`
- Creating the flash animation
- Choosing/importing shutter sound
- Blueprint event wiring
- Connecting `OnPhotoTaken` to the Mission System

---

# Important Rule

Do not ask Codex to build the whole feature in one giant pass.

Build and test in this order:

```text
Trigger
  ↓
Visibility
  ↓
P Input
  ↓
Photo Success
  ↓
Prompt
  ↓
Flash + Sound
  ↓
Mission Completion
```

This makes bugs much easier to identify and fix.
