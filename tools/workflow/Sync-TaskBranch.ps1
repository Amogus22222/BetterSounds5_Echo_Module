[CmdletBinding()]
param(
    [string]$Upstream = 'origin/main',

    [ValidateSet('rebase', 'merge')]
    [string]$Mode = 'rebase'
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
    throw "Working tree is dirty. Commit or stash before syncing.`n$dirty"
}

Invoke-Git fetch origin --prune

if ($Mode -eq 'rebase') {
    Invoke-Git rebase $Upstream
} else {
    Invoke-Git merge --ff-only $Upstream
}

Write-Host "Synced current branch with $Upstream using $Mode"
