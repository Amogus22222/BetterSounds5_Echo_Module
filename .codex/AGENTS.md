# Repo Agent Instructions

This repo uses documentation-first parallel AI-agent workflow.

## Core Rules

- `main` is stable.
- `integration` is optional staging for multi-agent or risky combined work.
- Use one short-lived branch per task:
  - `task/<short-name>`
  - `fix/<short-name>`
  - `spike/<short-name>`
  - `review/<short-name>`
  - `docs/<short-name>`
- Do not use long-lived `codex` or `antigravity` branches for feature accumulation.
- Use separate local worktrees for parallel execution.
- Prefer additive changes.
- Preserve user changes.
- Document verification steps.
- Summarize touched files and risks.
- If external GitHub settings cannot be enforced from repo, document them instead of claiming they are done.

## Before Editing

Run or inspect:

```powershell
git status --short --branch
git branch --all --verbose --no-abbrev
git worktree list
```

Read:

- `docs/workflow/AI_AGENT_WORKFLOW.md`
- `docs/workflow/BRANCHING_STRATEGY.md`
- `docs/checklists/AGENT_TASK_CHECKLIST.md`
- Task-relevant project docs in repo root.

State:

- Current branch.
- Dirty files.
- Planned branch/base.
- File ownership zone.
- Verification plan.

## During Work

- Keep changes small.
- Do not let two agents edit same high-conflict file in parallel.
- Treat `Scripts/Game/`, `Prefabs/`, `Configs/BS5/Presets/`, `Sounds/**/*.acp`, and `resourceDatabase.rdb` as review-sensitive.
- Do not manually edit `resourceDatabase.rdb`.
- Do not touch `Scripts/WorkbenchGame/EnfusionMCP/` unless task explicitly owns Workbench MCP tooling.

## Before Commit Or PR

Run:

```powershell
git status --short
git diff --stat
git diff --check
```

If workflow docs changed:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
```

For Arma changes, run Workbench validation when available. If not available, record exact reason.

## Final Report

Include:

- What changed.
- Changed files.
- Verification.
- Risks.
- Human actions needed.
