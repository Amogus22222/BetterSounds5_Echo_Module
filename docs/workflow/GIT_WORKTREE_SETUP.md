# Git Worktree Setup

Use `git worktree` so Codex and Antigravity can work at same time without sharing one working directory.

## Recommended Local Layout

```text
G:/GitNew/BetterSounds5_Echo_Module              # main control checkout
G:/GitNew/BetterSounds5_Echo_Module-codex        # Codex worktree
G:/GitNew/BetterSounds5_Echo_Module-antigravity  # Antigravity worktree
```

## Create Worktrees From `main`

Run from `G:/GitNew/BetterSounds5_Echo_Module`:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main

git worktree add ../BetterSounds5_Echo_Module-codex -b task/codex-audio-cache main
git worktree add ../BetterSounds5_Echo_Module-antigravity -b task/ag-prefab-audit main
```

Use short task names. Prefer `task/<owner>-<topic>` when both agents are active.

## Create Worktrees From `integration`

Use this only when the team has created `integration` for staged multi-agent work.

```powershell
git fetch origin --prune
git worktree add ../BetterSounds5_Echo_Module-codex -b task/codex-audio-cache origin/integration
git worktree add ../BetterSounds5_Echo_Module-antigravity -b task/ag-prefab-audit origin/integration
```

## Use Helper Script

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Setup-AgentWorktrees.ps1 `
  -Base main `
  -CodexBranch task/codex-audio-cache `
  -AntigravityBranch task/ag-prefab-audit
```

## List Worktrees

```powershell
git worktree list
git worktree list --porcelain
```

## Keep Task Branch Updated

From inside a task worktree:

```powershell
git fetch origin --prune
git rebase origin/main
```

If based on `integration`:

```powershell
git fetch origin --prune
git rebase origin/integration
```

Helper:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Sync-TaskBranch.ps1 -Upstream origin/main
```

## Push Task Branch

From inside task worktree:

```powershell
git push -u origin HEAD
```

Open PR:

```text
task/<short-name> -> main
```

or for staged multi-agent work:

```text
task/<short-name> -> integration
```

## Cleanup After Merge

From control checkout:

```powershell
git fetch origin --prune
git worktree remove ../BetterSounds5_Echo_Module-codex
git branch -d task/codex-audio-cache
git worktree prune
```

If branch was squash-merged and Git cannot prove it merged:

```powershell
git branch -D task/codex-audio-cache
```

Use `-D` only after confirming PR is merged and no work is needed.

## Recover Missing Or Moved Worktree

If a worktree folder was deleted manually:

```powershell
git worktree prune
git worktree list
```

If branch is locked by a stale worktree:

```powershell
git worktree list --porcelain
git worktree remove --force ../BetterSounds5_Echo_Module-codex
git worktree prune
```

Use `--force` only when the target folder is already gone or all changes were saved.

## Dirty Worktree Rule

Do not rebase, switch branch, remove a worktree, or pull until this is clean or intentionally saved:

```powershell
git status --short
```

Save work before sync:

```powershell
git add <files>
git commit -m "WIP: describe saved work"
```

or stash only if reviewer can tolerate temporary hidden state:

```powershell
git stash push -m "temporary sync stash"
```
