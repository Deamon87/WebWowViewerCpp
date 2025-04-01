param(
    ## The path to the script to run
    [Parameter(Mandatory = $true)]
    [string] $Path,

    ## The arguments to the script
    [string] $ArgumentList
)


# Set paths for log files
$envBeforeLog = "env_before.log"
$envAfterLog  = "env_after.log"

# -------------------------------------------
# Step 1: Capture the initial environment variables
# -------------------------------------------
$envBefore = Get-ChildItem Env: | Sort-Object Name

# # Log the initial environment variables to a file
# $envBefore | Format-Table -AutoSize | Out-File $envBeforeLog

# Convert the environment variables to a hashtable for easier comparison
$envBeforeHash = @{}
foreach ($item in $envBefore) {
    $envBeforeHash[$item.Name] = $item.Value
}

# -------------------------------------------
# Step 2: Run the batch file and capture its environment output
# -------------------------------------------
# Replace 'your_script.bat' with your batch file path.
# The command runs the batch file and then runs "set" to list all environment variables.
$envAfter = cmd.exe /c "`"$Path`" $argumentList > nul && set"

# -------------------------------------------
# Step 3: Parse the environment variables output from the batch file run
# -------------------------------------------
$envAfterHash = @{}
$envAfter | ForEach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
        $key = $matches[1]
        $value = $matches[2]
        $envAfterHash[$key] = $value
    }
}

# -------------------------------------------
# Step 4: Compare the before and after snapshots
# -------------------------------------------
$added = @{}
$modified = @{}

foreach ($key in $envAfterHash.Keys) {
    if (-not $envBeforeHash.ContainsKey($key)) {
        # Environment variable added by the batch file
        $added[$key] = $envAfterHash[$key]
    } elseif ($envBeforeHash[$key] -ne $envAfterHash[$key]) {
        # Environment variable modified by the batch file
        $modified[$key] = @{
            "Before" = $envBeforeHash[$key]
            "After"  = $envAfterHash[$key]
        }
    }
}

# -------------------------------------------
# Step 5: Output the results
# -------------------------------------------
if ($added.Count -gt 0) {
    $added.GetEnumerator() | Foreach-Object {
#         echo "Key: $($_.Key), Value: $($_.Value)" | Out-File -Append -FilePath added_env.txt -Encoding utf8
        echo "$($_.Key)=$($_.Value)" | Out-File -Append -FilePath $env:GITHUB_ENV -Encoding utf8
    }
}

if ($modified.Count -gt 0) {
    $modified.GetEnumerator() | Foreach-Object {
        echo "Key: $($_.Key)"
        if ($($_.Key) -eq "path") {
            Compare-Object -ReferenceObject ($_.Value.Before -split ';') -DifferenceObject ($_.Value.After -split ';') |
                Where-Object { $_.SideIndicator -eq '=>' } |
                ForEach-Object { $_.InputObject } |
                Out-File -Append -FilePath $env:GITHUB_PATH -Encoding utf8
#                 Out-File -Append -FilePath added_path.txt -Encoding utf8
#             echo "Key: $($_.Key), Before_Value: $($_.Value.Before), After_Value: $($_.Value.After)" | Out-File -Append -FilePath modified.txt -Encoding utf8
        }
    }
}