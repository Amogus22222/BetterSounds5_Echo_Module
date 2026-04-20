# Repo Sync Validation

Use this checklist after workflow docs or helper scripts change.

## Required Files

These files must exist:

- `README.md`
- `docs/workflow/AI_AGENT_WORKFLOW.md`
- `docs/workflow/GIT_WORKTREE_SETUP.md`
- `docs/workflow/BRANCHING_STRATEGY.md`
- `docs/workflow/REPO_SYNC_VALIDATION.md`
- `docs/workflow/HUMAN_ACTIONS_REQUIRED.md`
- `docs/workflow/FINAL_AUDIT_REPORT.md`
- `docs/checklists/AGENT_TASK_CHECKLIST.md`
- `docs/checklists/PR_REVIEW_CHECKLIST.md`
- `docs/agents/CODEX_ROLE.md`
- `docs/agents/ANTIGRAVITY_ROLE.md`
- `docs/agents/HANDOFF_PROTOCOL.md`
- `.codex/AGENTS.md`
- `.codex/SKILLS.md`
- `.codex/TASK_TEMPLATE.md`
- `.github/pull_request_template.md`
- `tools/workflow/Start-TaskBranch.ps1`
- `tools/workflow/Setup-AgentWorktrees.ps1`
- `tools/workflow/Sync-TaskBranch.ps1`
- `tools/workflow/Validate-WorkflowDocs.ps1`

## Branch Model Consistency

- [ ] `main` is documented as stable branch.
- [ ] `integration` is documented as optional staging branch.
- [ ] `task/*`, `fix/*`, `spike/*`, `review/*`, and `docs/*` are documented as short-lived branches.
- [ ] `codex` and `antigravity` are not documented as primary development lanes.
- [ ] Worktree examples use `task/*` branches.
- [ ] PR flow is documented as `task/* -> main` or `task/* -> integration -> main`.

## Worktree Consistency

- [ ] Docs use `G:/GitNew/BetterSounds5_Echo_Module` as current main checkout example.
- [ ] Codex worktree path example ends with `-codex`.
- [ ] Antigravity worktree path example ends with `-antigravity`.
- [ ] Cleanup commands include `git worktree remove` and `git worktree prune`.

## Human-Only Settings

- [ ] GitHub branch protection is listed in `HUMAN_ACTIONS_REQUIRED.md`.
- [ ] Required reviews are listed as external GitHub settings.
- [ ] Required status checks are listed as external GitHub settings.
- [ ] Merge queue/linear history are documented as manual decisions.
- [ ] Docs do not claim GitHub settings are already configured.

## Validation Commands

Run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1
git status --short
git diff --check
```

Expected:

- Workflow doc validator exits `0`.
- Local markdown links in workflow docs resolve.
- Only intended docs/tool files are changed.
- `git diff --check` reports no whitespace errors.
