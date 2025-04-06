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

param (
    [string]$filename
)
$env:Path -split ";" | Where-Object {
    Test-Path -Path (Join-Path -Path $_ -ChildPath $filename)  -PathType Leaf
} | ForEach-Object {
    echo (Join-Path -Path $_ -ChildPath $filename)
    return;
}