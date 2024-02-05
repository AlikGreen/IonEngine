$directory = "D:\Data\Projects\IonEngine" # replace with your directory path
$csharpFiles = Get-ChildItem -Path $directory -Filter *.cs -Recurse
$lineCount = 0

foreach ($file in $csharpFiles) {
    $lineCount += (Get-Content $file.FullName | Measure-Object -Line).Lines
}

Write-Output "Total lines of C# code: $lineCount"

$lineCount = 0

foreach ($file in $csharpFiles) {
    $content = Get-Content $file.FullName
    foreach ($line in $content) {
        # Ignore lines that are empty or start with a comment
        if (-not [string]::IsNullOrWhiteSpace($line) -and -not $line.TrimStart().StartsWith("//")) {
            $lineCount++
        }
    }
}

Write-Output "Total lines of C# code (excluding comments and empty lines): $lineCount"
