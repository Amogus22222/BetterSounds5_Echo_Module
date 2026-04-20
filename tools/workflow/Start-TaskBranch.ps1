[CmdletBinding()]
param(
    [ValidateSet('task', 'fix', 'spike', 'review', 'docs')]
    [string]$Type = 'task',

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[a-z0-9][a-z0-9._-]*$')]
    [string]$Name,

    [ValidateSet('main', 'integration')]
    [string]$Base = 'main',
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
        throw "Base branch '$Branch' does not exist locally or at $RemoteName/$Branch. Create 'integration' only when it is actually needed."
    }

    Invoke-Git pull --ff-only $RemoteName $Branch
}

$repo = (& git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or -not $repo) {
    throw 'Not inside a git repository.'
}

Set-Location $repo

$dirty = (& git status --porcelain)
if ($dirty) {
    throw "Working tree is dirty. Commit, stash, or move changes before creating a task branch.`n$dirty"
}

$branch = "$Type/$Name"

& git show-ref --verify --quiet "refs/heads/$branch"
if ($LASTEXITCODE -eq 0) {
    throw "Local branch already exists: $branch"
}

Invoke-Git fetch $Remote --prune

& git show-ref --verify --quiet "refs/remotes/$Remote/$branch"
if ($LASTEXITCODE -eq 0) {
    throw "Remote branch already exists: $Remote/$branch"
}

Select-BaseBranch -Branch $Base -RemoteName $Remote
Invoke-Git switch -c $branch

Write-Host "Created and switched to $branch from $Base"
