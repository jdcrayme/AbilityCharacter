# Cleanup pass - change log

Scope agreed for this pass: fix the UE 5.8 deprecation warnings, tidy safely
(**no public class names or APIs change**), refactor code smells where
risk-free, reorganize `Source/` into a `Core/ + Features/` layout, and
document the plugin extraction plan. No new gameplay (the rifle ability waits
for assets).

## Applying the changes

The deliverable is a git patch series (`patches/0001..0007`). From your local
clone on `main`:

```
git checkout -b cleanup-pass
git am /path/to/patches/*.patch     # or: git apply --3way *.patch
```

Then right-click the `.uproject` -> Generate project files, and build. If you
prefer, apply only the first commits (the warning fix is self-contained in
`0001`).

## What changed, commit by commit

### 0001 fix(abilities): replace deprecated AbilityTags with SetAssetTags

The five C4996 warnings from the build log. `UGameplayAbility::AbilityTags`
is deprecated for game code and will become private in a future release; each
ability constructor now sets its tag through `SetAssetTags(FGameplayTagContainer(TAG_...))`.
No other deprecated API usage was found in the module (Enhanced Input's
`AddMappingContext` is still current in 5.8; native gameplay tags are already
used throughout).

### 0002 fix(attributes): rep-notify StaminaMax correctly

Real bug found during the sweep: `OnRep_StaminaMax` broadcast the **HealthMax**
attribute in its `GAMEPLAYATTRIBUTE_REPNOTIFY` (copy-paste error), so clients
never received proper `StaminaMax` change notifications. Also declares the
`UAttributeSet` overrides as `virtual ... override` and tidies comments.

### 0003 refactor(controllers): share input mapping and touch logic

The four player controllers carried four verbatim copies of the
input-mapping-context setup, touch-controls widget handling and
`ShouldUseTouchControls`. The three feature controllers now derive from
`AAbilityCharacterPlayerController` (which already contained exactly that
logic), and a shared `RespawnPawnAtPlayerStart` helper replaces the duplicated
`GetAllActorsOfClass` respawn code in the platforming and side-scrolling
controllers. Net -290 lines.

Blueprint-safe: the shared properties keep their names and types (they only
move up the hierarchy), so existing Blueprint children keep their configured
values.

### 0004 refactor(enemy): share AI attack activation

`ACombatEnemy::DoAIComboAttack` / `DoAIChargedAttack` were two copies of the
same activation flow; both now delegate to a private `TryAIAttack(tag)`. Also
drops a dead commented-out line and unused includes, and fixes stray
formatting in `AnimNotify_DoAttackTrace`.

### 0005 chore: hygiene pass

Copyright headers and real doc comments for `ActorWidgetComponent`,
`HasActorInterface`, `AsyncTaskCooldownChanged`, `HideByTagComponent` (was
template placeholder text); UTF-8 BOM stripped from `HideByTagComponent`;
`HideByTagComponent` now includes its actual base (`SceneComponent.h`);
five unused includes removed from `CharacterBase.cpp`.

### 0006 refactor: reorganize into Core/ + Features/ + Template/

Pure file moves plus include-path updates - **no class renames, no Content
changes** - so Blueprints and level assets are unaffected. All project
includes are now module-root-relative (UE convention), and `Build.cs` keeps
only the module root as include path. `ARCHITECTURE.md` documents the new
layout and the plugin mapping. `README.md` file paths updated to match.

### 0007 docs: ARCHITECTURE.md + this file

## What I could not verify here

This cleanup was produced outside the engine (no UE 5.8 compiler available),
so it is verified statically, not by compilation:

- every `#include` in the module resolves to an existing file or a known
  engine header (scripted check, all pass);
- no `AbilityTags` / other deprecated API calls remain (grep, clean);
- every moved file exists at its new path and headers stay with their cpp
  files (scripted check, all pass).

**Build the project once after applying** and expect a clean compile; if
anything slips through it will be an include path - each fix is one line.

## Load-bearing config - do not remove prematurely

`Config/DefaultEngine.ini` contains the standard template redirects:

```
+ActiveGameNameRedirects=(OldGameName="/Script/TP_ThirdPerson", NewGameName="/Script/AbilityCharacter")
+ActiveClassRedirects=(OldClassName="TP_ThirdPersonCharacter", ...) etc.
```

These are **actively used**: 88 external actors in `Lvl_Combat` reference
`/Script/TP_ThirdPerson.*` classes, and the three template Blueprints
(`BP_ThirdPersonCharacter/GameMode/PlayerController`) reference the old class
names. That is why the three demo classes were moved to `Template/` instead
of deleted. After an editor "resave all through redirects" pass (see
ARCHITECTURE.md), the redirects and the `Template/` classes can be dropped in
a follow-up change.

## Deliberate non-changes

Kept intentionally, with reasons:

- **`Template/` classes** - redirect targets for the template Blueprints
  (above).
- **`UAsyncTaskCooldownChanged` and `UHideByTagComponent`** - referenced by
  neither C++ nor Content today, but both are generic plugin-core material
  (cooldown UI node, tag-driven visibility). Delete them in a follow-up if
  you disagree.
- **Damage-application duplication** - `CharacterBase::TakeDamage`,
  `ACombatLavaFloor::OnFloorHit` and `ACombatDamageableBox::ApplyDamage` each
  apply a damage effect + cue with slightly different parameters. A shared
  helper is possible but each site has different context details; left as a
  documented future refactor rather than risked blind.
- **`ConstructorHelpers` in ability constructors** (montage/effect lookups) -
  works, but hard-references content from C++. Move to Blueprint-configurable
  `TSoftObjectPtr`s when you touch those abilities next.
- **`LogTemp` logging** - left in place; converting to a dedicated log
  category is trivial but touches many files for little gain this pass.
- **Content folders** (`Content/Variant_Combat/...`) - untouched; renaming
  them is an in-editor operation (Fix Up Redirectors), not a git change.

## Suggested follow-up order

1. Apply patches, regenerate project files, compile, smoke-test the three
   game modes (combat level, platforming, side-scrolling).
2. Resave project through the redirects, then drop the `TP_ThirdPerson`
   redirects + `Template/` classes.
3. Pick rifle assets/animations, then build the rifle on `URangedAbility`
   (plan in ARCHITECTURE.md).
4. Extract `Core/` into the `ModularCharacter` plugin when the next project
   needs it (recipe in ARCHITECTURE.md).
