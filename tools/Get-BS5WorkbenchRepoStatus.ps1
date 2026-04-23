param(
	[string]$CanonicalPath = 'G:\BettersMods\BetterSounds5'
)

$ErrorActionPreference = 'Stop'

if (!(Test-Path $CanonicalPath)) {
	Write-Output "Missing canonical path: $CanonicalPath"
	exit 1
}

$item = Get-Item -LiteralPath $CanonicalPath
$isReparsePoint = ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0
$target = $null
if ($isReparsePoint) {
	$target = $item.Target
}

Write-Output ("CanonicalPath=" + $item.FullName)
Write-Output ("IsJunction=" + ($item.LinkType -eq 'Junction'))
if ($target) {
	Write-Output ("Target=" + $target)
}

$gprojPath = Join-Path $CanonicalPath 'addon.gproj'
Write-Output ("HasAddonGproj=" + (Test-Path $gprojPath))
