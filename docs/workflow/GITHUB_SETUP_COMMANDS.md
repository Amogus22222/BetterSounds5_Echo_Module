# GitHub Setup Commands

Use these commands from the repository root after `gh` is installed and authenticated.

Repository:

```text
Amogus22222/BetterSounds5_Echo_Module
```

Remote:

```text
https://github.com/Amogus22222/BetterSounds5_Echo_Module.git
```

## Install `gh` On Windows

The GitHub Codex plugin can confirm repository access and collaborator permission when the plugin exposes those tools. In this session it does not expose branch protection or rulesets endpoints, so the protection checks and write commands below still require `gh` to be installed and available in `PATH`.

Use one installed package manager:

```powershell
winget install --id GitHub.cli
```

or:

```powershell
choco install gh
```

or:

```powershell
scoop install gh
```

Then open a new PowerShell session and verify:

```powershell
gh --version
Get-Command gh
```

## Authenticate

```powershell
gh auth login --hostname github.com --git-protocol https --web
gh auth status --hostname github.com
```

If the token has insufficient scopes, refresh with repository scope:

```powershell
gh auth refresh --hostname github.com --scopes repo
```

For fine-grained tokens, grant this repository at least:

- Metadata: read
- Contents: read
- Administration: read/write, only if branch protection or rulesets must be changed

## Read-Only Checks

```powershell
gh repo view Amogus22222/BetterSounds5_Echo_Module --json nameWithOwner,defaultBranchRef,viewerPermission,isPrivate
gh api repos/Amogus22222/BetterSounds5_Echo_Module --jq '{full_name,private,permissions}'
gh api -i repos/Amogus22222/BetterSounds5_Echo_Module/branches/main/protection
gh api -i repos/Amogus22222/BetterSounds5_Echo_Module/rulesets
```

Notes:

- `viewerPermission` should be `ADMIN` before changing branch protection.
- A `404` from branch protection can mean protection is not configured or the token cannot see it. Check `viewerPermission` and repository permissions before assuming either case.
- There is no safe no-write API call that proves a later branch-protection update will succeed. The first `PUT` is the write.

## Configure `main` Protection

Run this only after the read-only checks confirm valid auth and sufficient repository administration permission.

This intentionally does not enable required status checks because this repo currently has no real CI workflow to require.

```powershell
$bodyPath = Join-Path $env:TEMP 'bs5-main-protection.json'

@'
{
  "required_status_checks": null,
  "enforce_admins": false,
  "required_pull_request_reviews": {
    "required_approving_review_count": 1,
    "dismiss_stale_reviews": false,
    "require_code_owner_reviews": false,
    "require_last_push_approval": false
  },
  "restrictions": null,
  "required_conversation_resolution": true
}
'@ | Set-Content -LiteralPath $bodyPath -Encoding UTF8

gh api --method PUT repos/Amogus22222/BetterSounds5_Echo_Module/branches/main/protection --input $bodyPath

Remove-Item -LiteralPath $bodyPath
```

Verify after writing:

```powershell
gh api repos/Amogus22222/BetterSounds5_Echo_Module/branches/main/protection --jq '{required_pull_request_reviews,required_conversation_resolution,required_status_checks}'
```

## Manual GitHub UI Path

If CLI permissions are not available, configure in GitHub UI:

1. Open repository settings.
2. Go to Rules > Rulesets, or Branches > Branch protection rules depending on the GitHub UI available to the repo.
3. Target branch: `main`.
4. Enable require pull request before merging.
5. Set required approvals to `1`.
6. Enable require conversation resolution before merging.
7. Do not enable required status checks until a real CI workflow exists.
8. Do not configure `integration` unless that branch exists and is being used.
9. Do not delete or modify legacy `codex` and `antigravity` branches unless the maintainer explicitly decides to do so.
