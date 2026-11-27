# Architecture

Long-term goal: a **modular third person character plugin** that can be dropped
into other projects. Characters stay thin: a character only wires up input and
a list of abilities, and modular **features** (ranged and melee combat,
movement modes like mantling or swimming, "use" actions, ...) customize the
character through abilities granted from data. A civilian NPC instance simply
never gets the combat or mantling abilities granted - nothing to strip out.

This document maps the current source layout onto that goal and describes how
to extract the plugin when the time comes.

## Current layout

```
Source/AbilityCharacter/
├── AbilityCharacter.Build.cs
├── AbilityCharacter.h/.cpp                  module + log category
│
├── Core/                                    future plugin core
│   ├── Characters/
│   │   └── CharacterBase                    thin GAS character base every
│   │                                        combat character derives from
│   ├── PlayerControllers/
│   │   └── AbilityCharacterPlayerController shared base: Enhanced Input
│   │                                        mapping contexts, touch controls,
│   │                                        Player Start respawn helper
│   ├── AbilitySystem/                       GAS plumbing
│   │   ├── CharacterAttributeSet            Health/Stamina, State.Dead tag
│   │   ├── CombatAbilitySystemComponent     allocates the combat effect
│   │   │                                    context (hit result + knockback)
│   │   ├── CombatEffectContext              FGameplayEffectContext subclass
│   │   ├── CombatGameplayTags               native tags (Combat.*,
│   │   │                                    Ability.Attack.*, State.Stance.*)
│   │   ├── AsyncTaskCooldownChanged         BP async node for cooldown UI
│   │   │                                    (currently unused by content)
│   │   └── HideByTagComponent               hides a component by gameplay tag
│   │                                        (currently unused by content)
│   ├── Interfaces/
│   │   ├── CombatDamageable                 damage/healing/danger reactions
│   │   ├── CombatInputLabelProvider         "which key is bound to this
│   │   │                                    event tag?" for prompts
│   │   └── CombatActivatable                generic actor activation
│   └── UI/
│       ├── ActorWidgetComponent             feeds the owner actor to its
│       │                                    widget via HasActorInterface
│       └── HasActorInterface
│
├── Features/                                one folder per feature slice
│   ├── Combat/
│   │   ├── CombatCharacter / CombatGameMode / CombatPlayerController
│   │   ├── AbilitySystem/
│   │   │   ├── CombatAbilityBase            montage -> trace event -> effect
│   │   │   │                                -> cue chain shared by all attacks
│   │   │   ├── MeleeAbility                 base: trace driven by anim notify
│   │   │   ├── RangedAbility                base: hitscan from view/muzzle
│   │   │   ├── ComboAttackAbility (+ AI)    input-buffered combo string
│   │   │   ├── ChargedAttackAbility (+ AI)  hold-to-charge release
│   │   │   ├── SetStanceAbility             stance = tags + granted abilities
│   │   │   ├── CombatStanceAbility / AICombatStanceAbility / NeutralStanceAbility
│   │   │   ├── StanceMonitorAbility / AIStanceMonitorAbility
│   │   │   │                                context -> stance driver
│   │   │   └── CombatStaminaCostEffect      shared SetByCaller stamina cost
│   │   ├── AI/                              AI controller, enemy, spawner,
│   │   │                                    StateTree tasks/conditions, EQS
│   │   ├── Animation/                       attack trace + combo/charge
│   │   │                                    check anim notifies
│   │   ├── Gameplay/                        arena/checkpoint volumes,
│   │   │                                    dummy, damageable box, lava floor
│   │   └── UI/CombatLifeBar
│   │
│   ├── Interaction/
│   │   ├── AbilitySystem/UseAbility         passive trace + prompt ability
│   │   ├── Interfaces/CombatUsable          CanUse/GetUseText/Use
│   │   ├── Gameplay/CombatActionVolume(+Component)
│   │   └── UI/CombatUsePromptActor/Interface/Widget
│   │
│   ├── Platforming/                         double/wall jump, coyote time,
│   │                                        dash (plain ACharacter)
│   └── SideScrolling/                       2.5D movement, jump pads,
│                                            moving/soft platforms, NPC
│
└── Template/                                original third person demo
    ├── AbilityCharacterCharacter             classes; redirect targets for
    └── AbilityCharacterGameMode              the template Blueprints
```

Dependency rules (already enforced by the layout):

1. `Core/` depends only on engine modules.
2. A feature may depend on `Core/`, never on another feature.
3. `Template/` depends on nothing; features do not depend on it (only
   feature player controllers derive from the base controller that lives in
   Core, not from the Template classes).

Class hierarchy of the combat slice:

```
ACharacter
└── ACharacterBase (Core)                    GAS, damageable, cue handling
    ├── ACombatCharacter  (Features/Combat)  player combat character
    └── ACombatEnemy      (Features/Combat)  AI combat character

UGameplayAbility
├── UCombatAbilityBase                       montage/effect/cue attack chain
│   ├── UMeleeAbility
│   │   └── UComboAttackAbility ── UAIComboAttackAbility
│   └── URangedAbility                       (awaiting the rifle feature)
├── USetStanceAbility ── UCombatStanceAbility / UAICombatStanceAbility
│                     └── UNeutralStanceAbility
├── UStanceMonitorAbility ── UAIStanceMonitorAbility
└── UUseAbility                              (Features/Interaction)
```

## How a feature customizes a character

Characters stay thin because every feature reaches the character through data,
not code. `ACharacterBase` knows only `UGameplayAbility`, `FGameplayTag` and
two generic tables configured on the character Blueprint:

| Concern | Mechanism |
| --- | --- |
| Granting | `DefaultAbilities` - abilities granted at init (e.g. add `GA_Use` to get the interaction feature) |
| Input | `EventInputMappings` - Input Action -> gameplay event tag table; abilities listen with `WaitGameplayEvent` |
| Ability activation | `AbilityInputMappings` - Input Action -> ability tag (activated by tag, no class reference) |
| Stances | `USetStanceAbility` grants/revokes feature abilities while a stance is active (combat stance arms the attacks) |

Adding a feature to a character = add rows to these tables; removing it
removes the feature cleanly. No C++ changes, no character subclasses per
feature. Keep this pattern for new features (the rifle ability below included).

## Future plugin layout

Target: `Plugins/ModularCharacter/` with one runtime module per concern. The
current folders map 1:1:

| Today | Plugin home |
| --- | --- |
| `Core/` | `Plugins/ModularCharacter/Source/ModularCharacterCore/` (Runtime, Default phase) |
| `Features/Combat/` | `Plugins/ModularCharacter/Source/ModularCharacterCombat/` (Runtime, Default phase) |
| `Features/Interaction/` | `Plugins/ModularCharacter/Source/ModularCharacterInteraction/` |
| `Features/Platforming/`, `Features/SideScrolling/` | separate plugins, or game-module features - they are already self-contained |
| `Template/` | stays in the game module; it is demo scaffolding, not plugin material |

Within a module, keep the same sub-folders (`AbilitySystem/`, `AI/`,
`Animation/`, `Gameplay/`, `Interfaces/`, `UI/`) and the module-root-relative
include style, so the move is `git mv` + `.uplugin` + `Build.cs`, not a
rewrite.

### Extraction recipe (per module)

1. `Plugins/ModularCharacter/Source/ModularCharacterCore/` with a
   `ModularCharacterCore.Build.cs` (dependencies: the current Public list minus
   what only features need), plus `Public/` and `Private/` if desired.
2. `git mv` the `Core/` files in, keep folder names.
3. Fix includes: they are already module-root-relative; only the root changes
   (`Core/...` -> files at the new module root, e.g. `Characters/...`).
   No class renames, so **no CoreRedirects needed** and no asset breakage.
4. Move the module's `.uproject` dependency entries into the plugin
   `.uplugin`.
5. Repeat for each feature module; feature modules depend on Core only.
6. The game module keeps `Template/`, maps and project-specific glue.

`CombatGameplayTags` (`Combat.*`, `Ability.Attack.*`, `State.Stance.*`) moves
with Core; native gameplay tags are registered globally by module, so tags
keep working when their defining module moves.

## Next feature: the rifle (ranged combat)

`URangedAbility` (Features/Combat/AbilitySystem) is already a working hitscan
base: on activation it traces from the player view (or the `muzzle` socket on
the avatar mesh), applies the shared damage effect and executes the damage
cue, reusing `UCombatAbilityBase`'s montage/effect/cue chain. The rifle
feature therefore needs **no new base class**:

1. Assets first (your current blocker): a rifle mesh (socket named `muzzle`),
   idle/aim-fire animation montages, optionally an aim-offset. Typical
   sources: Fab/marketplace weapon packs, or a Mixamo/UE5 mannequin retarget
   for animations. *(Suggestions only - pick what fits your art direction.)*
2. `URifleAbility : URangedAbility` (or a Blueprint subclass): montage,
   damage tuning, `MuzzleSocketName`, ammo handling. Trigger it from a new
   `Combat.Input.FirePressed` event tag + an `EventInputMappings` row.
3. Optional later: an ammo attribute in `UCharacterAttributeSet` and a
   SetByCaller cost effect (mirror `UCombatStaminaCostEffect`), plus a fire
   loop / automatic fire mode.

## Editor housekeeping (once)

Several level assets still reference the pre-rename module path
(`/Script/TP_ThirdPerson.*`) and the old template class names; the
`ActiveGameNameRedirects` / `ActiveClassRedirects` in `Config/DefaultEngine.ini`
currently resolve them. After this cleanup compiles, run once in the editor:

1. Open every map in the project (or use the Resave Project utility) so all
   assets load through the redirects.
2. Resave. The redirect-resolved paths get baked into the assets.
3. Afterwards the `TP_ThirdPerson` redirects and - if no Blueprint still
   derives from them - the `Template/` classes can be deleted in a follow-up
   change. Until then, **leave the redirects in place**: 88 external actors in
   `Lvl_Combat` alone reference the old module name.

Content folders (`Content/Variant_Combat/...`) are unrelated to the source
layout and were intentionally left untouched; renaming them is an in-editor
operation (Fix Up Redirectors) if you want it.
