# Post-Fix Audit

Date: 2026-04-21

Scope: workflow docs, repo-local Codex instructions, GitHub PR template, and `tools/workflow/*.ps1` helper scripts only.

## What Was Not Ideal

- Some pre-PR and sync examples still assumed every task branch should compare or rebase against `origin/main`.
- `integration` was documented as optional, but a few examples did not make clear enough that it should be absent until a real staged-validation need exists.
- Worktree cleanup examples did not state strongly enough that `git worktree remove` requires a clean target worktree.
- Dirty worktree handling, `git worktree prune`, and `git worktree repair` needed clearer troubleshooting boundaries.
- Helper scripts and docs were close, but branch naming validation and `integration` base handling were not fully aligned.
- Manual GitHub settings and repo-local automation were documented, but their boundary needed to be more explicit.

## What Was Fixed

- Clarified that `integration` is created only for real multi-agent or risky combined validation, not as a default branch.
- Clarified that `integration`-based task branches sync, rebase, diff, and review against `origin/integration`.
- Expanded worktree cleanup guidance for clean removal, dirty worktree preservation, prune, repair, and cautious force removal.
- Updated README workflow links so they point only to files that exist in this repo.
- Aligned branch naming rules across docs, PR template, `.codex` notes, and helper scripts.
- Updated helper scripts to validate workflow branch prefixes, reject local/remote task branch name collisions, and use an existing local or remote `integration` base without creating a new remote staging branch.
- Extended workflow validation to require this audit file, check key consistency rules, verify local markdown links, and parse PowerShell helper scripts.
- Updated the historical final audit report where it could be misread as current branch state.

## Manual GitHub Steps Remaining

These are intentionally not done by repo-local scripts:

- Configure or change branch protection for `main`.
- Configure protection for `integration` if that branch is ever created and used.
- Enable required reviews, conversation resolution, status checks, linear history, or merge queue.
- Delete or protect legacy `codex` and `antigravity` remote branches after human review.
- Add CI only after the repo has a real reliable validation command worth enforcing.

## Ready Status

Ready to commit after repo-local validation:

- `powershell -ExecutionPolicy Bypass -File tools/workflow/Validate-WorkflowDocs.ps1` passed.
- `git diff --check` passed; Git printed only line-ending normalization warnings from `.gitattributes`.
- No game code, prefabs, configs, audio assets, or `resourceDatabase.rdb` were changed.
