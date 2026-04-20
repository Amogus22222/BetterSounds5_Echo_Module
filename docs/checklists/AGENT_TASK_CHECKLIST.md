# Agent Task Checklist

Use this for each Codex or Antigravity task.

## Task Intake

- [ ] Read task request.
- [ ] Identify owner: Codex, Antigravity, or human.
- [ ] Identify base branch: `main` or `integration`.
- [ ] Identify branch name: `task/<short-name>`, `fix/<short-name>`, `spike/<short-name>`, `review/<short-name>`, or `docs/<short-name>`.
- [ ] Identify file ownership zones.
- [ ] Check if another agent is working in same files.
- [ ] Read relevant docs:
  - [ ] `MODPLAN.md`
  - [ ] `MODPLAN_NEW.md`
  - [ ] `TaskDescription.md`
  - [ ] `TECH_HINTS.md`
  - [ ] Workbench crash notes when touching scripts or prefabs.

## Pre-Change

- [ ] Confirm branch:

```powershell
git status --short --branch
```

- [ ] Confirm remote refs:

```powershell
git fetch origin --prune
```

- [ ] Confirm worktree:

```powershell
git worktree list
```

- [ ] Confirm no unrelated dirty files.
- [ ] Summarize repo state and plan before edits.
- [ ] For code or asset changes, state which files are owned by this task.

## Change Rules

- [ ] Prefer additive changes.
- [ ] Keep one task branch per task.
- [ ] Do not edit same high-conflict file as another active agent.
- [ ] Do not manually edit `resourceDatabase.rdb`.
- [ ] Do not make Workbench-generated asset changes without explaining why.
- [ ] Update docs when behavior or workflow changes.
- [ ] Keep PR small enough to review.

## Verification

- [ ] Run repo-local docs validation when workflow docs change:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
```

- [ ] For script changes, run available Workbench script validation or document why unavailable.
- [ ] For prefab changes, open/validate in Workbench when available or document why unavailable.
- [ ] For audio graph/resource changes, verify resource refs and event names.
- [ ] For config/preset changes, verify matching fallback values in `Scripts/Game/BS5_PresetRegistry.c` when relevant.
- [ ] Check final diff:

```powershell
git diff --stat
git diff --check
```

## Pre-PR

- [ ] Rebase or merge latest actual base. If the task branch was created from `main`, use:

```powershell
git fetch origin --prune
git rebase origin/main
```

- [ ] If the task branch was created from `integration`, use:

```powershell
git fetch origin --prune
git rebase origin/integration
```

- [ ] Confirm changed files:

```powershell
git status --short
git diff --name-only origin/main...HEAD
```

- [ ] If targeting `integration`, compare against `origin/integration` instead:

```powershell
git status --short
git diff --name-only origin/integration...HEAD
```

- [ ] Fill PR template.
- [ ] Include changed-files summary.
- [ ] Include verification commands and results.
- [ ] Include known risks.
- [ ] Request review from the other agent or human.

## Merge/Update

- [ ] After merge, update control checkout:

```powershell
git switch main
git pull --ff-only origin main
```

- [ ] Remove merged task worktree:

First confirm the target worktree is clean:

```powershell
git -C ../BetterSounds5_Echo_Module-codex status --short
```

If it is dirty, preserve or intentionally discard the work before cleanup.

```powershell
git worktree remove ../BetterSounds5_Echo_Module-codex
git branch -d task/<short-name>
git worktree prune
```

- [ ] If using `integration`, merge validated `integration` to `main` by PR.
- [ ] Delete remote task branch after merge if project policy allows.
