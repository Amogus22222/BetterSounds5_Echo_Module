# H-01 Parallel Research Prompt

Use this prompt with the external antigravity agent.

```text
You are acting as a senior performance/reliability auditor for an Arma Reforger / Enfusion / EnforceScript mod.

Repository root:
G:\GitNew\BetterSounds5_Echo_Module

Your task is a parallel research-only assignment for finding H-01 from the audit review:
"GC pressure / hot-path allocations".

Important constraints:
- Do not assume the original forensic audit is correct by default.
- Re-check claims against the actual code and real engine assumptions.
- Do not blindly recommend pooling.
- Do not propose invasive rewrites unless measurement would clearly justify them.
- Treat engine behavior as unknown unless you can support it from code, docs, or direct runtime observation.
- Prefer a measurement-first plan over speculative optimization.
- You have access to the repo and Enfusion MCP / Workbench tooling.

Current local status in the repo:
- Safe and guarded audit fixes are already implemented and compile-clean.
- Wave 3 risky work is intentionally deferred.
- H-01 is still investigation-only.

Relevant files to inspect first:
- Scripts/Game/BS5_EchoRuntime.c
- Scripts/Game/BS5_EnvironmentAudioClassifier.c
- Scripts/Game/BS5_EchoTypes.c
- docs/audits/CODEX_AUDIT_REVIEW.md
- docs/audits/CODEX_FIX_IMPLEMENTATION_PLAN.md
- docs/audits/CODEX_TASK_BREAKDOWN.md
- docs/audits/CODEX_OPEN_QUESTIONS.md

Focus:
1. Identify real hot-path allocations that can plausibly matter at runtime.
2. Separate:
   - allocations likely negligible,
   - allocations likely meaningful,
   - allocations that are downstream of larger costs such as traces/queries/spawn/audio.
3. Reassess whether pooling/preallocation is actually a good fit for this codebase.
4. Design a practical profiling and measurement plan for this repo and engine workflow.
5. Produce a go/no-go decision framework for any future H-01 optimization work.

Specific questions to answer:

1. Allocation inventory
- Which arrays/objects/strings are allocated in the most frequently executed paths?
- Which of these are per-shot, per-candidate, per-query, per-emission, or per-debug-only?
- Which ones are already guarded well enough?

2. Cost ranking
- Rank the likely contributors by practical significance, not theoretical purity.
- Distinguish "obvious but cheap" from "likely user-visible under stress".
- Explain whether traces, entity queries, audio spawning, or string work likely dominate before GC does.

3. Risk of optimization
- For each meaningful candidate, explain the risk of:
  - object pooling,
  - scratch-buffer reuse,
  - static reusable arrays,
  - precomputed/cached strings,
  - restructuring debug logging,
  - reducing candidate cloning/copying.
- Call out where pooling would introduce lifecycle bugs, stale state, ownership confusion, or harder debugging.

4. Measurement plan
- Propose an explicit profiling protocol for this mod.
- Include:
  - test scenarios,
  - weapon types / shot cadence patterns,
  - indoor/outdoor / urban / trench / slapback-heavy cases,
  - MG burst case,
  - delayed tail case,
  - debug on/off comparison,
  - what telemetry to collect,
  - how many repetitions are enough,
  - what counts as a meaningful regression/improvement.
- If Workbench / Enfusion tooling has limitations, say so and provide realistic fallback methods.

5. Recommendation output
- Give a final recommendation bucket for each candidate:
  - DO NOT TOUCH
  - MEASURE FIRST
  - SAFE PREP ONLY
  - PROBABLY WORTH OPTIMIZING
- If you recommend any prep work, keep it narrow and low-risk.

Deliverable format:

Produce a markdown report with these sections:
- Executive summary
- Verified H-01 findings
- Allocation inventory
- Ranked cost assessment
- Risk assessment for each optimization family
- Measurement protocol
- Go / no-go criteria
- Recommended next tasks
- Open questions / assumptions

Additional requirements:
- Be explicit about what is fact, what is inference, and what is unknown.
- If you cite engine behavior, say how you know it.
- Do not recommend any code change just because it is "cleaner".
- Do not expand scope into M-06 / M-07 / other audit findings except where they materially affect H-01 measurement.
- Prefer conclusions that a conservative maintainer could actually trust.
```
