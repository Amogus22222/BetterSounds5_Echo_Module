# PR Review Checklist

Use this when one agent reviews the other.

## Scope

- [ ] PR title matches branch purpose.
- [ ] PR targets correct branch: `main` for isolated work, `integration` for staged multi-agent work.
- [ ] Changed files match stated ownership zone.
- [ ] No unrelated formatting churn or generated-file noise.
- [ ] No silent edits to `resourceDatabase.rdb`.

## Architecture

- [ ] Runtime contract remains clear between driver, runtime, classifier, emitters, presets, and prefabs.
- [ ] No new hidden global state without lifecycle/reset plan.
- [ ] Caches have invalidation or bounded growth.
- [ ] Fallback behavior is explicit and safe.
- [ ] Failure paths log enough for tuning without spamming.

## Arma Reforger / Enfusion

- [ ] EnforceScript syntax matches project patterns.
- [ ] Workbench startup crash notes were considered for large script edits.
- [ ] No risky top-level allocation pattern introduced without Workbench validation.
- [ ] Prefab resource refs use valid GUID/path format.
- [ ] Script component names match prefab component usage.
- [ ] Network/replication assumptions are explicit when state crosses client/server boundary.

## Audio Behavior

- [ ] Tail/slapback/suppressed paths preserve existing event contract.
- [ ] ACP event names and signal names remain synchronized.
- [ ] Emitter prefab names match config and script references.
- [ ] Distance, lifetime, and intensity tuning changes are intentional.
- [ ] Open/field/forest/city/indoor behavior is not accidentally collapsed into one profile.

## Config And Presets

- [ ] `Configs/BS5/Presets/*.conf` and fallback preset values stay synchronized when relevant.
- [ ] Default/light/dynamic technical presets are reviewed together.
- [ ] Values changed in `Weapon_Base.et` match intended runtime defaults.

## Regression Risk

- [ ] Automatic-fire scalability considered.
- [ ] Suppressed and unsuppressed weapons both considered.
- [ ] Explosion bridge still considered if runtime entry points changed.
- [ ] Workbench MCP handler files not changed unless task explicitly owns them.
- [ ] Docs updated if workflow, behavior, or validation process changed.

## Verification

- [ ] Author listed exact commands run.
- [ ] Reviewer can reproduce repo-local checks.
- [ ] Workbench validation result is present or skipped with reason.
- [ ] Screens/logs are referenced when live gameplay behavior was tested.
- [ ] Known risks are specific enough for next task.

## Review Outcome

Use one:

```text
Approve: no blocking issues.
Request changes: blocking issue(s) listed with file/line and required fix.
Comment only: non-blocking suggestions.
Needs human: reviewer cannot verify Workbench/GitHub/external setting.
```
