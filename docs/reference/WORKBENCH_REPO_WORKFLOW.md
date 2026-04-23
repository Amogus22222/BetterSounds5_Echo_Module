# Workbench Repo Workflow

## Canonical Rule

`enfusion-mcp` and local Reforger Workbench should always target one canonical addon path:

- `G:\BettersMods\BetterSounds5`

That path should not be treated as an independently edited copy.
It should be a junction pointing at the currently active git repo worktree.

## Why

Without a canonical path, three states drift apart:

1. active git repo under `G:\GitNew\...`
2. MCP-configured project root under `G:\BettersMods`
3. whatever Workbench last opened under `Documents\My Games\ArmaReforgerWorkbench\addons`

That drift causes:

- MCP validating/building the wrong tree
- Workbench opening stale files
- manual edits landing in a non-versioned local copy
- branch/version confusion between “main local mod” and newer git repos

## Supported Workflow

1. Keep development in git worktrees only.
2. Point `G:\BettersMods\BetterSounds5` at the active repo via junction.
3. Let MCP/Workbench always open the canonical path.
4. Switch repos by repointing the junction, not by copying folders around manually.

## Scripts

From any BS5 repo root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\Set-BS5WorkbenchRepo.ps1
```

Check status:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\Get-BS5WorkbenchRepoStatus.ps1
```

Environment sanity check:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\Test-BS5WorkbenchEnvironment.ps1
```

## Safety

- If `G:\BettersMods\BetterSounds5` is a real directory, `Set-BS5WorkbenchRepo.ps1` moves it to `G:\BettersMods\_repo_switch_backups\...` before creating the junction.
- The canonical path remains stable for MCP config.
- Git history stays only in the active repo, not in the Workbench root.

## MCP Rule

When using `enfusion-mcp`:

- open/build/validate `G:\BettersMods\BetterSounds5\addon.gproj`
- do not point MCP at ad-hoc repos outside the canonical root unless debugging path issues

## Remaining Environment Risk

If Workbench still reports:

- missing dependency `58D0FB3206B6F859`
- game addon not found

that is a local Workbench/base-game environment issue, not a repo sync issue.
The junction workflow fixes source-of-truth and MCP path consistency; it does not repair a broken base game installation by itself.
