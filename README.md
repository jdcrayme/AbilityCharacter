# AbilityCharacter

Developed with Unreal Engine 5

## Project structure

The source module is organized so every gameplay feature is a self-contained
slice that can later move into the planned modular character plugin, and the
shared base sits in `Core/`:

```
Source/AbilityCharacter/
├── Core/            shared base: CharacterBase, base PlayerController, GAS
│                    plumbing (attributes, ASC, tags, effect context),
│                    shared interfaces and widget glue
├── Features/
│   ├── Combat/      melee/charged/combo/ranged abilities, stances, AI,
│   │                animation notifies, combat actors and life bar
│   ├── Interaction/ the use ability described below: usable interface,
│   │                action volumes, prompt UI
│   ├── Platforming/ double/wall jump, coyote time, dash
│   └── SideScrolling/
└── Template/        the original third person demo classes (kept as
                     redirect targets for the template Blueprints)
```

See `ARCHITECTURE.md` for the full map from this layout to the future plugin,
and `CLEANUP.md` for the change log of the cleanup pass.

## Use Ability (Interaction System)

A passive **use ability** that lets a character interact with anything **usable** in the world:

1. When granted, the ability periodically **line traces** ahead of the character along the view direction.
2. If the ray hits a usable target — an actor implementing the **`CombatUsable`** interface, or an actor hosting a component that implements it (e.g. a **`CombatActionVolumeComponent`** on a vehicle) — a **world-space use prompt** appears along the ray, by default midway between the ray source and the intersection point.
3. Pressing the **use key** sends the `Event.Input.UsePressed` gameplay event; the ability receives it and **calls the target's `Use` function**.

### Design: abilities without character dependencies

The character classes contain **no use-ability code at all**. Everything is data-driven:

| Concern | Mechanism | Where it's configured |
| --- | --- | --- |
| Granting | `CharacterBase::DefaultAbilities` (the same list every other ability uses) | Blueprint of the character |
| Input | `CharacterBase::EventInputMappings` — a generic Input Action → gameplay event table; each row picks its own trigger event (Started / Completed / Triggered), so hold-style abilities receive press and release; abilities listen for their event tag (`WaitGameplayEvent`) | Blueprint of the character |
| Prompt look | `PromptWidgetClass` on the ability — any widget implementing `CombatUsePromptInterface` | Blueprint of the ability |

Adding the use ability to a character (or a future grapple/inspect/vehicle ability) requires **zero C++ changes**: create a Blueprint subclass, add it to `DefaultAbilities`, and add an entry to `EventInputMappings`. Removing the entries removes the feature cleanly. `CharacterBase` only knows about abilities through `UGameplayAbility`, `FGameplayTag`, and its generic mapping tables.

### Files

| File | Purpose |
| --- | --- |
| `Features/Interaction/AbilitySystem/UseAbility.h/.cpp` | The passive ability: periodic trace, target resolution, world-space prompt, key handling |
| `Features/Interaction/Interfaces/CombatUsable.h/.cpp` | Blueprintable interface for usable actors and components (`CanUse`, `GetUseText`, `Use`) |
| `Core/Interfaces/CombatInputLabelProvider.h/.cpp` | Interface for pawns that can name the key bound to an input event tag (`GetKeyLabelForEvent`); `CharacterBase` answers from its `EventInputMappings`, so prompts always show the character's configured key |
| `Features/Interaction/Gameplay/CombatActionVolumeComponent.h/.cpp` | **Actor component** that makes any actor usable: UseText, `OnUsed` event, linked `CombatActivatable` actors, optional auto-created trace box |
| `Features/Interaction/Gameplay/CombatActionVolume.h/.cpp` | Standalone, level-placeable action volume (a box + the component above) |
| `Features/Interaction/UI/CombatUsePromptActor.h/.cpp` | World-space prompt holder: a `WidgetComponent` actor the ability spawns, moves and hides; smoothly follows its target between traces |
| `Features/Interaction/UI/CombatUsePromptInterface.h/.cpp` | Widget interface: `UpdatePromptText(KeyText, ActionText)` — how the ability feeds the prompt without knowing the widget class |
| `Features/Interaction/UI/CombatUsePromptWidget.h/.cpp` | Default prompt widget with a C++-built `[E] Open` layout (works with no assets); Blueprint subclasses can design their own layout and use `BP_OnPromptTextChanged` |

### Editor setup (once per project)

1. **Input**: `IA_Use` already exists and is bound to **E** in `IMC_Default` (any Input Action works — the ability only cares about the event tag).
2. **Ability Blueprint**: create `GA_Use` with `UseAbility` as parent (right-click in the content browser → Blueprints → Class → search "UseAbility"). Configure on the ability defaults:
   - `Prompt Widget Class` — defaults to the built-in `CombatUsePromptWidget`; assign any widget implementing `CombatUsePromptInterface`.
   - `Trace Distance` / `Trace Interval` / `Trace Channel` — the interaction ray (targets must **block** the trace channel; Visibility by default).
   - `Prompt Space` / `Prompt Draw Size` / `Prompt Distance Ratio` (0.5 = midway between ray source and hit point) / `Prompt Smoothing Speed`.
   - `Use Key Label` — the prompt's fallback key text, used when the character provides no key label of its own (see the next step).
3. **Character**: open the character Blueprint (e.g. `BP_CombatCharacter`) and:
   - add `GA_Use` to **Default Abilities**;
   - under **Ability System | Input → Event Input Mappings**, add `IA_Use` → `Event.Input.UsePressed`;
   - set the entry's **Key Label** (e.g. "E") — the prompt asks the character for the key bound to each event tag and shows that label, falling back to the ability's `Use Key Label` when it's empty.

No other setup is required. The ability is granted and activated automatically; AI ability instances stay idle (the trace and prompt only run for locally controlled players).

### Placing an action volume

Drag a **CombatActionVolume** into a level and configure it (all settings live on its **Volume** component):

- **Use Text** — action shown in the prompt ("Open", "Talk", ...).
- **Actors To Activate** — every actor implementing the existing `CombatActivatable` interface is activated when the volume is used.
- **On Used** — Blueprint-assignable event fired on use (e.g. open a door, mount a turret).
- **Single Use** — the volume stops being usable after one use.

### Making any actor usable (vehicles, doors, NPCs...)

Add a **CombatActionVolumeComponent** to the actor's Blueprint. It implements `CombatUsable`, so the use ability finds it when the ray hits the **owning actor**, and forwards `CanUse` / `GetUseText` / `Use` to the component. Two ways to provide the trace target:

- leave **Create Trace Box** checked and the component creates an invisible box that blocks only the Visibility channel (characters walk through it, interaction rays hit it), or
- uncheck it and let the owner's own collision serve as the trace target (e.g. a vehicle mesh that already blocks Visibility).

#### Vehicle example

1. Add the component to the vehicle Blueprint, set **Use Text** to "Drive", uncheck **Create Trace Box** (the vehicle mesh blocks the trace).
2. Bind the component's **On Used** event → `Get Player Controller` → `Possess` (the vehicle).
3. While driving, the vehicle can host its **own** ability set the same way (anything built on `CharacterBase` or with its own `AbilitySystemComponent`): give it a `UseAbility` Blueprint for in-vehicle interactions, or simply bind a key to possess the character back for exiting.

Because the interaction ray prefers the **player camera** viewpoint over the avatar's eyes, tracing keeps following the player's actual view even while the controlled pawn changes — which makes possess-style vehicle entry work without any ability changes.

### Custom prompt widgets

Create a Widget Blueprint (either with a designed layout or a `CombatUsePromptWidget` subclass) and implement `CombatUsePromptInterface`:

- **Update Prompt Text (Key Text, Action Text)** is called on every trace while a target is in range — the key label from the ability and the action text from the usable target (its `GetUseText`).

Assign the widget class to `Prompt Widget Class` on the ability Blueprint. The ability never depends on a concrete widget class; `Prompt Space = Screen` renders the same widget camera-facing with a constant pixel size if you prefer a non-diegetic prompt.

### Notes

- The trace and prompt only run for locally controlled players; AI ability instances stay idle.
- The trace channel, distance, interval, prompt widget class and key label are `EditDefaultsOnly` properties on `UUseAbility`; tune them on a Blueprint subclass of the ability.
- Set `bDebugTrace` on the ability to visualize the interaction ray.
- The demo targets standalone/listen-server play. In dedicated-server games, interacting runs on the machine that owns the input; replicate your interactable's effects (or route them through gameplay events) as needed.
- The prompt actor is spawned and destroyed by the ability and never saved into the level.

### Troubleshooting

The use input chain logs three checkpoints in order. Press the use key with the prompt visible and watch the Output Log (`Window → Output Log`, filter for `Input` or `UseAbility`):

| Log line | Checkpoint proven to work |
| --- | --- |
| `[AbilityInput] <pawn> forwarded gameplay event Event.Input.UsePressed` | Enhanced Input fired the mapping and the character forwarded the event |
| `[UseAbility] use input received` | The event reached the passive ability's `WaitGameplayEvent` task |
| `[UseAbility] <pawn> used <target>` | The target's `Use` was called — from here the target's own Blueprint responds |

- **Nothing logs on press** → the `Event Input Mappings` entry isn't firing. Confirm the entry exists on the pawn Blueprint actually used in the level (Input Action and Event Tag set), then run `showdebug enhancedinput` in the console while playing to see whether the action fires and which mappings are active.
- **`[AbilityInput]` logs but `[UseAbility]` doesn't** → the event tag in the character's Event Input Mappings doesn't match `Event.Input.UsePressed` exactly (the logged tag shows what was sent).
- **`[UseAbility] use input received, but no usable target is in range`** → the input chain works; the re-trace on press lost the target (it left range, became unusable, or something now blocks the ray).
- **`[UseAbility] ... used ...` logs but nothing happens in the world** → the interaction fired; the target's response (its action volume's `On Used` binding or `Actors To Activate`) isn't set up.
