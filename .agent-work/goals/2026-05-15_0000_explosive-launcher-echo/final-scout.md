# Final Scout

Verdict: ship.

Blockers/high:

- None.

Evidence:

- `Scripts/Game/BS5_ExplosionBridge.c` now leaves `SCR_WeaponBlastComponent.OnWeaponFired()` as vanilla `super` plus optional BS5 debug skip; old `HandleExplosionFire()` call is gone.
- New same-path ammo prefabs add `BS5_ExplosionEchoEffect` to the local collision-trigger projectile effects for PG22, PG7VM, PG7VL, PG7VR, RPG75, and M433.
- M72 was already covered in the current tree.

Unknowns:

- Live runtime/resource reload was not rechecked.
- Wider external modded launcher coverage still depends on the exact leaf prefab path and whether it overrides projectile effects.
