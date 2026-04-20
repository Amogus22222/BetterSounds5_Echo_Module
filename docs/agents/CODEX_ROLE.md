# Codex Role

Codex is the primary repo-local implementation agent.

## Operating Mode

- Inspect before editing.
- Summarize current repo state and plan before file edits.
- Prefer small, additive changes.
- Keep one task branch per task.
- Use separate worktree for parallel work.
- Do not use long-lived `codex` branch for feature accumulation.
- Preserve user changes and unrelated dirty files.
- Document verification steps.

## Branch Rules

Start from `main` unless task owner chooses an existing `integration` branch for staged combined validation:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
git switch -c task/<short-name>
```

Use helper:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Start-TaskBranch.ps1 -Type task -Name <short-name> -Base main
```

For an `integration`-based task, use `-Base integration` and sync with `origin/integration`:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Start-TaskBranch.ps1 -Type task -Name <short-name> -Base integration
powershell -ExecutionPolicy Bypass -File tools/workflow/Sync-TaskBranch.ps1 -Upstream origin/integration
```

Allowed branches:

- `task/*`
- `fix/*`
- `spike/*`
- `review/*`
- `docs/*`

Do not create or use `codex` as a normal work branch.

## Expected Behavior

Codex should:

- Read relevant project docs before code edits.
- Identify file ownership zones.
- Avoid parallel edits in high-conflict files.
- Use project patterns over new abstractions.
- Keep docs synchronized with changed behavior.
- Report risk and uncertainty plainly.
- Produce changed-files summary after edits.
- Produce final audit when workflow/docs are changed.

## Commit And PR Expectations

Before PR:

```powershell
git status --short
git diff --stat
git diff --check
```

Compare against the actual PR target: `origin/main` for direct work, or `origin/integration` for staged multi-agent work.

PR must include:

- Summary.
- Changed files.
- Verification.
- Workbench validation status.
- Risk.
- Reviewer requested.

## Validation Responsibility

For workflow docs:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
```

For Arma Reforger changes:

- Run Workbench validation when available.
- If Workbench or MCP validation is unavailable, state exact blocker.
- Do not claim external GitHub settings were configured unless actually done.
