# Subagents

## Code mapper

Verdict: needs-more-evidence, likely culprit found.

- `SCR_WeaponBlastComponent.OnWeaponFired()` was unconditional and could emit `explosion=1` at launcher shot time.
- `SCR_MuzzleEffectComponent.OnFired()` is the normal shot hook to keep.
- `BS5_ExplosionEchoEffect.OnEffect()` is the intended projectile-impact explosion hook.

## Prefab scout

Verdict: needs-more-evidence, coverage partial.

- Base inserts do not cover child prefabs that define their own `PROJECTILE_EFFECTS`.
- `Explosives_base.et` is not a useful projectile-effect coverage hook.
- Need same-path leaf replacements for rocket/GL ammo that override projectile effects.

## Final code scout

Verdict: fix-before-ship.

- Found that the first `Launcher_Base.et` override was too thin and risked stripping vanilla launcher behavior.
- Found that owner-prefab launcher detection still needed runtime proof for UGL, so the classifier was narrowed to rocket ammo / launcher-prefab only and UGL ammo was left rifle-like.
- Follow-up fix rebuilt `Launcher_Base.et` with vanilla launcher components plus BS5 overrides.
