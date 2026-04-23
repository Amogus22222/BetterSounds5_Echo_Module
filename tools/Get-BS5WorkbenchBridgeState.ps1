param(
	[string]$Root = 'G:\BettersMods',
	[string]$CanonicalModPath = 'G:\BettersMods\BetterSounds5'
)

$ErrorActionPreference = 'Stop'

$canonicalBridge = Join-Path $CanonicalModPath 'Scripts\WorkbenchGame\EnfusionMCP'

Write-Output ("Root=" + $Root)
Write-Output ("CanonicalModPath=" + $CanonicalModPath)
Write-Output ("CanonicalBridgePath=" + $canonicalBridge)
Write-Output ("CanonicalBridgeExists=" + (Test-Path $canonicalBridge))

if (!(Test-Path $Root)) {
	Write-Output "Root missing"
	exit 1
}

$bridgeDirs = Get-ChildItem -LiteralPath $Root -Recurse -Directory -Filter EnfusionMCP -ErrorAction SilentlyContinue
if (!$bridgeDirs) {
	Write-Output "No EnfusionMCP directories found under root."
	exit 0
}

$rows = foreach ($dir in $bridgeDirs) {
	$isCanonical = $dir.FullName -eq $canonicalBridge
	[PSCustomObject]@{
		Path = $dir.FullName
		State = if ($isCanonical) { 'canonical' } else { 'stale' }
	}
}

$rows | Sort-Object State, Path | Format-Table -AutoSize
