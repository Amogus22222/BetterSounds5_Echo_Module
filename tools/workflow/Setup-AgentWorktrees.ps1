[CmdletBinding()]
param(
    [string]$Base = 'main',
    [string]$CodexBranch = 'task/codex-work',
    [string]$AntigravityBranch = 'task/ag-work',
    [string]$CodexPath,
    [string]$AntigravityPath,
    [string]$Remote = 'origin'
)

$ErrorActionPreference = 'Stop'

function Invoke-Git {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Args)
    & git @Args
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Args -join ' ') failed with exit code $LASTEXITCODE"
    }
}

function Assert-BranchFree {
    param([string]$Branch)
    & git show-ref --verify --quiet "refs/heads/$Branch"
    if ($LASTEXITCODE -eq 0) {
        throw "Local branch already exists: $Branch"
    }
}

function Assert-PathFree {
    param([string]$Path)
    if (Test-Path -LiteralPath $Path) {
        throw "Path already exists: $Path"
    }
}

$repo = (& git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or -not $repo) {
    throw 'Not inside a git repository.'
}

Set-Location $repo

$repoName = Split-Path -Leaf $repo
$parent = Split-Path -Parent $repo

if (-not $CodexPath) {
    $CodexPath = Join-Path $parent "$repoName-codex"
}

if (-not $AntigravityPath) {
    $AntigravityPath = Join-Path $parent "$repoName-antigravity"
}

$dirty = (& git status --porcelain)
if ($dirty) {
    throw "Control worktree is dirty. Commit, stash, or move changes before creating parallel worktrees.`n$dirty"
}

Assert-BranchFree $CodexBranch
Assert-BranchFree $AntigravityBranch
Assert-PathFree $CodexPath
Assert-PathFree $AntigravityPath

Invoke-Git fetch $Remote --prune
Invoke-Git switch $Base
Invoke-Git pull --ff-only $Remote $Base

Invoke-Git worktree add $CodexPath -b $CodexBranch $Base
Invoke-Git worktree add $AntigravityPath -b $AntigravityBranch $Base

Write-Host "Created Codex worktree: $CodexPath ($CodexBranch)"
Write-Host "Created Antigravity worktree: $AntigravityPath ($AntigravityBranch)"
