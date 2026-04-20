# Handoff Protocol

Use this when one agent hands work to another agent or human.

## Required Handoff Summary

```markdown
## Handoff Summary

Owner: Codex | Antigravity | Human
Branch: task/<short-name>
Base: main | integration
Worktree: <local path>
Task: <one sentence>

## Changed Files

- <path>: <what changed and why>

## Files Intentionally Not Touched

- <path>: <reason>

## Verification

- Command: `<command>`
  Result: <pass/fail/skipped and reason>

## Risks

- <specific risk or "None known">

## Reviewer Request

- <what the next agent should check>

## Next Action

- <merge/review/test/fix decision>
```

## Changed-Files Summary Rules

- List every changed file.
- Mention generated Workbench files separately.
- Mention binary/audio changes separately.
- If `resourceDatabase.rdb` changed, explain exactly why.
- If Workbench validation was not run, say why.

## Review Handoff

Reviewer response format:

```markdown
## Review Result

Outcome: Approve | Request changes | Comment only | Needs human

## Findings

- <file>:<line> - <problem> - <required fix>

## Verification Checked

- <what reviewer reproduced or inspected>

## Residual Risk

- <risk or "None known">
```

## Conflict Handoff

If conflict occurs:

```markdown
## Conflict Notice

Branch: task/<short-name>
Conflicting branch: task/<other-short-name>
Files:
- <path>

Reason:
- <why overlap happened>

Proposed owner:
- <agent/human>

Recommended fix:
- <rebase/merge/manual split/abandon one branch>
```
