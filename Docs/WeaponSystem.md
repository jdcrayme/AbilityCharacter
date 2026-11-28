# Weapon System Patch Pack — Setup Guide

This patch set adds a modular weapon system to the AbilityCharacter project:

- equipping / drawing / holstering through a gameplay ability and a passive
  `UWeaponEquipmentComponent` (weapons spawn from `UWeaponData` assets)
- firing through a data-driven `UWeaponFireAbility` (montage -> attack trace
  notify -> effect -> cue chain inherited from `UCombatAbilityBase`)
- reloading through `UWeaponReloadAbility`, which transfers rounds from the
  `UCombatInventoryComponent` reserve into the weapon's
  `UWeaponStateComponent` magazine at the magazine-in montage notify
- aiming through `UWeaponAimAbility` (persistent hold-style ability owning
  the replicated `State.Aiming` tag)
- all weapon state communicated through the replicated `State.Weapon` tag
  slot (the same `ApplyGrantedTags` mechanism the stance system uses)

New weapons are new `UWeaponData` assets, never new C++ classes.

---

## 1. Design decision this patch implements

**Why the magazine is weapon-local and the reserve is inventory** (the
"should the weapon state component be a generic inventory" question):

- The **inventory** is the possession layer: what the character carries, in
  what quantities, keyed by tag (`Ammo.Rifle x 90`). It is generic,
  weapon-agnostic, and answers questions like "what does the character have
  and can it be dropped/stacked/looted".
- The **magazine** is the weapon's operational state: it changes at the
  exact fire-frame notify, mid-reload at the mag-in notify, and it must
  travel with the weapon actor when the weapon is dropped or handed over.
  Keeping it on the weapon actor makes "dropped with the weapon" automatic:
  the actor owns its own state, and no character-side inventory transaction
  is needed on drop/pickup.
- The **ammo type tag** is the type system shared by both layers: the weapon
  declares the type it consumes, the inventory counts entries keyed by the
  same tag, and reload is a plain tag-keyed transfer between the two pools.

Boundary rule: *the inventory answers what the character possesses; the
weapon state answers what the weapon is doing.*

---

## 2. What the patches deliver (and what they cannot)

The patches deliver **C++ only**. This environment has no Unreal Editor, no
engine binaries and no content pipeline, so everything that lives in
`.uasset` files — Blueprints, montages, input action assets, gameplay effect
assets, sockets on meshes, anim layers — is not in the patches and is listed
below as editor work.

**The code was written against the exact patterns already in this repo** (tag
macros, `ApplyGrantedTags`, `CombatAbilityBase`'s montage chain,
`AnimNotify_SendGameplayEvent`, `ConstructorHelpers` conventions, module-
relative includes) but **it has not been compiled here**. Treat the first
editor compile as the real test; any compile fixes are small and expected
(they are normal C++ fixes, not design changes).

Apply order matters (each patch depends on the previous ones):

```bash
git am 0001-weapons-native-gameplay-tags.patch
git am 0002-*.patch
...
# or all at once:
git am *.patch
```

---

## 3. Editor work required to bring the demo up

Steps marked **[MCP-able]** can be automated through an Unreal Editor
automation server (MCP) or editor Python scripting if one is connected;
steps marked **[Manual]** need hands in the editor. Nothing below is
optional for a fully animated demo, but step ordering is: the system works
without animation first (montageless instant draw/fire paths exist for
exactly that reason).

### 3.1 First compile  [Manual]

1. Apply the patches, open the project, let the module recompile.
2. Fix any compile errors (expected to be trivial: include paths or
   API-macro nits — see section 5).

### 3.2 Character skeleton sockets  [MCP-able]

The equipment component attaches weapons to sockets on the character mesh:

- `hand_r` (default `HandSocketName`) — where drawn weapons attach
- `spine_03` (default `BackSocketName`) — where holstered weapons attach

Add both sockets to the character skeleton at sensible locations (the names
are `EditAnywhere` on `UWeaponEquipmentComponent`, so per-character
overrides are a BP edit, not a code change).

### 3.3 Weapon mesh sockets  [MCP-able]

Weapon meshes need:

- `muzzle` (name configurable per weapon via `FWeaponFiringData.MuzzleSocketName`)
- `GripLeft` — the left-hand IK target for two-handed grips

### 3.4 The `DA_Weapon` data asset  [MCP-able]

Create `UWeaponData` assets (e.g. `DA_Weapon_Rifle`):

- `WeaponActorClass`: `AWeapon` (the C++ class in this patch)
- `Mesh`: a skeletal weapon mesh with the sockets above
- `OverlayAnimLayerClass`: the anim layer from 3.6 (optional — works
  without, just no overlay pose)
- `GrantedAbilities`: the four C++ abilities
  (`UWeaponFireAbility`, `UWeaponReloadAbility`, `UWeaponAimAbility`,
  `UWeaponEquipAbility`) with tags `Ability.Weapon.Fire` / `.Reload` /
  `.Aim` / `.Equip`
- `Firing.BaseDamage`, `FireInterval`, `TraceDistance`, `TraceRadius`,
  `MuzzleSocketName`
- `Ammo.AmmoTypeTag`: `Ammo.Rifle` (native tag added by this patch) or a new
  one (then declare it in `DefaultGameplayTags.ini`)
- `Firing.DamageEffect`: reuse the existing `GE_DamageHealth`
- `Firing.CooldownEffect`: see 3.5
- Montages from 3.7 (optional — all four abilities have montageless paths)

### 3.5 Gameplay effect assets  [MCP-able]

- **Cooldown effect**: one GE with Duration Policy `HasDuration`, duration
  magnitude set by caller with the tag `Data.Cooldown`. The fire ability
  feeds it the weapon's `FireInterval`, so one asset paces every weapon.
  (Without it, fire cadence falls out of the fire montage's length.)
- **Aim movement effect** (optional): a walk-speed modifier GE for
  `UWeaponAimAbility::AimMovementEffect`.
- The damage effect is reused from existing content (`GE_DamageHealth`).

### 3.6 Animation layers  [Manual — graph work]

1. In the character's anim blueprint, add a **layer** (e.g.
   `WeaponOverlay`) that the weapon layers will implement.
2. Create an anim layer asset for the weapon category (e.g.
   `AL_Weapon_Rifle`) implementing that interface: upper-body pose for
   locomotion, full-body for reloads, its own aim offsets. Linking happens
   through `LinkAnimClassLayers` from this patch's code at the draw/holster
   moment — no graph work needed for the linking itself.
3. The skeleton must be compatible with the character's (linked layers
   require matching bone names); document this contract for any character
   consuming the weapon feature.
4. Optional: drive the aim pose off the `State.Aiming` tag (the anim
   blueprint can read gameplay tags through the standard tag queries).

### 3.7 Montages and notifies  [MCP-able]

- **Fire montage**: place the existing `AnimNotify_DoAttackTrace` at the
  fire frame (same notify the ranged attacks already use).
- **Equip / unequip montages**: place `AnimNotify_SendGameplayEvent` with
  tag `Event.Weapon.Swap` at the frame the hand reaches the weapon.
- **Reload montage**: place `AnimNotify_SendGameplayEvent` with tag
  `Event.Weapon.MagIn` where the magazine seats.

### 3.8 Input wiring  [MCP-able]

New input action assets and rows in the input mapping context, then rows on
the character blueprint:

- `AbilityInputMappings`: fire -> `Ability.Weapon.Fire` etc. if using
  direct activation, **or** (matching the existing charged attack pattern)
  `EventInputMappings` rows forwarding `Event.Input.FirePressed` /
  `ReloadPressed` / `AimPressed` + `AimReleased` / `EquipPressed` — the
  weapon abilities trigger on those events natively.
- For **automatic fire**, add a `Triggered` row for the fire input action
  (the fire ability is instanced and cooldown-gated, so each trigger that
  passes the cooldown fires one shot with a full commit).

### 3.9 Character blueprint  [MCP-able]

On the demo character BP (derived from `ACombatCharacter`):

- assign `StartingWeapon` (the `DA_Weapon` asset) and
  `StartingAmmoReserve`
- re-parenting or recompiling BPs after the native class gained components
  is automatic; the civilian-NPC variant derives from a base without the
  equipment components (they are added by `ACombatCharacter`, not by
  `ACharacterBase`)

### 3.10 HUD  [MCP-able]

- ammo counter: bind to the weapon state component's `OnAmmoChanged`
  (mag / mag size) and the inventory's `OnEntryChanged` (reserve)
- prompts: `ICombatInputLabelProvider::GetKeyLabelForEvent` already
  surfaces the key labels for the new event tags

### 3.11 Netcode verification  [Manual]

Run a dedicated server + client: fire consumes magazine server-side
(LocalPredicted montage + server damage), reload transfers reserve ->
magazine server-side, equips replicate through the `State.Weapon` slot and
the weapon actor replication. Simulated proxies get attach/layer through
OnReps.

---

## 4. Known follow-ups (not in this patch)

- weapon swap between two equipped weapons (currently equip replaces)
- drop/pickup: `UnequipWeapon` destroys the actor; a drop flow hands the
  actor (with its magazine) to a pickup actor instead
- inventory FastArraySerializer upgrade when entry counts grow
- first-shot accuracy / spread curves (`State.Aiming` is already available
  for tightening)
- projectile weapons: derive a `UWeaponProjectileFireAbility` from
  `UCombatAbilityBase` following the same data-driven pattern
- tracer/tracer effect: `OnWeaponFired` BlueprintImplementableEvent receives
  the muzzle transform on the fire frame

## 5. Compile risk register

Code was not compiled here (no engine available). Highest-risk API touch
points, in order of the confidence this repo's own patterns back them:

1. `LinkAnimClassLayers` / `UnlinkAnimClassLayers` on
   `USkeletalMeshComponent` (standard dynamic linking API).
2. `FGameplayEffectSpec::SetSetByCallerMagnitude` for cooldown duration —
   mirrors the repo's damage SetByCaller usage exactly.
3. `ASC->RegisterGameplayTagEvent` / `UnRegisterGameplayTagEvent` — the repo
   already uses the register side in `ACharacterBase`.
4. Replicated `TObjectPtr<AWeapon>` / `TObjectPtr<UWeaponData>` properties —
   routine.
5. `AnimNotify_SendGameplayEvent` delivery of `Event.Weapon.*` events — the
   notify already exists and its header comment anticipates these tags.

Everything else mirrors code paths already present in the repo
(`ApplyGrantedTags`, `GiveAbility/ClearAbility`, montage tasks, the damage
effect context chain).
