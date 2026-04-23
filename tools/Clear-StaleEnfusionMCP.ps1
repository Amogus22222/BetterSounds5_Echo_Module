param(
	[string]$Root = 'G:\BettersMods',
	[string]$CanonicalModPath = 'G:\BettersMods\BetterSounds5',
	[switch]$Apply
)

$ErrorActionPreference = 'Stop'

$canonicalBridge = Join-Path $CanonicalModPath 'Scripts\WorkbenchGame\EnfusionMCP'

if (!(Test-Path $Root)) {
	Write-Error ("Root missing: " + $Root)
	exit 1
}

$bridgeDirs = Get-ChildItem -LiteralPath $Root -Recurse -Directory -Filter EnfusionMCP -ErrorAction SilentlyContinue
$staleDirs = @()
foreach ($dir in $bridgeDirs) {
	if ($dir.FullName -ne $canonicalBridge) {
		$staleDirs += $dir
	}
}

if ($staleDirs.Count -eq 0) {
	Write-Output "No stale EnfusionMCP directories found."
	exit 0
}

Write-Output ("Canonical bridge kept: " + $canonicalBridge)
Write-Output "Stale bridge directories:"
$staleDirs | Select-Object -ExpandProperty FullName

if (!$Apply) {
	Write-Output "Dry run only. Re-run with -Apply to delete stale bridge directories."
	exit 0
}

foreach ($dir in $staleDirs) {
	Remove-Item -Recurse -Force -LiteralPath $dir.FullName
	Write-Output ("Deleted: " + $dir.FullName)
}

Write-Output "Done."
