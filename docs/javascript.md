# UEM-256 JavaScript/Node.js Implementation

A fast, deterministic string hashing method for Node.js and browsers.

## Installation

### NPM
```bash
npm install uem256
```

### Manual
Copy `uem256.js` to your project.

## Quick Start

### CommonJS (Node.js)
```javascript
const { hash, UEM256 } = require('uem256');

// Simple one-shot hashing
const digest = hash('hello world');
console.log(digest);
// Output: aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375

// Streaming API
const ctx = new UEM256();
ctx.update('hello').update(' ').update('world');
console.log(ctx.hexdigest());
// Output: aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375
```

### ES Modules (Browsers, modern Node.js)
```javascript
import { hash, UEM256 } from './uem256.js';

const digest = hash('hello world');
console.log(digest);
```

### TypeScript
```typescript
import { UEM256, hash, digest } from 'uem256';

const myHash: string = hash('data');
const myDigest: Buffer = digest('data');
```

## API Reference

### `hash(data: string | Buffer | Uint8Array): string`
Compute the hash digest as a hexadecimal string.

```javascript
const { hash } = require('uem256');
console.log(hash('hello'));
// Output: e7cf3ef4f17c3999a94f2c6f612e8a888e5b1026878e4e19398b23a8949ccca
```

### `digest(data: string | Buffer | Uint8Array): Buffer`
Compute the hash digest as a Buffer (binary).

```javascript
const { digest } = require('uem256');
const binary = digest('hello');
console.log(binary.length); // 32
console.log(binary.toString('hex'));
// Output: e7cf3ef4f17c3999a94f2c6f612e8a888e5b1026878e4e19398b23a8949ccca
```

### `hexdigest(data: string | Buffer | Uint8Array): string`
Alias for `hash()`. Returns hexadecimal string.

```javascript
const { hexdigest } = require('uem256');
console.log(hexdigest('hello'));
// Output: e7cf3ef4f17c3999a94f2c6f612e8a888e5b1026878e4e19398b23a8949ccca
```

### `UEM256` Class

#### Constructor
```javascript
const ctx = new UEM256();
```

#### `update(data: string | Buffer | Uint8Array): UEM256`
Add data to be hashed. Chainable.

```javascript
ctx.update('hello').update(' ').update('world');
```

#### `digest(): Buffer`
Finalize and return the binary digest (32 bytes).

```javascript
const binary = ctx.digest();
console.log(binary.length); // 32
```

#### `hexdigest(): string`
Finalize and return the hex-encoded digest (64 characters).

```javascript
const hex = ctx.hexdigest();
console.log(hex.length); // 64
```

#### `final(): Buffer`
Alias for `digest()`. Returns the binary digest.

```javascript
const binary = ctx.final();
```

#### `copy(): UEM256`
Create an independent copy of the current context.

```javascript
const ctx1 = new UEM256().update('hello');
const ctx2 = ctx1.copy();
ctx1.update(' world');
ctx2.update(' there');
console.log(ctx1.hexdigest() !== ctx2.hexdigest()); // true
```

## Test Vectors

The implementation uses the same test vectors as the Python reference implementation:

| Input | Expected Output |
|-------|-----------------|
| `` | `ff524be2b601b4c42fd991416c21387105df2a0436c7bb2981481b5e4b44f0a9` |
| `a` | `bb32eb5d34a3fcc5636f144012382083c49543d46c34fa9755225c4f063d0cdf` |
| `hello world` | `aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375` |
| `UEM-256` | `eb96958ffd15ab17f2f8af60f499cce94a82ff52eac1f3178256e20f643e5cb8` |

## Running Tests

```bash
npm test
```

This runs the test suite and verifies all test vectors.

## Performance

The JavaScript implementation uses native BigInt operations for 64-bit arithmetic, making it reasonably fast for Node.js environments. For CPU-intensive workloads, consider using the native C bindings or the SIMD-accelerated versions.

## Compatibility

- **Node.js**: 12.0.0+
- **Browsers**: All modern browsers with BigInt support (Chrome 67+, Firefox 68+, Safari 14+, Edge 79+)
- **TypeScript**: Full type definitions included

## Differences from Python Implementation

The JavaScript implementation maintains API parity with the Python version:

- ✓ Same algorithm and test vectors
- ✓ Streaming `UEM256` class with `update()` API
- ✓ One-shot `hash()`, `digest()`, and `hexdigest()` functions
- ✓ Support for string and Buffer inputs
- ✓ Context copying with `copy()`
- ✓ Chainable `update()` method

## License

MIT

## See Also

- [Python Implementation](../uem256/_core.py)
- [C Reference Implementation](../src/encryption.c)
- [Project Documentation](../README.md)
