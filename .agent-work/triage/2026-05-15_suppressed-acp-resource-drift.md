# Triage - Suppressed ACP resource drift

Generated: 2026-05-15
Mod root: G:\BettersMods\BettersMods\BetterSounds5_Echo_Module

## Symptoms

Workbench/runtime logs repeatedly failed to open:

`{5B784125E2E54AA0}Sounds/Weapons/Rifles/BS5/Weapons_Silinced_EchoMaster.acp`

The failure happened while spawning:

`{A59D3E1092B44A6C}Prefabs/Props/BS5_TailEmitter_Silenced.et`

## Attempts already made

User confirmed the ACP plays in Audio Editor, so the audio graph itself is not treated as the primary failure.

## Evidence gathered

- `Sounds/Weapons/Rifles/BS5/Weapons_Silinced_EchoMaster.acp.acp.meta` registers the current resource as `{064282CFD84AFC4E}Sounds/Weapons/Rifles/BS5/Weapons_Silinced_EchoMaster.acp.acp`.
- `Prefabs/Props/BS5_TailEmitter_Silenced.et` still referenced the stale `{5B784125E2E54AA0}...Weapons_Silinced_EchoMaster.acp`.
- `Prefabs/Weapons/Core/Weapon_Base.et` had a correct `BS5_EchoDriverComponent.m_sSuppressedMasterAcp`, but its legacy `BS5_WeaponEchoSettingsComponent.m_SuppressedEchoProject` still referenced the same stale `{5B...}.acp`.
- `rg` found no remaining `{5B784125E2E54AA0}` or quoted `Weapons_Silinced_EchoMaster.acp` references after the fix.

## Assumptions challenged

This is not evidence that the ACP graph is broken. The failing path was a stale resource reference loaded by prefab/script playback routing.

It is also not a reason to rewrite the playback stack. The direct suppressed ACP field was already correct; the concrete failing path was the fallback emitter prefab resource.

## MCP/API/BIKI/base-game checks

- MCP `prefab inspect` confirmed `BS5_TailEmitter_Silenced.et` had `SoundComponent.Filenames` pointing at the stale `{5B...}.acp` before the fix.
- MCP `prefab inspect` confirmed the merged prefab now points at `{064282CFD84AFC4E}...Weapons_Silinced_EchoMaster.acp.acp`.
- MCP API lookup confirmed `SoundComponent`/`BaseSoundComponent` playback uses events from the component's loaded sound resource, and `AudioSystem.PlayEventInitialize/PlayEvent` take ACP resource names directly.
- Workbench state check succeeded and reported edit mode.

## Subagent evidence

Subagents were not attempted. The failure surface was narrow and local evidence was sufficient.

## Root cause candidate

Resource drift after the suppressed ACP ended up registered as `Weapons_Silinced_EchoMaster.acp.acp` with GUID `{064282CFD84AFC4E}`, while the suppressed emitter prefab and legacy settings component still referenced the older non-existing `{5B784125E2E54AA0}...Weapons_Silinced_EchoMaster.acp`.

## Recovery path

Replace the stale suppressed ACP resource references with the current registered resource name. Do not rename the audio project in this fix because the editor currently plays the registered file and renaming would be a larger asset workflow change.

## Change applied, if any

- Updated `Prefabs/Props/BS5_TailEmitter_Silenced.et` `SoundComponent.Filenames`.
- Updated `Prefabs/Weapons/Core/Weapon_Base.et` `BS5_WeaponEchoSettingsComponent.m_SuppressedEchoProject`.

## Validation result

- `rg` found no remaining stale suppressed `{5B784125E2E54AA0}` references.
- `git diff --check` passed with only existing line-ending normalization warnings from Git.
- MCP `prefab inspect` now shows the corrected suppressed ACP in the merged emitter prefab.
- Script reload was not run because no script files changed.

## Follow-up / handoff

The same resource-drift pattern appears to exist for `Weapons_MG_EchoMaster.acp.acp` versus references to `Weapons_MG_EchoMaster.acp`. That was not changed in this suppressed-tail fix.
