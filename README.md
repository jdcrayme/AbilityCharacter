# AbilityCharacter

Developed with Unreal Engine 5

## Use Ability (Interaction System)

A passive **use ability** that gives the character the ability to interact with **action volumes** in the world:

1. When granted, the ability starts a periodic **linear ray cast** ahead of the character (along the view direction).
2. If the ray finds an actor implementing the **`CombatUsable`** interface (for example a **`CombatActionVolume`**), a **use prompt widget** ("[E] Use") appears near the bottom of the screen.
3. Pressing the **use key** sends the `Combat.Input.UsePressed` gameplay event (forwarded by the character exactly like the attack inputs); the ability receives it and **calls the volume's `Use` function**.

Because everything runs through the GameplayAbility system, the feature stays data-driven: the ability is granted automatically, the prompt is a widget class you can restyle in a Blueprint, and any actor can become usable by implementing the interface.

### Files

| File | Purpose |
| --- | --- |
| `Source/AbilityCharacter/Variant_Combat/AbilitySystem/UseAbility.h/.cpp` | The passive ability: trace, prompt and key handling |
| `Source/AbilityCharacter/Variant_Combat/Interfaces/CombatUsable.h/.cpp` | Blueprintable interface for usable actors (`CanUse`, `GetUseText`, `Use`) |
| `Source/AbilityCharacter/Variant_Combat/Gameplay/CombatActionVolume.h/.cpp` | Invisible volume the interaction ray finds; fires `OnUsed` and activates linked `CombatActivatable` actors |
| `Source/AbilityCharacter/Variant_Combat/UI/CombatUsePromptWidget.h/.cpp` | Prompt widget with a C++-built `[Key] Action` layout (no assets needed) |

`CharacterBase` grants the ability automatically through the `DefaultUseAbility` property (set it to None on a character to disable interactions), and `CombatCharacter` forwards the use input as the `Combat.Input.UsePressed` gameplay event.

### Editor setup (once)

1. Create an Input Action `IA_Use` (bool value is fine).
2. Add it to `IMC_Default` and map it to the **E** key.
3. Open the player character Blueprint (e.g. `BP_ThirdPersonCharacter`) and assign `IA_Use` to the new **Use Action** property.

No other setup is required: the ability is granted and activated automatically for every combat character.

### Placing an action volume

Drag a **CombatActionVolume** into a level. Configure:

- **Use Text** - action shown in the prompt ("Open", "Talk", ...).
- **Actors To Activate** - every actor implementing the existing `CombatActivatable` interface is activated when the volume is used.
- **On Used** - Blueprint-assignable event fired on use.
- **Single Use** - the volume stops being usable after one use.

### Making custom usable actors

Either implement the `CombatUsable` interface in a Blueprint (Can Use / Get Use Text / Use are BlueprintNativeEvents) or derive from `CombatActionVolume` in C++. Custom usable actors need collision that blocks the ability's trace channel (Visibility by default); `CombatActionVolume` sets this up for you.

### Notes

- The trace and the prompt only run for locally controlled players; AI ability instances stay idle.
- The trace channel, distance, interval, prompt widget class and key label are `EditDefaultsOnly` properties on `UUseAbility`; tune them on a Blueprint subclass of the ability.
- Set `bDebugTrace` on the ability to visualize the interaction ray.
- The demo targets standalone/listen-server play. In dedicated-server games, interacting runs on the machine that owns the input; replicate your interactable's effects (or route them through gameplay events) as needed.
