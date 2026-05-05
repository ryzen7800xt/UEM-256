# Cross-Platform Support

UEM-256 now includes explicit support for building and testing on macOS and common Linux distributions, in addition to Windows.

## Supported platforms

* macOS (clang or gcc)
* Ubuntu / Debian
* Fedora / CentOS / RHEL
* Arch Linux
* Other Linux distributions with a C11-capable compiler

## Dependencies

* C compiler with C11 support: `cc`, `gcc`, or `clang`
* Python 3 to run the fuzz tester

## Build and test

From the repository root:

```bash
./tests/run_tests.sh
```

If you have GNU Make installed, you can also use:

```bash
make test
```

Additional Make targets:

```bash
make lint
make fmt
```

The runner will:

* detect a supported C compiler
* build the C test executable
* fall back to scalar-only mode if the SIMD build fails
* run the C test suite
* run the Python fuzz tester

## macOS notes

On macOS, install the developer tools if needed:

```bash
xcode-select --install
```

If you prefer GNU `gcc`, install it with Homebrew:

```bash
brew install gcc
```

## Linux notes

On Linux, install a standard compiler package and Python 3 if needed. For example:

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install build-essential python3
```

Fedora:

```bash
sudo dnf install gcc python3
```

Arch Linux:

```bash
sudo pacman -S gcc python
```

## Windows support

Windows remains supported via the PowerShell runner:

```powershell
tests\run_tests.ps1
```

## Notes

* The shell runner is portable and does not require a separate build system.
* SIMD acceleration is optional; the code falls back to scalar mode on systems without AVX2 support.
* The algorithm is not intended for security-critical use cases.
