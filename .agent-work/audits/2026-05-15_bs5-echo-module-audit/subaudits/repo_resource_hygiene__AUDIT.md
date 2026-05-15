# Subaudit - Repo and resource hygiene

## Scope

Read-only review of repo state, manifest identity, generated files, Workbench handler scripts, and high-risk resource references.

## Files inspected

- `addon.gproj`
- `.agent-work/project-map.md`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Weapons/Core/MachineGun_Base.et`
- `Prefabs/Props/BS5_TailEmitter_MG.et`
- `Prefabs/Props/BS5_TailEmitter_Silenced.et`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Sounds/Weapons/Rifles/BS5/*.acp*`
- `Scripts/WorkbenchGame/EnfusionMCP/*`

## MAP context used

The map already flagged ACP/path drift, missing `Sounds/FinalMix.afm`, `.acp.acp` naming drift, tracked Workbench MCP handlers, and local scratch hygiene.

## Enfusion MCP checks

No Workbench mutation or validation was run. This subaudit used text resource references and git state rather than prefab mutation.

## Functional summary

The strongest publish/runtime risk is resource integrity: multiple ACP graphs still reference a missing `Sounds/FinalMix.afm`, while machinegun and silenced echo paths are split between `.acp` references and checked-in `.acp.acp` resources.

## Findings

### A-001: Missing `Sounds/FinalMix.afm` is still referenced by ACP graphs

- Severity: High
- Category: resource drift / stale reference
- Evidence: `Sounds/FinalMix.afm` is absent, but multiple ACP files reference `{B764D803219C775E}Sounds/FinalMix.afm`, including `Weapons_Rifles_EchoMaster.acp`, `Weapons_MG_EchoMaster.acp.acp`, `Weapons_Silinced_EchoMaster.acp.acp`, and slapback ACPs.
- Why this is AI-slop / risk: The resource graph carries a deleted or unregistered dependency and can fail at audio project load/play time.
- API/BIKI/base-game verification: BI audio docs confirm ACP graphs depend on referenced audio resources.
- Behavior risk if changed: Rebuilding ACPs or restoring the mix asset can change final audio routing and must be checked in Workbench/audio playback.
- Cleanup direction: Restore/register `Sounds/FinalMix.afm` or rewire/rebuild every ACP that references it.

### A-002: Machinegun and silenced ACP references have `.acp` / `.acp.acp` drift

- Severity: High
- Category: resource drift / config mismatch
- Evidence: `MachineGun_Base.et`, `BS5_TailEmitter_MG.et`, and `BS5_EchoDriverComponent.MACHINEGUN_MASTER_ACP` reference `Weapons_MG_EchoMaster.acp`, while the checked-in resource meta names `Weapons_MG_EchoMaster.acp.acp`. `Weapon_Base.et` mixes `Weapons_Silinced_EchoMaster.acp.acp` in `m_sSuppressedMasterAcp` with legacy `.acp` fields, and `BS5_TailEmitter_Silenced.et` uses the `.acp` path.
- Why this is AI-slop / risk: One logical asset has multiple resource identities. Some paths can silently fall back to prefab `SoundComponent` or be blacklisted by the invalid audio cache.
- API/BIKI/base-game verification: Resource names in `.meta` files are the current project resource names; runtime audio APIs require valid project paths.
- Behavior risk if changed: Broad replacement can pick the wrong canonical resource if Workbench has an alias or pending generated state.
- Cleanup direction: Decide the canonical ACP filenames in Workbench, then map every script/prefab reference to the correct GUID/path one by one.

### A-003: Tracked Workbench MCP handler scripts are publish hygiene risk

- Severity: Medium
- Category: publish hygiene / generated tooling
- Evidence: `git ls-files Scripts/WorkbenchGame/EnfusionMCP/*` returns 20 tracked handler scripts.
- Why this is AI-slop / risk: Local Workbench helper scripts can be accidentally shipped as gameplay content.
- API/BIKI/base-game verification: The project AGENTS policy says handler scripts should be cleaned before publish/release hygiene when injected.
- Behavior risk if changed: Removing them may break current MCP Workbench control until reinjected.
- Cleanup direction: Before release, decide whether these are intentionally versioned. If not, remove from publish scope and use `wb_cleanup` after Workbench work.

### A-004: Scratch/generated files need checkpoint policy

- Severity: Low
- Category: repo hygiene
- Evidence: `.agent-work/project-map.md` is untracked scratch; `console.log` and `error.log` are tracked root files.
- Why this is AI-slop / risk: Audit artifacts and generated logs can pollute release checkpoints.
- API/BIKI/base-game verification: Not applicable.
- Behavior risk if changed: None for runtime if handled as repo hygiene.
- Cleanup direction: Keep `.agent-work` out of publish commits unless requested; review tracked logs before release.

## Duplicate/overlapping logic

Resource references overlap across script constants, weapon-prefab fields, legacy components, and emitter prefab `SoundComponent` fields.

## Dead or unreachable code candidates

None from repo scan alone.

## Performance hotspots

Broken resource references can force fallback attempts and invalid-cache behavior in the audio hot path.

## API uncertainty / required follow-up

This subaudit did not validate whether Workbench/runtime tolerates `.acp` aliases for checked-in `.acp.acp` resources.

## Sanity notes

No files were mutated by the subagent or main agent outside the audit artifacts.
