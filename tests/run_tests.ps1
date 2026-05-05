$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..")
$srcDir = Join-Path $root "src"
$testExe = Join-Path $scriptDir "test_uem256.exe"
$testSource = Join-Path $scriptDir "test_uem256.c"
$scalarSources = @(
    (Join-Path $srcDir "encryption.c")
)
$simdSource = Join-Path $srcDir "encryption_simd.c"

function Compile-WithCl {
    Write-Host "Compiling with cl.exe..."
    $args = @(
        '/nologo',
        '/std:c11',
        '/W4',
        "/I$srcDir",
        "/Fe:$testExe",
        $testSource,
        $scalarSources[0],
        $simdSource
    )
    cl @args
    return $LASTEXITCODE
}

function Compile-WithGcc {
    Write-Host "Compiling with gcc/clang..."
    $compiler = $args[0]
    $cmdArgs = @(
        '-std=c11',
        '-Wall',
        '-Wextra',
        '-O2',
        "-I$srcDir",
        '-o', $testExe,
        $testSource,
        $scalarSources[0],
        $simdSource
    )
    & $compiler @cmdArgs
    return $LASTEXITCODE
}

$compilerFound = $false
if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
    $compilerFound = $true
    $exitCode = Compile-WithCl
} elseif (Get-Command gcc.exe -ErrorAction SilentlyContinue) {
    $compilerFound = $true
    $args = @('gcc')
    $exitCode = Compile-WithGcc
} elseif (Get-Command clang.exe -ErrorAction SilentlyContinue) {
    $compilerFound = $true
    $args = @('clang')
    $exitCode = Compile-WithGcc
}

if (-not $compilerFound) {
    Write-Error "No supported C compiler found. Install Visual Studio, MSVC, MinGW, or Clang and retry."
    exit 1
}

if ($exitCode -ne 0) {
    Write-Warning "Initial compilation failed; retrying scalar-only build."
    if (Test-Path $testExe) { Remove-Item $testExe -Force }
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        cl /nologo /std:c11 /W4 /I"$srcDir" /Fe:"$testExe" "$testSource" "$scalarSources[0]"
        $exitCode = $LASTEXITCODE
    } else {
        & $args[0] -std=c11 -Wall -Wextra -O2 -I"$srcDir" -o "$testExe" "$testSource" "$scalarSources[0]"
        $exitCode = $LASTEXITCODE
    }
}

if ($exitCode -ne 0) {
    Write-Error "Compilation failed. Please ensure your compiler supports C11 and is available on PATH."
    exit $exitCode
}

Write-Host "Running UEM-256 tests..."
& $testExe
if ($LASTEXITCODE -ne 0) {
    Write-Error "C tests failed."
    exit $LASTEXITCODE
}

Write-Host "Running UEM-256 fuzz tests..."
$python = if (Get-Command python3.exe -ErrorAction SilentlyContinue) {
    'python3'
} elseif (Get-Command python.exe -ErrorAction SilentlyContinue) {
    'python'
} else {
    Write-Error "Python 3 is not installed or not on PATH."
    exit 1
}
& $python (Join-Path $scriptDir "fuzz_uem256.py")
if ($LASTEXITCODE -ne 0) {
    Write-Error "Fuzz tests failed."
    exit $LASTEXITCODE
}

Write-Host "All tests passed."
exit 0
