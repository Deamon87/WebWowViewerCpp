##############################################################################
##
## Invoke-CmdScript
##
## From PowerShell Cookbook (O'Reilly)
## by Lee Holmes (http://www.leeholmes.com/guide)
##
##############################################################################

<#

.SYNOPSIS

Invoke the specified batch file (and parameters), but also propagate any
environment variable changes back to the PowerShell environment that
called it.

.EXAMPLE

PS > type foo-that-sets-the-FOO-env-variable.cmd
@set FOO=%*
echo FOO set to %FOO%.

PS > $env:FOO
PS > Invoke-CmdScript "foo-that-sets-the-FOO-env-variable.cmd" Test

C:\Temp>echo FOO set to Test.
FOO set to Test.

PS > $env:FOO
Test

#>

param(
    ## The path to the script to run
    [Parameter(Mandatory = $true)]
    [string] $Path,

    ## The arguments to the script
    [string] $ArgumentList
)

Set-StrictMode -Version 3

$tempFile = [IO.Path]::GetTempFileName()

echo "tempFile = $tempFile"
echo "Path = $Path"

if (!(Test-Path $Path)) {
  Write-Warning "$Path is absent"
}

## Store the output of cmd.exe.  We also ask cmd.exe to output
## the environment table after the batch file completes
$result = (cmd /c "`"$Path`" $argumentList && set")
echo "result = $result"

## Go through the environment variables in the temp file.
## For each of them, set the variable in our local environment.
result | Foreach-Object {
    if($_ -match "^(.*?)=(.*)$")
    {
        Set-Content "env:\$($matches[1])" $matches[2]
    }
}
