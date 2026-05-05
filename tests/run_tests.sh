#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$script_dir/.." && pwd)"
src_dir="$root/src"
test_exe="$script_dir/test_uem256"
test_source="$script_dir/test_uem256.c"
scalar_source="$src_dir/encryption.c"
simd_source="$src_dir/encryption_simd.c"

if command -v cc >/dev/null 2>&1; then
    compiler=cc
elif command -v gcc >/dev/null 2>&1; then
    compiler=gcc
elif command -v clang >/dev/null 2>&1; then
    compiler=clang
else
    echo "No supported C compiler found. Install clang or gcc and retry." >&2
    exit 1
fi

compile() {
    echo "Compiling with $compiler..."
    "$compiler" -std=c11 -Wall -Wextra -O2 -I"$src_dir" -o "$test_exe" "$test_source" "$scalar_source" "$simd_source"
}

if ! compile; then
    echo "Initial compilation failed; retrying scalar-only build..."
    rm -f "$test_exe"
    "$compiler" -std=c11 -Wall -Wextra -O2 -I"$src_dir" -o "$test_exe" "$test_source" "$scalar_source"
fi

if [ ! -x "$test_exe" ]; then
    echo "Compilation failed. Please ensure your compiler supports C11." >&2
    exit 1
fi

echo "Running UEM-256 tests..."
"$test_exe"

echo "Running UEM-256 fuzz tests..."
if command -v python3 >/dev/null 2>&1; then
    python_cmd=python3
elif command -v python >/dev/null 2>&1; then
    python_cmd=python
else
    echo "Python 3 is required to run the fuzz tester." >&2
    exit 1
fi

"$python_cmd" "$script_dir/fuzz_uem256.py"

echo "All tests passed."
exit 0
