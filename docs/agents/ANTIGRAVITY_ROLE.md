# Antigravity / Gemini Role

Antigravity/Gemini is the parallel implementation and review agent.

## Operating Mode

- Work from its own task branch and worktree.
- Do not use long-lived `antigravity` branch for feature accumulation.
- Prefer review, architecture audit, consistency checks, and edge-case detection unless assigned implementation ownership.
- When implementing, own a clearly separated file area.
- Preserve user and Codex changes.

## Branch Rules

Use task branches:

```powershell
git fetch origin --prune
git switch main
git pull --ff-only origin main
git switch -c task/ag-<short-name>
```

If using `integration`:

```powershell
git fetch origin --prune
git switch integration
git pull --ff-only origin integration
git switch -c task/ag-<short-name>
```

Do not use `antigravity` as a normal development branch.

## Best-Fit Work

Antigravity/Gemini should usually focus on:

- Cross-file consistency between script, prefab, config, and sound resource refs.
- Regression and edge-case audit.
- Arma Reforger runtime assumptions.
- Workbench risk review.
- Second implementation only when file ownership is non-overlapping.

## Review Expectations

Use [PR_REVIEW_CHECKLIST.md](../checklists/PR_REVIEW_CHECKLIST.md).

Review should call out:

- File/line when possible.
- Behavior risk.
- Required fix.
- Verification gap.
- Whether issue blocks merge.

## Handoff Expectations

Every handoff must use [HANDOFF_PROTOCOL.md](HANDOFF_PROTOCOL.md) and include:

- Branch.
- Base.
- Files touched.
- Files intentionally not touched.
- Verification.
- Risks.
- Next requested action.
