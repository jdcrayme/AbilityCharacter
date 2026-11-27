# Cleanup pass 2 - change log

Scope agreed for this pass: the follow-ups identified after the first cleanup
pass landed - replicated stance tags, per-row input trigger events, the
ranged-ability fire-timing fix, a generic gameplay-event anim notify, a shared
passive-ability base, tag-tree normalization, and repo hygiene. Still no new
gameplay (the weapon feature is designed in `ARCHITECTURE.md` and waits on the
next step), still no public API breaks (one property rename, redirected).

## Applying the changes

The deliverable is a git patch series (`patches/0001..0008`). From your local
clone on `main`:

```
git checkout -b cleanup-pass-2
git am /path/to/patches/*.patch     # or: git apply --3way *.patch
```

Then right-click the `.uproject` -> Generate project files, and build. Smoke
test: combat level (combo/charged attacks, stance swap on arena entry/exit,
use prompts), then the things listed under "Editor follow-ups" below.

## What changed, commit by commit

### 0001 fix(input): per-row trigger events on EventInputMappings

`FInputActionEventMapping` gains a `TriggerEvent` property
(Started/Completed/Triggered; default Started, so existing rows behave
exactly as before). Hold-style abilities can now receive the press AND the
release of one input action through two rows - which is what the weapon
feature needs (aim hold, fire press/release) and what previously forced
`ACombatCharacter` to hardcode the charged attack's release binding in C++.

### 0002 fix(tags): replicate stance tags through a gameplay effect slot

Real multiplayer (and anim-BP-on-client) bug: `SetStanceAbility` used
`AddLooseGameplayTag`, and loose tags are server-only - no client ever saw
`State.Stance.*`, and they were never removed on stance change or death.
Stance tags are now applied through
`UCombatAbilitySystemComponent::ApplyGrantedTags(SlotTag, Tags)`: an infinite
`UCombatTagGrantEffect` carries the tags on the spec's `DynamicGrantedTags`,
replicates to every client, and one slot per feature group (`State.Stance`)
replaces the previous stance's tags when re-applied. `SetStanceAbility` falls
back to loose tags for plain `UAbilitySystemComponent`s.

The stamina/death tags in `CharacterAttributeSet` stay loose deliberately:
they gate server-side logic only, and applying effects from inside
`PostGameplayEffectExecute` needs its own careful pass.

### 0003 refactor(passive): shared auto-activate base for passive abilities

The grant-means-activate `OnGiveAbility` block existed verbatim in
`UUseAbility` and both stance monitors. Extracted to
`UPassiveAbilityBase` (Core/AbilitySystem), which also owns the
InstancedPerActor/no-retrigger policy they shared. Blueprint-safe: native
parent changes need no redirects; `GA_Use` keeps its values.

### 0004 refactor(abilities): notify-driven ranged fire + authority-gated damage

Three fixes in the attack chain:

- `URangedAbility` no longer hitscans at activation (the shot landed before
  the montage's first frame) and no longer fires after a montage-less
  activation was already ended by the base class. It now fires on the
  montage's attack trace notify - the same discipline melee traces use - with
  an end-of-montage fallback for montages without notifies, one shot per
  activation.
- `UCombatAbilityBase::PerformHitCheck/ApplyDamageToTarget` are gated on
  avatar authority so predicted client instances only run cosmetics.
- The player attack abilities (`UMeleeAbility` branch, `URangedAbility`) are
  `LocalPredicted`: the owning client starts its montage immediately, the
  server applies the damage. Single-player behavior is unchanged.
- `MeleeTraceRadius` renamed to `TraceRadius` (it is also the hitscan sphere
  radius) with an `ActivePropertyRedirects` entry so `GA_Punch` keeps its
  value. `OnMontageFinished`/`PerformHitCheck` became virtual for the ranged
  override.

### 0005 feat(anim): generic gameplay-event anim notify

`UAnimNotify_SendGameplayEvent` (Features/Combat/Animation) sends a
configurable gameplay event to the mesh owner, carrying itself as
`OptionalObject` plus an optional `BoneName` payload - the exact contract
`CombatAbilityBase` already reads from `AnimNotify_DoAttackTrace`. Weapon
montages get their fire frame (`Event.AttackTrace`), magazine-in moment
(`Event.Weapon.MagIn`) and equip attach beat as notify *data* instead of new
notify classes. The three dedicated notifies stay for the existing montages.

### 0006 chore(tags): one event tag tree (Event.*)

The event tags had three shapes (`Combat.AttackTrace`,
`Combat.Input.PrimaryPressed`, `Combat.Event.ArenaEntered`). All are
`Event.*` now (`Event.Input.*`, `Event.AttackTrace`, ...). Renames are covered
by `+TagRedirects` in `Config/DefaultGameplayTags.ini`; the only content
reference to an old string was `BP_CombatCharacter`'s EventInputMappings row.
Legacy ini tags (`Ability.Punch`, `Input.*`) remain - they are referenced by
content.

### 0007 chore: repo hygiene

`.idea/` and the generated `*.slnx` are untracked and gitignored; the dead
`LogCombatCharacter` declaration is gone; the 16 `LogTemp` calls now use the
module's `LogAbilityCharacter` category. `IA_Test.uasset` was NOT deleted:
`IMC_Combat` and `BP_CombatCharacter` reference it - check what it is bound
to in the editor before removing.

### 0008 docs: this file + ARCHITECTURE.md/README updates

Corrected the rifle plan: weapon meshes, fire sound and projectile are in
`Content/Weapons`, the full rifle/pistol animation sets were already in the
mannequin content (equip/fire/reload/dry-fire, ADS idles, aim offsets), and
`Muzzle` sockets exist on the weapon skeletons. The rifle is unblocked.

## What I could not verify here

Same caveat as pass 1: produced without a UE 5.8 compiler, verified
statically:

- every `#include` resolves to a project file or a known engine header
  (scripted check, all pass);
- engine API usage is limited to APIs already used in this repo
  (`EGameplayEffectDurationType` from `CombatStaminaCostEffect`,
  `DynamicGrantedTags` verified against the engine source and GAS docs,
  `ETriggerEvent`/`InputActionValue.h` verified against Epic's samples);
- no deprecated API calls, no leftover old tag strings (grep, clean);
- the property rename carries an `ActivePropertyRedirects` entry.

**Build once after applying.**

## Editor follow-ups (data only, no C++)

1. **Resave through the redirects**: open the project, let it load (the tag
   redirects and the property redirect remap on load), then resave the
   affected assets (`BP_CombatCharacter`, `GA_Punch`, maps with external
   actors). After that, the `+TagRedirects` and
   `+ActivePropertyRedirects` entries can be dropped.
2. **Migrate the combat attacks to EventInputMappings** (optional but
   recommended): in `BP_CombatCharacter`, add rows
   `IA_ComboAttack -> Event.Input.PrimaryPressed (Started)`,
   `IA_ChargedAttack -> Event.Input.SecondaryPressed (Started)`,
   `IA_ChargedAttack -> Event.Input.SecondaryReleased (Completed)`, then
   remove `ComboAttackAction`/`ChargedAttackAction` from the character's
   input properties and delete the hardcoded bindings in a small follow-up
   patch. Until then the C++ bindings keep working - both paths forward the
   same events.
3. **Check `IA_Test`** in `IMC_Combat`: it is wired to a key; confirm it is
   intentional or remove it (and its references) in the editor.

## Known multiplayer gap (for the weapon feature)

Input-derived gameplay events are sent locally (`SendGameplayEventToActor`)
and are not replicated. Activation works (LocalPredicted abilities notify the
server), but *intra-ability* events - the charged attack's release event -
only fire on the sending machine. The weapon feature should use replicated
events (`ServerSetReplicatedEvent`/generic replicated events) or the ASC's
ability-input binding for press/release semantics. Also still open from pass
1: the `TP_ThirdPerson` resave-before-redirect-removal dance, and the
`ConstructorHelpers` content pins in the attack abilities (move to
Blueprint-configurable `TSoftObjectPtr`s when the weapon abilities touch
them).

## Deliberate non-changes

- **Stamina/death loose tags in the attribute set** - server-side gating
  only; converting them means applying effects from within
  `PostGameplayEffectExecute`, which needs its own pass.
- **`ACombatCharacter`'s hardcoded attack bindings** - kept until the data
  rows exist (editor follow-up 2); both paths forward identical events.
- **The three dedicated anim notifies** - baked into the existing montages;
  new content uses `UAnimNotify_SendGameplayEvent`.
- **`.mcp.json` / `.zcodeignore`** - left tracked; they are tool config you
  added deliberately. Review yourself.
- **Legacy ini tags** (`Ability.Punch`, `Input.*`) - referenced by content.

## Suggested follow-up order

1. Apply, build, smoke-test, then do the three editor follow-ups above.
2. Build the rifle feature on `URangedAbility` (plan in `ARCHITECTURE.md`;
   assets are in place).
3. Extract `Core/` into the `ModularCharacter` plugin when the next project
   needs it (recipe unchanged from pass 1).
