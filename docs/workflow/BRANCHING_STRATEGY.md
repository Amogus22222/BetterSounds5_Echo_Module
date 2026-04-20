# Branching Strategy

## Stable Branch

`main` is stable. It should contain code and assets that have passed review and documented verification.

## Optional Staging Branch

`integration` is optional. Use it when multiple task branches need combined validation before `main`.

Do not create `integration` for routine one-agent work or just to satisfy the workflow. It may be absent until the first task that actually needs staged combined validation.

Create it only when needed:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
git switch -c integration
git push -u origin integration
```

## Short-Lived Work Branches

Allowed branch prefixes:

```text
task/<short-name>
fix/<short-name>
spike/<short-name>
review/<short-name>
docs/<short-name>
```

`<short-name>` should be lowercase and may use letters, numbers, `.`, `_`, or `-`. This matches the repo-local helper scripts.

Examples:

```text
task/codex-emitter-cache
task/ag-prefab-consistency-audit
fix/slapback-event-fallback
spike/sector-cache-distance-tuning
review/codex-audio-cache-notes
docs/workflow-agent-setup
```

## Forbidden As Primary Development Lanes

Do not use these as feature accumulation branches:

```text
codex
antigravity
gemini
agent
dev
workspace
```

Existing `codex` and `antigravity` branches may remain until a human decides what to do. They should not receive new feature work.

## Base Branch Choice

Use `main` as base when:

- Change is isolated.
- Only one agent works on the area.
- Risk is low.
- PR can merge directly after review.

Use `integration` as base when:

- Multiple task branches must be tested together.
- Changes touch shared runtime contracts across scripts, prefabs, configs, or sounds.
- Workbench validation needs a combined branch.

## Branch Lifecycle

Start from `main`:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
git switch -c task/<short-name>
```

Start from `integration` only when staged multi-agent work needs it:

```powershell
git fetch origin --prune
git switch integration
git pull --ff-only origin integration
git switch -c task/<short-name>
```

If `integration` exists only on the remote, run `git switch --track origin/integration` before creating the task branch.

Push:

```powershell
git push -u origin HEAD
```

Update a `main`-based task branch:

```powershell
git fetch origin --prune
git rebase origin/main
```

Update an `integration`-based task branch:

```powershell
git fetch origin --prune
git rebase origin/integration
```

Open PR:

```text
task/<short-name> -> main
```

or:

```text
task/<short-name> -> integration
```

After merge:

```powershell
git switch main
git pull --ff-only origin main
git branch -d task/<short-name>
git push origin --delete task/<short-name>
```

Delete only the completed short-lived task branch. Do not delete `codex`, `antigravity`, or other legacy remote branches without an explicit human maintainer decision.

## Commit Rules

- Keep commits focused.
- Do not mix unrelated script, prefab, audio asset, and workflow changes in one PR unless task requires it.
- Commit generated Workbench files only when they are expected and reviewed.
- Include verification details in PR, not only in commit message.

## Migration For Existing Agent Branches

If `codex` or `antigravity` later diverge:

1. Inspect divergence:

```powershell
git fetch origin --prune
git log --oneline --left-right --cherry-pick main...origin/codex
git log --oneline --left-right --cherry-pick main...origin/antigravity
```

2. Create task branches from useful commits:

```powershell
git switch main
git pull --ff-only origin main
git switch -c task/recover-codex-work
git cherry-pick <commit>
```

3. Open normal PRs from recovered task branches.

4. Ask human maintainer before deleting remote `codex` or `antigravity`.
