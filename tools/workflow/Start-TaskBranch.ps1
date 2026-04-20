[CmdletBinding()]
param(
    [ValidateSet('task', 'fix', 'spike', 'review', 'docs')]
    [string]$Type = 'task',

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[a-z0-9][a-z0-9._-]*$')]
    [string]$Name,

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
Invoke-Git switch $Base
Invoke-Git pull --ff-only $Remote $Base
Invoke-Git switch -c $branch

Write-Host "Created and switched to $branch from $Base"
