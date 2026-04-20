[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$repo = (& git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or -not $repo) {
    throw 'Not inside a git repository.'
}

Set-Location $repo

$requiredFiles = @(
    'README.md',
    'docs/workflow/AI_AGENT_WORKFLOW.md',
    'docs/workflow/GIT_WORKTREE_SETUP.md',
    'docs/workflow/BRANCHING_STRATEGY.md',
    'docs/workflow/REPO_SYNC_VALIDATION.md',
    'docs/workflow/HUMAN_ACTIONS_REQUIRED.md',
    'docs/workflow/FINAL_AUDIT_REPORT.md',
    'docs/workflow/POST_FIX_AUDIT.md',
    'docs/checklists/AGENT_TASK_CHECKLIST.md',
    'docs/checklists/PR_REVIEW_CHECKLIST.md',
    'docs/agents/CODEX_ROLE.md',
    'docs/agents/ANTIGRAVITY_ROLE.md',
    'docs/agents/HANDOFF_PROTOCOL.md',
    '.codex/AGENTS.md',
    '.codex/SKILLS.md',
    '.codex/TASK_TEMPLATE.md',
    '.github/pull_request_template.md',
    'tools/workflow/Start-TaskBranch.ps1',
    'tools/workflow/Setup-AgentWorktrees.ps1',
    'tools/workflow/Sync-TaskBranch.ps1',
    'tools/workflow/Validate-WorkflowDocs.ps1'
)

$missing = @()
foreach ($file in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        $missing += $file
    }
}

if ($missing.Count -gt 0) {
    throw "Missing required workflow files:`n$($missing -join "`n")"
}

$checks = @(
    @{ File = 'docs/workflow/AI_AGENT_WORKFLOW.md'; Pattern = 'main.*stable|stable.*main'; Name = 'AI workflow documents main as stable' },
    @{ File = 'docs/workflow/AI_AGENT_WORKFLOW.md'; Pattern = 'integration.*optional|optional.*integration'; Name = 'AI workflow documents optional integration' },
    @{ File = 'docs/workflow/AI_AGENT_WORKFLOW.md'; Pattern = 'task/\*'; Name = 'AI workflow documents task branches' },
    @{ File = 'docs/workflow/AI_AGENT_WORKFLOW.md'; Pattern = 'origin/integration'; Name = 'AI workflow documents integration upstream sync' },
    @{ File = 'docs/workflow/BRANCHING_STRATEGY.md'; Pattern = 'codex.*antigravity|antigravity.*codex'; Name = 'Branching strategy mentions legacy agent branches' },
    @{ File = 'docs/workflow/BRANCHING_STRATEGY.md'; Pattern = 'origin/integration'; Name = 'Branching strategy documents integration upstream sync' },
    @{ File = 'docs/workflow/GIT_WORKTREE_SETUP.md'; Pattern = 'git worktree add'; Name = 'Worktree doc includes create command' },
    @{ File = 'docs/workflow/GIT_WORKTREE_SETUP.md'; Pattern = 'git worktree remove'; Name = 'Worktree doc includes cleanup command' },
    @{ File = 'docs/workflow/GIT_WORKTREE_SETUP.md'; Pattern = 'git worktree repair'; Name = 'Worktree doc includes repair command' },
    @{ File = 'docs/workflow/GIT_WORKTREE_SETUP.md'; Pattern = 'clean worktree|worktree is clean'; Name = 'Worktree doc states clean removal requirement' },
    @{ File = 'docs/workflow/HUMAN_ACTIONS_REQUIRED.md'; Pattern = 'Branch Protection'; Name = 'Human actions include branch protection' },
    @{ File = 'docs/workflow/HUMAN_ACTIONS_REQUIRED.md'; Pattern = 'repo-local helper scripts'; Name = 'Human actions distinguish repo-local helpers from GitHub settings' },
    @{ File = '.codex/AGENTS.md'; Pattern = 'Do not use long-lived `codex` or `antigravity`'; Name = 'Codex instructions prohibit long-lived agent lanes' },
    @{ File = '.github/pull_request_template.md'; Pattern = 'Verification'; Name = 'PR template includes verification section' }
)

$failed = @()
foreach ($check in $checks) {
    $match = Select-String -Path $check.File -Pattern $check.Pattern -Quiet
    if (-not $match) {
        $failed += $check.Name
    }
}

if ($failed.Count -gt 0) {
    throw "Workflow docs failed consistency checks:`n$($failed -join "`n")"
}

$branchNames = Select-String -Path @(
    'docs/workflow/AI_AGENT_WORKFLOW.md',
    'docs/workflow/BRANCHING_STRATEGY.md',
    'docs/workflow/GIT_WORKTREE_SETUP.md',
    '.codex/AGENTS.md'
) -Pattern 'task/|fix/|spike/|review/|docs/' -AllMatches

if (-not $branchNames) {
    throw 'No short-lived branch examples found.'
}

$markdownFiles = @('README.md') + (Get-ChildItem -Path 'docs', '.codex', '.github' -Recurse -File -Filter '*.md' | ForEach-Object { $_.FullName })
$brokenLinks = @()

foreach ($markdownFile in $markdownFiles) {
    $text = Get-Content -LiteralPath $markdownFile -Raw
    $baseDir = Split-Path -Parent $markdownFile
    if (-not $baseDir) {
        $baseDir = '.'
    }
    $matches = [regex]::Matches($text, '\[[^\]]+\]\(([^)]+)\)')

    foreach ($match in $matches) {
        $target = $match.Groups[1].Value.Trim()
        if (-not $target -or $target.StartsWith('#') -or $target -match '^[a-zA-Z][a-zA-Z0-9+.-]*:') {
            continue
        }

        $targetPath = ($target -split '#')[0]
        if (-not $targetPath) {
            continue
        }

        $resolved = Join-Path $baseDir $targetPath
        if (-not (Test-Path -LiteralPath $resolved)) {
            $relativeSource = Resolve-Path -Relative $markdownFile
            $brokenLinks += "$relativeSource -> $target"
        }
    }
}

if ($brokenLinks.Count -gt 0) {
    throw "Broken local markdown links:`n$($brokenLinks -join "`n")"
}

$parseFailures = @()
foreach ($scriptFile in Get-ChildItem -Path 'tools/workflow' -File -Filter '*.ps1') {
    $tokens = $null
    $parseErrors = $null
    [System.Management.Automation.Language.Parser]::ParseFile($scriptFile.FullName, [ref]$tokens, [ref]$parseErrors) | Out-Null
    if ($parseErrors.Count -gt 0) {
        $relativeScript = Resolve-Path -Relative $scriptFile.FullName
        $messages = $parseErrors | ForEach-Object { "$($_.Extent.StartLineNumber): $($_.Message)" }
        $parseFailures += "$relativeScript`n$($messages -join "`n")"
    }
}

if ($parseFailures.Count -gt 0) {
    throw "PowerShell helper syntax errors:`n$($parseFailures -join "`n")"
}

Write-Host 'Workflow docs validation passed.'
