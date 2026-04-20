# Human Actions Required

These settings cannot be guaranteed by repo-local file changes alone. Configure them in GitHub repository settings if wanted.

Repository:

```text
https://github.com/Amogus22222/BetterSounds5_Echo_Module
```

## Branch Protection

Recommended protected branches:

- `main`
- `integration` if used

Recommended `main` protection:

- Require pull request before merging.
- Require at least 1 approving review.
- Require conversation resolution before merge.
- Require linear history if project prefers rebase/squash merges.
- Require status checks once CI exists.
- Restrict force pushes.
- Restrict deletion.

Recommended `integration` protection:

- Require pull request before merging.
- Require at least 1 approving review for multi-agent work.
- Allow faster iteration than `main` if humans prefer.
- Restrict force pushes.

## Required Reviews

Suggested rule:

- Codex-authored PRs require Antigravity or human review.
- Antigravity-authored PRs require Codex or human review.
- Changes to `Scripts/Game/`, `Prefabs/`, `Configs/BS5/Presets/`, `Sounds/**/*.acp`, or `resourceDatabase.rdb` require human review if Workbench validation was not run.

## Required Status Checks

No CI workflow exists in repo at time of this workflow setup.

When CI is added, consider requiring:

- Markdown/link/doc validation.
- PowerShell helper syntax check.
- EnforceScript/Workbench validation if automation is available.
- Asset/reference validation if practical.

## Merge Strategy

Recommended:

- Squash merge for small task branches.
- Rebase merge only when commit history is clean and meaningful.
- Merge commit into `integration` is acceptable for staging combined work.
- No direct pushes to `main` except emergency maintainer action.

## Merge Queue

Optional. Use merge queue only if PR volume grows or concurrent merges create repeated breakage.

## Remote Branch Cleanup

Remote `codex` and `antigravity` currently may exist. Do not delete from repo-local automation.

Human options:

1. Leave them as archived/legacy branches.
2. Protect them from pushes.
3. Delete them after confirming no external tool depends on them.

Check first:

```powershell
git fetch origin --prune
git log --oneline --left-right --cherry-pick origin/main...origin/codex
git log --oneline --left-right --cherry-pick origin/main...origin/antigravity
```

Delete only by explicit maintainer decision:

```powershell
git push origin --delete codex
git push origin --delete antigravity
```
