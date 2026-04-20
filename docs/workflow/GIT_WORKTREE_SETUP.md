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

The helper creates local worktrees and task branches only. It does not create the remote/staging `integration` branch; create `integration` first only when staged multi-agent work actually needs it.

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Setup-AgentWorktrees.ps1 `
  -Base main `
  -CodexBranch task/codex-audio-cache `
  -AntigravityBranch task/ag-prefab-audit
```

If `integration` already exists locally or at `origin/integration`:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Setup-AgentWorktrees.ps1 `
  -Base integration `
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

If based on `integration`, pass the integration upstream explicitly:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Sync-TaskBranch.ps1 -Upstream origin/integration
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

`git worktree remove` works only when the target worktree is clean. Check inside the task worktree first:

```powershell
git -C ../BetterSounds5_Echo_Module-codex status --short
```

If it prints changes, do not remove the worktree silently. Commit the work, stash it with a clear message, move it to a new task branch, or ask the human maintainer what to preserve.

When the task worktree is clean and the PR is merged, run from the control checkout:

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

If a worktree folder was moved rather than deleted, repair Git's worktree links from the control checkout:

```powershell
git worktree repair ../BetterSounds5_Echo_Module-codex
git worktree list
```

If Git says a branch is already checked out in a worktree, inspect first:

```powershell
git worktree list --porcelain
```

If the listed path no longer exists, run `git worktree prune`. If the listed path exists, check `git -C <path> status --short` and preserve any work before cleanup.

If a worktree was intentionally locked with `git worktree lock`, use `git worktree unlock <path>` only after confirming the lock reason is obsolete.

Use `git worktree remove --force <path>` only after confirming the target worktree has no work that needs to be kept, or after the work was intentionally saved elsewhere.

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
