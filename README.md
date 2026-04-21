# BetterSounds5 Echo Module

Event-driven weapon echo, tail, and slapback system for Arma Reforger / Enfusion.

## Current State

- Core BS5 echo/tail runtime is implemented and compile-validated in Workbench.
- Audit-driven safety fixes from Waves 0-2 are already in place.
- `H-01` was measured and is currently deferred: no pooling/perf rewrite is justified now.
- Next major workstream is targeted slapback expansion for specific scenarios.
- MCP bridge under `Scripts/WorkbenchGame/EnfusionMCP/` stays in the repo and is intentionally retained for local tooling.

## Key Paths

- Runtime driver: `Scripts/Game/BS5_EchoDriverComponent.c`
- Runtime analysis/emission: `Scripts/Game/BS5_EchoRuntime.c`
- Environment classification: `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- Shared runtime types: `Scripts/Game/BS5_EchoTypes.c`
- Weapon tuning entry point: `Prefabs/Weapons/Core/Weapon_Base.et`

## Documentation Map

- Active roadmap: [MODPLAN.md](MODPLAN.md)
- Project context: [PROJECT_CONTEXT.md](docs/reference/PROJECT_CONTEXT.md)
- Workbench and MCP notes: [WORKBENCH_AND_MCP.md](docs/reference/WORKBENCH_AND_MCP.md)
- Slapback expansion plan: [SLAPBACK_EXPANSION_PLAN.md](docs/plans/SLAPBACK_EXPANSION_PLAN.md)
- Audit review and implementation backlog: [docs/audits](docs/audits)

## Working Rules

- Do not blindly rewrite acoustic tuning, spawn semantics, or trace filters.
- Treat `Weapon_Base.et`, BS5 ACP graphs, and runtime signal contracts as coupled systems.
- Prefer narrow runtime-safe changes over structural rewrites.
- Validate substantial script changes in real Workbench startup, not only via static validation.
