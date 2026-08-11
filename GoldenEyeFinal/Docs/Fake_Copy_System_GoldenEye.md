# Fake Copy System – GoldenEye Key Objective

## Goal

Create a fake copy/download interaction for the objective:

> **Copy GoldenEye Key and Leave Original**

The intended flow is:

1. Bond enters a trigger zone.
2. The game shows:
   - **Press P to Copy GoldenEye Key**
3. Bond presses `P`.
4. A progress bar fills from `0%` to `100%`.
5. If Bond leaves the trigger before completion, the copy is cancelled and resets.
6. When progress reaches `100%`:
   - Show **Download Complete**
   - Complete the objective:
     - **Copy GoldenEye Key and Leave Original**
7. The interaction cannot be completed twice.

---

# Recommended Architecture

Use a reusable C++ actor:

```text
ACopyOpportunity
        ↓
BP_CopyGoldenEyeKey
```

The C++ actor should handle:

- Trigger detection
- Copy state
- Copy duration
- Progress calculation
- Cancellation
- Completion events

The HUD / Blueprint side should handle:

- Interaction prompt
- Progress bar
- "Download Complete" text
- Sounds
- Mission objective completion

Recommended flow:

```text
Bond enters Copy Zone
        ↓
OnCopyAvailable
        ↓
Show "Press P to Copy GoldenEye Key"
        ↓
Press P
        ↓
TryStartCopy()
        ↓
Copying 0% → 100%
        ↓
OnCopyProgressChanged
        ↓
Update Progress Bar
        ↓
100%
        ↓
OnCopyCompleted
        ↓
"Download Complete"
        ↓
Complete Mission Objective
```

---

# PART 1 – What to Give Codex

Copy and paste the following prompt into Codex.

```text
Implement a reusable timed copy/download interaction system for my Unreal Engine GoldenEye project.

Existing architecture:
- Player character is AJamesBondCharacter.
- Player controller is ABondPlayerController.
- The project already has a fake photo interaction system using opportunity actors and player input.
- All mission objectives are active simultaneously.
- Mission completion is event-driven and there is no required objective order.
- Do not redesign the existing mission system.

Create a new reusable C++ actor called ACopyOpportunity.

==================================================
CORE COMPONENTS
==================================================

Add:

- UBoxComponent* TriggerBox
- float CopyDuration
- float CurrentCopyProgress
- bool bPlayerInside
- bool bCopyInProgress
- bool bCopyCompleted

CopyDuration should be editable in Blueprint / Unreal Editor.

Default:

CopyDuration = 3.0f

CurrentCopyProgress should represent:

0.0 = 0%
1.0 = 100%

==================================================
TRIGGER BEHAVIOR
==================================================

Detect when AJamesBondCharacter enters and leaves TriggerBox.

When Bond enters:

- Set bPlayerInside = true.
- Make this CopyOpportunity available to Bond.
- Broadcast OnCopyAvailable if the copy has not already completed.

When Bond leaves:

- Set bPlayerInside = false.
- Clear this CopyOpportunity from Bond.
- Broadcast OnCopyUnavailable.

IMPORTANT:

If Bond leaves the trigger while copying:

- Cancel the current copy.
- Stop the copy timer/progress.
- Reset CurrentCopyProgress to 0.0.
- Set bCopyInProgress = false.
- Broadcast an appropriate cancellation/reset event if useful.
- The player must re-enter and start the copy again.

Do not mark the objective completed when cancelled.

==================================================
START COPY
==================================================

Add:

TryStartCopy()

The copy should only start if:

- Bond is inside TriggerBox.
- Copy is not already running.
- Copy has not already completed.

If any condition fails:

- Reject the request.
- Add a useful UE_LOG explaining why.

If successful:

- Set bCopyInProgress = true.
- Reset CurrentCopyProgress to 0.0.
- Broadcast OnCopyStarted.
- Begin progressing toward completion.

==================================================
PROGRESS
==================================================

Progress should increase smoothly from:

0.0 → 1.0

over CopyDuration seconds.

Use Unreal-friendly timer/tick logic.

Avoid unnecessary per-frame work when the copy is inactive.

Broadcast:

OnCopyProgressChanged(float Progress)

as progress changes.

The UI will use this value directly for a UMG Progress Bar.

Clamp progress between:

0.0 and 1.0

==================================================
COMPLETION
==================================================

When progress reaches 1.0:

- Set CurrentCopyProgress = 1.0.
- Set bCopyInProgress = false.
- Set bCopyCompleted = true.
- Broadcast one final OnCopyProgressChanged(1.0).
- Broadcast OnCopyCompleted.

The copy must not complete more than once.

After completion:

- Pressing the interaction key again should do nothing.
- Leaving and re-entering the trigger should not restart the copy.
- OnCopyAvailable should not be broadcast again for a completed CopyOpportunity.

==================================================
BLUEPRINT EVENTS / DELEGATES
==================================================

Expose BlueprintAssignable delegates for:

- OnCopyAvailable
- OnCopyUnavailable
- OnCopyStarted
- OnCopyProgressChanged(float Progress)
- OnCopyCancelled
- OnCopyCompleted

Do not create any UI directly in ACopyOpportunity.

==================================================
PLAYER INPUT INTEGRATION
==================================================

Integrate this with AJamesBondCharacter.

The project already has a photo interaction that uses the P key / TakePhoto interaction.

Review the existing interaction architecture before changing anything.

If the existing P interaction can be cleanly generalized and reused, prefer that instead of creating duplicate input handling.

The goal is:

Bond presses P
    ↓
If there is an active CopyOpportunity
    ↓
Call TryStartCopy()

Add whatever clean reference/function is necessary, for example:

ACopyOpportunity* CurrentCopyOpportunity

SetCopyOpportunity(ACopyOpportunity*)
ClearCopyOpportunity(ACopyOpportunity*)
TryCopyInteraction()

However, first inspect the existing photo interaction implementation and avoid creating conflicting P-key bindings.

Do not put input handling directly inside ACopyOpportunity.

==================================================
DEBUG LOGS
==================================================

Add useful UE_LOG messages for:

- Bond entered copy zone
- Bond left copy zone
- Copy became available
- Copy started
- Copy progress
- Copy cancelled
- Copy completed
- Copy attempt rejected because Bond is outside
- Copy attempt rejected because already copying
- Copy attempt rejected because already completed

Do not spam the log every frame unnecessarily.

For progress logging, log useful milestones or otherwise keep logging reasonable.

==================================================
DO NOT IMPLEMENT
==================================================

Do NOT:

- Implement mission completion directly.
- Hardcode "Copy GoldenEye Key and Leave Original" into the C++ actor.
- Create UMG widgets.
- Create a progress bar.
- Create sounds.
- Modify weapon systems.
- Modify health systems.
- Modify AI.
- Modify time slow.
- Modify unrelated mission logic.
- Create an actual file-copying or download system.

This is intentionally a fake gameplay interaction.

==================================================
REUSABILITY
==================================================

ACopyOpportunity should be reusable.

Later I may use it for:

- Downloading computer data
- Hacking terminals
- Copying files
- Uploading data
- Planting software
- Other timed spy interactions

Do not make the class specific to the GoldenEye key objective.

==================================================
AFTER IMPLEMENTATION
==================================================

After finishing, tell me:

1. Which files were created.
2. Which existing files were modified.
3. How ACopyOpportunity works.
4. How it integrates with AJamesBondCharacter.
5. Whether you reused or generalized the existing P interaction.
6. Exactly what I need to configure in Unreal Editor.
7. Any Blueprint properties I need to assign.
```

---

# PART 2 – What You Should Do After Codex Finishes

Do not set up the Blueprint before the C++ class exists.

Recommended order:

```text
Codex
  ↓
Compile
  ↓
Create BP_CopyGoldenEyeKey
  ↓
Test Trigger
  ↓
Test P Input
  ↓
Create UI
  ↓
Connect Progress
  ↓
Connect Objective
```

---

# Step 1 – Build the Project

After Codex finishes:

1. Review the changed files.
2. Build the Unreal project.
3. Fix any compile errors before continuing.

Expected:

```text
Build succeeded
```

Do not continue until Unreal recognizes:

```text
ACopyOpportunity
```

---

# Step 2 – Find CopyOpportunity in Unreal

Open Unreal Engine.

Look under your project's C++ classes.

You should see something similar to:

```text
C++ Classes
    YourProject
        CopyOpportunity
```

---

# Step 3 – Create BP_CopyGoldenEyeKey

Right-click:

```text
CopyOpportunity
```

Choose:

```text
Create Blueprint class based on CopyOpportunity
```

Name it:

```text
BP_CopyGoldenEyeKey
```

Open it.

You should see an inherited:

```text
TriggerBox
```

component.

---

# Step 4 – Place the Copy Zone

Drag:

```text
BP_CopyGoldenEyeKey
```

into the Bunker level.

Place it where Bond should stand when copying the GoldenEye key.

Example:

```text
      GOLDENEYE KEY / DEVICE
        ┌───────────────┐
        │               │
        │      KEY      │
        │               │
        └───────────────┘


             Bond
              ↓

        ┌─────────────┐
        │  COPY ZONE  │
        └─────────────┘
```

Resize the TriggerBox so the player has a reasonable interaction area.

Do not make it extremely small.

---

# Step 5 – Configure Copy Duration

Select:

```text
BP_CopyGoldenEyeKey
```

In the Details panel, find:

```text
Copy Duration
```

Start with:

```text
3.0
```

Suggested tuning:

```text
2.0 sec = fast
3.0 sec = recommended
5.0 sec = more deliberate
```

For the first implementation, use:

```text
3.0 seconds
```

---

# Step 6 – Test Trigger Detection Before UI

Do not create the progress bar yet.

Play the level.

Walk Bond into the Copy Zone.

Check the Output Log.

Expected:

```text
Bond entered copy zone
Copy available
```

Leave the zone.

Expected:

```text
Bond left copy zone
Copy unavailable
```

If this does not work, fix the trigger/collision before continuing.

---

# CHECKPOINT 1 – Trigger

Desired behavior:

```text
Outside Copy Zone
❌ Copy unavailable

Inside Copy Zone
✅ Copy available
```

---

# Step 7 – Test the P Interaction

Walk into:

```text
BP_CopyGoldenEyeKey
```

Press:

```text
P
```

Expected:

```text
Copy started
```

Progress should begin internally.

Expected completion:

```text
Copy completed
```

Do not add UI until this works.

---

# Step 8 – Test Cancellation

Enter the trigger.

Press:

```text
P
```

Let the copy run partially.

For example:

```text
Copy progress: ~50%
```

Then walk outside the trigger.

Expected:

```text
Copy cancelled
Progress reset to 0
```

Walk back inside.

Press:

```text
P
```

The copy should start again from:

```text
0%
```

---

# CHECKPOINT 2 – Core Gameplay

Confirm all of these:

- Bond can only start copying inside the trigger.
- Pressing P starts the copy.
- Progress increases from 0 to 100.
- Leaving the trigger cancels the copy.
- Cancelled progress resets to 0.
- Re-entering allows a new attempt.
- Completion only happens once.

Do not continue until these work.

---

# Step 9 – Create the Copy UI

You may reuse your existing interaction UI if it is flexible enough.

Recommended UI:

```text
WBP_InteractionPrompt
```

for:

```text
[P] Copy GoldenEye Key
```

Then create a separate reusable progress widget:

```text
WBP_InteractionProgress
```

Suggested layout:

```text
Copying GoldenEye Key...

[████████████░░░░░░░░] 60%
```

UMG elements:

- TextBlock for action text
- ProgressBar
- Optional TextBlock for percentage
- Optional completion text

Keep this reusable.

Do not call it:

```text
WBP_GoldenEyeKeyProgress
```

Prefer:

```text
WBP_InteractionProgress
```

because later you can reuse it for:

- Downloading data
- Hacking
- Uploading
- Copying files

---

# Step 10 – Wire OnCopyAvailable

Open:

```text
BP_CopyGoldenEyeKey
```

Connect:

```text
OnCopyAvailable
        ↓
Show Interaction Prompt
        ↓
[P] Copy GoldenEye Key
```

Connect:

```text
OnCopyUnavailable
        ↓
Hide Interaction Prompt
```

Expected behavior:

```text
Enter trigger
→ [P] Copy GoldenEye Key

Leave trigger
→ Prompt disappears
```

---

# Step 11 – Wire OnCopyStarted

Connect:

```text
OnCopyStarted
        ↓
Hide normal interaction prompt
        ↓
Show WBP_InteractionProgress
        ↓
Set text:
"Copying GoldenEye Key..."
```

At the start:

```text
Progress = 0%
```

---

# Step 12 – Wire OnCopyProgressChanged

Use:

```text
OnCopyProgressChanged
```

The event should provide:

```text
Progress
```

from:

```text
0.0 → 1.0
```

Connect the value directly to the UMG ProgressBar percentage.

Conceptually:

```text
OnCopyProgressChanged(Progress)
        ↓
ProgressBar.SetPercent(Progress)
```

If you also display percentage text:

```text
Progress * 100
        ↓
Round
        ↓
Convert to Text
        ↓
"63%"
```

Example:

```text
Copying GoldenEye Key...

[█████████████░░░░░░░] 63%
```

---

# Step 13 – Wire OnCopyCancelled

Connect:

```text
OnCopyCancelled
        ↓
Hide Progress UI
        ↓
Reset Progress Bar to 0
```

If Bond is now outside the trigger:

```text
Do not show the interaction prompt
```

When Bond enters again:

```text
OnCopyAvailable
        ↓
[P] Copy GoldenEye Key
```

---

# Step 14 – Wire OnCopyCompleted

Connect:

```text
OnCopyCompleted
        ↓
Set Progress Bar = 100%
        ↓
Show "Download Complete"
```

Recommended presentation:

```text
Copying GoldenEye Key...

[████████████████████] 100%

DOWNLOAD COMPLETE
```

You can leave the completion message visible briefly, then hide it.

For example:

```text
OnCopyCompleted
        ↓
Show "Download Complete"
        ↓
Delay 1.0 sec
        ↓
Hide Progress Widget
```

This delay is presentation only.

Mission completion should already be triggered by the completion event.

---

# Step 15 – Add Sound Effects

Optional but recommended.

Possible sounds:

### Start

```text
Electronic beep
```

### During Copy

Optional subtle data-processing sound.

### Complete

```text
Confirmation beep
```

Blueprint example:

```text
OnCopyStarted
    └─ Play Sound 2D

OnCopyCompleted
    └─ Play completion sound
```

Do not make the sound system part of `ACopyOpportunity`.

---

# Step 16 – Connect the Mission Objective

Only do this after the copy mechanic and UI work correctly.

Connect:

```text
OnCopyCompleted
        ↓
MissionSystem
        ↓
CompleteObjective
        ↓
Copy GoldenEye Key and Leave Original
```

Conceptually:

```text
OnCopyCompleted
    ├─ Progress = 100%
    ├─ Download Complete
    ├─ Completion Sound
    │
    └─ Mission System
            ↓
       Complete Objective
            ↓
Copy GoldenEye Key and Leave Original ✓
```

---

# Important Mission-System Rule

Do not make `ACopyOpportunity` know which mission it belongs to.

Bad:

```cpp
CompleteGoldenEyeKeyMission();
```

Better:

```text
OnCopyCompleted
        ↓
Blueprint / Mission System
        ↓
Complete appropriate objective
```

This keeps the system reusable.

---

# Step 17 – Test Completion Cannot Repeat

Complete the copy once.

Then test:

1. Press `P` again.
2. Leave the trigger.
3. Re-enter.
4. Press `P`.

Expected:

```text
Nothing happens.
```

The prompt should not return.

The progress bar should not restart.

The mission objective should not complete twice.

---

# FINAL TEST FLOW

Run this complete test:

```text
Bond approaches GoldenEye Key
        ↓
Outside Trigger
        ↓
No Prompt

        ↓

Bond enters Trigger
        ↓
[P] Copy GoldenEye Key

        ↓

Bond presses P
        ↓
Copying GoldenEye Key...
0% → 30% → 60%

        ↓

Bond walks away
        ↓
COPY CANCELLED
Progress resets

        ↓

Bond comes back
        ↓
[P] Copy GoldenEye Key

        ↓

Press P
        ↓
0% → 100%

        ↓

DOWNLOAD COMPLETE

        ↓

Mission Objective:
Copy GoldenEye Key and Leave Original ✓
```

---

# Recommended State Flow

The interaction can be thought of as:

```text
Unavailable
     ↓
Bond enters zone
     ↓
Available
     ↓
P
     ↓
Copying
     │
     ├──── Bond leaves ───→ Cancelled
     │                         ↓
     │                       Reset
     │
     └──── 100% ───────────→ Completed
```

After:

```text
Completed
```

the actor should remain completed permanently for that play session.

---

# What Codex Should Handle

Codex should handle:

- `ACopyOpportunity`
- Trigger Box
- Enter/leave detection
- Copy duration
- Copy state
- Copy progress
- Copy cancellation
- Progress reset
- Copy completion
- BlueprintAssignable delegates
- Bond reference to active CopyOpportunity
- P-input integration
- Avoiding conflicts with the existing photo interaction
- Debug logs
- Preventing duplicate completion

---

# What You Should Handle

You should handle:

- Creating `BP_CopyGoldenEyeKey`
- Placing it in the level
- Resizing TriggerBox
- Setting CopyDuration
- Testing overlaps
- Creating/reusing interaction prompt UI
- Creating `WBP_InteractionProgress`
- Creating the UMG ProgressBar
- Wiring progress events
- Displaying percentage
- Showing "Download Complete"
- Adding sounds
- Connecting `OnCopyCompleted` to the mission system

---

# Recommended Checkpoints

Do not skip these.

## Checkpoint 1

```text
Enter / leave trigger works
```

## Checkpoint 2

```text
P starts copy
```

## Checkpoint 3

```text
Progress reaches 100%
```

## Checkpoint 4

```text
Leaving cancels and resets
```

## Checkpoint 5

```text
Progress UI works
```

## Checkpoint 6

```text
Download Complete appears
```

## Checkpoint 7

```text
Mission objective completes
```

If an earlier checkpoint fails, do not continue to later systems.

---

# Future Reuse

Once this works, the same system can support other Bunker objectives.

For example:

```text
BP_CopyGoldenEyeKey
        ↓
ACopyOpportunity
```

and:

```text
BP_DownloadComputerData
        ↓
ACopyOpportunity
```

The only differences could be:

```text
Interaction Text
Copy Duration
Completion Event
```

This means you should not need a completely new C++ system when you implement:

> Download data from computer

later.
