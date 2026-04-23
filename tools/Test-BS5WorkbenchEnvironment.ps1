param(
	[string]$CanonicalPath = 'G:\BettersMods\BetterSounds5',
	[string]$WorkbenchExe = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe',
	[string]$WorkbenchDataGproj = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger Tools\Workbench\addons\data\ArmaReforger.gproj'
)

$ErrorActionPreference = 'Stop'

function Write-Check {
	param(
		[string]$Name,
		[bool]$Passed,
		[string]$Detail
	)

	$status = if ($Passed) { 'OK' } else { 'FAIL' }
	Write-Output ("[{0}] {1}: {2}" -f $status, $Name, $Detail)
}

$allPassed = $true

$canonicalExists = Test-Path $CanonicalPath
Write-Check 'Canonical path' $canonicalExists $CanonicalPath
if (!$canonicalExists) {
	$allPassed = $false
}
else {
	$item = Get-Item -LiteralPath $CanonicalPath
	$isJunction = $item.LinkType -eq 'Junction'
	Write-Check 'Canonical junction' $isJunction ("LinkType=" + $item.LinkType)
	if (!$isJunction) {
		$allPassed = $false
	}
}

$addonGproj = Join-Path $CanonicalPath 'addon.gproj'
$addonExists = Test-Path $addonGproj
Write-Check 'Canonical addon.gproj' $addonExists $addonGproj
if (!$addonExists) {
	$allPassed = $false
}

$exeExists = Test-Path $WorkbenchExe
Write-Check 'Workbench exe' $exeExists $WorkbenchExe
if (!$exeExists) {
	$allPassed = $false
}

$dataGprojExists = Test-Path $WorkbenchDataGproj
Write-Check 'Workbench data gproj' $dataGprojExists $WorkbenchDataGproj
if (!$dataGprojExists) {
	$allPassed = $false
}
else {
	$dataText = Get-Content -Raw $WorkbenchDataGproj
	$hasDependencyGuid = $dataText -match 'GUID "58D0FB3206B6F859"'
	Write-Check 'Base game dependency GUID' $hasDependencyGuid '58D0FB3206B6F859'
	if (!$hasDependencyGuid) {
		$allPassed = $false
	}
}

if ($allPassed) {
	Write-Output 'Environment check passed. Remaining launch/build failures are likely Workbench/Steam runtime state, not repo path drift.'
	exit 0
}

Write-Output 'Environment check failed. Fix the FAIL items before trusting MCP build/play results.'
exit 1
