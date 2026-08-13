# Python library usage

UEM-256 is available as a Python package in this repository. It can be installed into a regular Python environment and used like any other library.

## Install

From the project root:

```bash
python -m pip install -e .
```

On Windows, the same command is often written with the launcher:

```powershell
py -m pip install -e .
```

## Basic usage

```python
import uem256

print(uem256.hexdigest("hello world"))
print(uem256.hash("hello world"))
print(uem256.digest("hello world"))
```

Example output:

```text
aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375
aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375
b'...32 raw bytes...'
```

## Streaming API

If you are hashing data in chunks, use the context object:

```python
import uem256

ctx = uem256.UEM256()
ctx.update("hello")
ctx.update(" world")
print(ctx.hexdigest())
```

This produces the same result as hashing the combined string in one call.

## Command-line usage

You can also invoke the package directly:

```bash
python -m uem256 "hello world"
```

This prints the UEM-256 hex digest for the supplied input.

## Notes

- `hash(...)` returns a 64-character hex string.
- `digest(...)` returns the raw 32-byte digest.
- `hexdigest(...)` is the most common choice for text output and logging.
- UEM-256 is intended for lightweight, deterministic hashing and is not a cryptographic security primitive.
- This python library was made in order to prove I know cryptography.
