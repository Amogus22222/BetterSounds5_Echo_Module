# CODEX Quick Wins

Prioritized low-risk / high-value fixes:

1. `DONE` Use `FindSignal` for read-only entity signal access (`M-02`)
2. `DONE` Replace `.IndexOf(".et") != -1` with `.EndsWith(".et")` (`L-04`)
3. `DONE` Remove queued driver instance callbacks in `OnDelete` (`L-06`)
4. `DONE` Gate hot emit/slapback debug-string construction behind debug checks (`L-02`)
5. `DONE` Store owner `EntityID` in delayed emission context and resolve on use (`H-03`, guarded)
6. `DONE` Cap invalid emitter resource cache size without changing valid cache behavior (`H-04`)
7. `DONE` Add minimal reentrancy guards to shared slapback / forward-facade query scratch state (`H-02`, guarded)

Probably not worth quick-win treatment:

- `H-01` hot-path allocation rewrites after current timing pass
- Candidate container rewrites (`M-05`, `M-09`)
- Acoustic tuning rewrites (`L-01`)
- Spawn-order changes (`M-06`)
- Trace filter rewrites (`M-07`)
- `ToLower()` rewrite from the external audit (`M-03`)
