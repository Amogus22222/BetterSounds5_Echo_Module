# Repo-Local Skill Notes

Use these repo-specific notes with normal Codex skills.

## Workflow Skill

When task touches branching, worktrees, PR process, or agent coordination:

1. Read `.codex/AGENTS.md`.
2. Read `docs/workflow/AI_AGENT_WORKFLOW.md`.
3. Read `docs/workflow/BRANCHING_STRATEGY.md`.
4. Read `docs/workflow/GIT_WORKTREE_SETUP.md`.
5. Run `tools/workflow/Validate-WorkflowDocs.ps1` after edits.

## Arma Reforger Mod Skill

When task touches game behavior:

1. Read `MODPLAN.md`.
2. Read `MODPLAN_NEW.md`.
3. Read `TaskDescription.md`.
4. Read `TECH_HINTS.md`.
5. Check Workbench crash notes before large script changes.
6. Keep script, prefab, config, and sound resource refs synchronized.

## High-Conflict File Skill

Before touching these areas, declare ownership and review path:

- `Scripts/Game/*.c`
- `Prefabs/**/*.et`
- `Configs/BS5/Presets/*.conf`
- `Sounds/**/*.acp`
- `resourceDatabase.rdb`
- `Scripts/WorkbenchGame/EnfusionMCP/`

## Verification Skill

Minimum repo checks:

```powershell
git status --short
git diff --stat
git diff --check
```

Workflow docs:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
```

Workbench checks are required for confidence on script/prefab/audio changes when Workbench is available.
