[CmdletBinding()]
param(
    [ValidateSet('main', 'integration')]
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
    param(
        [string]$Branch,
        [string]$RemoteName
    )

    & git show-ref --verify --quiet "refs/heads/$Branch"
    if ($LASTEXITCODE -eq 0) {
        throw "Local branch already exists: $Branch"
    }

    & git show-ref --verify --quiet "refs/remotes/$RemoteName/$Branch"
    if ($LASTEXITCODE -eq 0) {
        throw "Remote branch already exists: $RemoteName/$Branch"
    }
}

function Assert-WorkflowBranchName {
    param([string]$Branch)
    if ($Branch -notmatch '^(task|fix|spike|review|docs)/[a-z0-9][a-z0-9._-]*$') {
        throw "Branch '$Branch' must match one of: task/<short-name>, fix/<short-name>, spike/<short-name>, review/<short-name>, docs/<short-name>."
    }
}

function Assert-PathFree {
    param([string]$Path)
    if (Test-Path -LiteralPath $Path) {
        throw "Path already exists: $Path"
    }
}

function Test-GitRef {
    param([string]$Ref)
    & git rev-parse --verify --quiet "$Ref^{commit}" *> $null
    return $LASTEXITCODE -eq 0
}

function Select-BaseBranch {
    param(
        [string]$Branch,
        [string]$RemoteName
    )

    if (Test-GitRef "refs/heads/$Branch") {
        Invoke-Git switch $Branch
    } elseif (Test-GitRef "refs/remotes/$RemoteName/$Branch") {
        Invoke-Git switch --track "$RemoteName/$Branch"
    } else {
        throw "Base branch '$Branch' does not exist locally or at $RemoteName/$Branch. Create 'integration' only when staged multi-agent work needs it."
    }

    Invoke-Git pull --ff-only $RemoteName $Branch
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

Assert-WorkflowBranchName $CodexBranch
Assert-WorkflowBranchName $AntigravityBranch
Assert-PathFree $CodexPath
Assert-PathFree $AntigravityPath

Invoke-Git fetch $Remote --prune
Assert-BranchFree -Branch $CodexBranch -RemoteName $Remote
Assert-BranchFree -Branch $AntigravityBranch -RemoteName $Remote
Select-BaseBranch -Branch $Base -RemoteName $Remote

Invoke-Git worktree add $CodexPath -b $CodexBranch $Base
Invoke-Git worktree add $AntigravityPath -b $AntigravityBranch $Base

Write-Host "Created Codex worktree: $CodexPath ($CodexBranch)"
Write-Host "Created Antigravity worktree: $AntigravityPath ($AntigravityBranch)"
