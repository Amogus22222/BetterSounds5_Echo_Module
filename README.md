# BetterSounds5_Echo_Module

BetterSounds5 Echo Module is an Arma Reforger mod project for dynamic weapon echo, tail, slapback, and environment-aware audio behavior.

## Project Docs

- Current implementation plan: [MODPLAN.md](MODPLAN.md)
- Self-contained technical spec: [MODPLAN_NEW.md](MODPLAN_NEW.md)
- Functional task description: [TaskDescription.md](TaskDescription.md)
- Workbench stability notes: [TECH_HINTS.md](TECH_HINTS.md)

## AI Agent Workflow

Codex and Antigravity/Gemini work in this repo through short-lived task branches and separate local `git worktree` directories. `main` stays stable. Use `integration` only when several agent branches need staging before `main`.

Start here:

- [AI agent workflow](docs/workflow/AI_AGENT_WORKFLOW.md)
- [Branching strategy](docs/workflow/BRANCHING_STRATEGY.md)
- [Git worktree setup](docs/workflow/GIT_WORKTREE_SETUP.md)
- [Agent task checklist](docs/checklists/AGENT_TASK_CHECKLIST.md)
- [PR review checklist](docs/checklists/PR_REVIEW_CHECKLIST.md)
- [Manual GitHub actions required](docs/workflow/HUMAN_ACTIONS_REQUIRED.md)

Repo-local agent instructions live in [.codex/AGENTS.md](.codex/AGENTS.md).
