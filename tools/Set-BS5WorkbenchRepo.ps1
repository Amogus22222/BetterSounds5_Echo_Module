param(
	[string]$RepoPath = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
	[string]$CanonicalPath = 'G:\BettersMods\BetterSounds5',
	[string]$BackupRoot = 'G:\BettersMods\_repo_switch_backups'
)

$ErrorActionPreference = 'Stop'

function Assert-PathWithinRoot {
	param(
		[string]$TargetPath,
		[string]$ExpectedRoot
	)

	$resolvedTarget = [System.IO.Path]::GetFullPath($TargetPath)
	$resolvedRoot = [System.IO.Path]::GetFullPath($ExpectedRoot)
	if (!$resolvedTarget.StartsWith($resolvedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Path '$resolvedTarget' is outside expected root '$resolvedRoot'."
	}
}

$RepoPath = (Resolve-Path $RepoPath).Path
$canonicalParent = Split-Path -Parent $CanonicalPath
if (!(Test-Path $canonicalParent)) {
	throw "Canonical parent path does not exist: $canonicalParent"
}

Assert-PathWithinRoot -TargetPath $CanonicalPath -ExpectedRoot $canonicalParent

if (!(Test-Path (Join-Path $RepoPath 'addon.gproj'))) {
	throw "Repo path does not look like a Reforger addon root: $RepoPath"
}

if (!(Test-Path $BackupRoot)) {
	New-Item -ItemType Directory -Path $BackupRoot | Out-Null
}

$canonicalItem = Get-Item -LiteralPath $CanonicalPath -ErrorAction SilentlyContinue
if ($canonicalItem) {
	if (($canonicalItem.Attributes -band [IO.FileAttributes]::ReparsePoint) -and $canonicalItem.LinkType -eq 'Junction') {
		$existingTarget = [System.IO.Path]::GetFullPath((Get-Item -LiteralPath $CanonicalPath).Target)
		if ($existingTarget -eq [System.IO.Path]::GetFullPath($RepoPath)) {
			Write-Output "Canonical path already points to repo: $RepoPath"
			exit 0
		}

		Remove-Item -LiteralPath $CanonicalPath -Force
	}
	else {
		$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
		$backupPath = Join-Path $BackupRoot ("BetterSounds5_" + $stamp)
		Move-Item -LiteralPath $CanonicalPath -Destination $backupPath
		Write-Output "Backed up previous canonical directory to: $backupPath"
	}
}

New-Item -ItemType Junction -Path $CanonicalPath -Target $RepoPath | Out-Null
Write-Output "Created junction:"
Write-Output "  $CanonicalPath -> $RepoPath"
