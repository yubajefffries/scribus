# Imports every .pub in a folder with our Scribus build, one process per file,
# and prints a summary table. Per-file output: <OutDir>\<name>\report.json, page PNGs, imported.sla.
param(
    [string]$Samples = 'C:\Developer\pub-samples',
    [string]$OutDir = 'C:\Developer\pubtest-out',
    [string]$Build = 'C:\Developer\Scribus-builds\Scribus-Release-x64-v143',
    [int]$TimeoutSec = 180
)
$ErrorActionPreference = 'Stop'
$script = Join-Path $PSScriptRoot 'pubtest_scribus.py'

# Throwaway profile: no first-run wizard, no font-substitution dialog.
$prefsDir = Join-Path $OutDir '_profile'
New-Item -ItemType Directory -Force $prefsDir | Out-Null
Set-Content -Encoding utf8 (Join-Path $prefsDir 'scribus172.rc') `
    '<SCRIBUSRC VERSION="1.7.0"><UI ShowFirstStartWizard="0" ShowStartupDialog="0"/><Fonts AutomaticSubstitution="0"/></SCRIBUSRC>'

$results = foreach ($pub in Get-ChildItem $Samples -Filter *.pub) {
    $out = Join-Path $OutDir $pub.BaseName
    if (Test-Path $out) { Remove-Item -Recurse -Force $out }
    New-Item -ItemType Directory -Force $out | Out-Null
    $sw = [Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process (Join-Path $Build 'Scribus.exe') -WorkingDirectory $Build -PassThru `
        -ArgumentList @('--prefs', "`"$prefsDir`"", '-ns', '-py', "`"$script`"", "`"$($pub.FullName)`"", "`"$out`"")
    $status = if ($p.WaitForExit($TimeoutSec * 1000)) { "exit $($p.ExitCode)" } else { $p.Kill(); 'TIMEOUT' }
    $reportFile = Join-Path $out 'report.json'
    $r = if (Test-Path $reportFile) { Get-Content $reportFile -Raw | ConvertFrom-Json } else { $null }
    [pscustomobject]@{
        File    = $pub.Name
        Result  = if ($r.ok) { 'OK' } elseif ($r) { 'ERROR' } else { $status }
        Pages   = if ($r) { @($r.pages).Count } else { 0 }
        Items   = ($r.pages | Measure-Object items -Sum).Sum
        Text    = ($r.pages | Measure-Object text_chars -Sum).Sum
        Images  = ($r.pages | Measure-Object images -Sum).Sum
        Seconds = [int]$sw.Elapsed.TotalSeconds
    }
}
$results | Format-Table -AutoSize
$results | ConvertTo-Json | Set-Content -Encoding utf8 (Join-Path $OutDir 'summary.json')
