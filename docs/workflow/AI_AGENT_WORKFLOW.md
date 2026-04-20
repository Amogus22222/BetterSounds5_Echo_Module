# AI Agent Workflow

This repo supports parallel work by two AI agents:

- Codex: implementation, repo-local edits, verification notes, small PRs.
- Antigravity/Gemini: parallel implementation when assigned a separate area, architecture review, consistency review, edge-case audit.

Default model: `main` is stable, real work happens on short-lived branches, and each agent works from its own local worktree.

## Current Repo Facts

- Repo is an Arma Reforger mod.
- Main source and asset areas:
  - `Scripts/Game/`
  - `Configs/BS5/Presets/`
  - `Prefabs/`
  - `Sounds/`
  - `Assets/`
- Workbench/MCP helper scripts currently exist in `Scripts/WorkbenchGame/EnfusionMCP/`.
- Top-level technical docs already exist and should be preserved:
  - `MODPLAN.md`
  - `MODPLAN_NEW.md`
  - `TaskDescription.md`
  - `TECH_HINTS.md`
  - Workbench crash notes

## Branching Model

Use this default:

```powershell
main                 # stable branch
integration          # optional staging branch for multi-agent/risky work
task/<short-name>    # normal work
fix/<short-name>     # bug fix
spike/<short-name>   # experiment, not automatically mergeable
review/<short-name>  # reviewer patch branch if needed
docs/<short-name>    # documentation-only work
```

Permanent `codex` and `antigravity` branches are not primary work lanes. If they exist, treat them as legacy parking branches until a human deletes or archives them. Do not accumulate features there.

## When To Use `integration`

Use `integration` when any of these are true:

- Codex and Antigravity both have active task branches touching runtime behavior.
- Changes touch shared contracts across `Scripts/Game/`, `Prefabs/`, `Configs/`, and `Sounds/`.
- Workbench validation must happen on a combined set before `main`.
- Two PRs are individually valid but need combined tuning.

Skip `integration` for small isolated docs fixes, narrow script fixes, or one-agent changes with low conflict risk. In those cases use `task/*` or `fix/*` into `main`.

Create `integration` if needed:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
git switch -c integration
git push -u origin integration
```

If `integration` already exists remotely:

```powershell
git fetch origin --prune
git switch --track origin/integration
```

## Parallel Worktree Model

Use separate local directories so agents do not share one working tree:

```text
G:/GitNew/BetterSounds5_Echo_Module              # stable/main control checkout
G:/GitNew/BetterSounds5_Echo_Module-codex        # Codex task worktree
G:/GitNew/BetterSounds5_Echo_Module-antigravity  # Antigravity task worktree
```

Example setup from the main checkout:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main

git worktree add ../BetterSounds5_Echo_Module-codex -b task/codex-audio-cache main
git worktree add ../BetterSounds5_Echo_Module-antigravity -b task/ag-prefab-audit main
```

If using `integration` as base:

```powershell
git fetch origin --prune
git worktree add ../BetterSounds5_Echo_Module-codex -b task/codex-audio-cache origin/integration
git worktree add ../BetterSounds5_Echo_Module-antigravity -b task/ag-prefab-audit origin/integration
```

See [GIT_WORKTREE_SETUP.md](GIT_WORKTREE_SETUP.md) for cleanup and recovery commands.

## File Ownership Rules

Assign one writer per file area where possible.

High-conflict zones:

- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Configs/BS5/Presets/*.conf`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Props/*.et`
- `Sounds/**/*.acp`
- `resourceDatabase.rdb`
- `Scripts/WorkbenchGame/EnfusionMCP/`

Rules:

- Do not let both agents edit same `.c`, `.et`, `.conf`, `.acp`, or `.rdb` file in parallel.
- If a task requires multiple zones, declare ownership in the task brief.
- If a Workbench-generated file changes unexpectedly, document why and include it in review notes.
- Treat `resourceDatabase.rdb` as Workbench-managed. Avoid manual edits. Review any change carefully.
- Before publishing the mod, remove temporary MCP handlers if required by release process.

## Agent Roles

Codex should usually own:

- Local repo edits.
- Small refactors.
- Script and docs changes.
- Mechanical consistency updates.
- Running repo-local validation.

Antigravity/Gemini should usually own:

- Architecture review.
- Cross-file consistency audit.
- Risk and edge-case review.
- Alternative implementation review.
- Separate implementation only when assigned non-overlapping files.

Either agent may implement when assigned a specific branch and file area.

## Compete Vs Review

Use competition only for uncertain design choices. In that case:

1. Each agent works on a separate `spike/*` branch.
2. A human or reviewer picks one path.
3. Losing spike is closed or archived.
4. Chosen path is reworked into a small `task/*` branch.

Use review for normal development:

1. Agent A authors `task/*`.
2. Agent B reviews the PR using [PR_REVIEW_CHECKLIST.md](../checklists/PR_REVIEW_CHECKLIST.md).
3. Agent A fixes review feedback on same branch.
4. PR merges to `integration` or `main`.

## Sync Points

Before starting:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
```

During work:

```powershell
git fetch origin --prune
git rebase origin/main
```

If using `integration`:

```powershell
git fetch origin --prune
git rebase origin/integration
```

Before PR:

```powershell
git status --short
git diff --stat origin/main...HEAD
git log --oneline origin/main..HEAD
```

## PR Flow

Small isolated work:

```text
task/* -> main
```

Parallel or risky work:

```text
task/* -> integration -> main
```

PRs must include:

- Summary.
- Changed files and ownership zones.
- Verification performed.
- Workbench validation status or reason it was not run.
- Known risks.
- Reviewer request for Codex or Antigravity.

Use [.github/pull_request_template.md](../../.github/pull_request_template.md).

## Documentation-First Operation

Before changing code or assets:

1. Read task-relevant docs in repo root and `docs/`.
2. Check `.codex/AGENTS.md`.
3. Confirm branch and worktree.
4. State file ownership.
5. Make small changes.
6. Update docs if behavior or workflow changes.

## Validation

Run repo-local workflow validation:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
```

For gameplay or Workbench changes, also perform Workbench startup and script/prefab validation when available. If unavailable, state that explicitly in PR notes.
