/**
 * UEM-256 - Uniform Encryption Method 256-bit
 * A fast, deterministic string hashing method for Node.js
 */

const MASK64 = (1n << 64n) - 1n;

const INITIAL_STATE = [
  0x6D6574615F6D6172n,
  0x7465735F7169735F n,
  0x6E69755F4D654F6Dn,
  0x646F6D5F656E6F6Dn,
];

/**
 * Rotate left a 64-bit value
 * @param {bigint} value - The value to rotate
 * @param {number} shift - The shift amount
 * @returns {bigint} - The rotated value
 */
function rotl64(value, shift) {
  shift = shift & 63;
  return (((value << BigInt(shift)) | (value >> BigInt(64 - shift))) & MASK64);
}

/**
 * Load 8 bytes as a little-endian 64-bit integer
 * @param {Buffer|Uint8Array} data - The data to load from
 * @param {number} offset - The offset to start loading from
 * @returns {bigint} - The loaded value
 */
function load64LE(data, offset = 0) {
  let value = 0n;
  for (let i = 0; i < 8; i++) {
    value |= BigInt(data[offset + i]) << BigInt(i * 8);
  }
  return value;
}

/**
 * Mix the state with a 64-bit block
 * @param {bigint[]} state - The 4-element state array
 * @param {bigint} block - The 64-bit block to mix
 */
function mix(state, block) {
  state[0] ^= block;

  for (let round = 0; round < 2; round++) {
    state[0] = (state[0] + state[1]) & MASK64;
    state[1] = rotl64(state[1], 13);
    state[1] ^= state[0];

    state[2] = (state[2] + state[3]) & MASK64;
    state[3] = rotl64(state[3], 16);
    state[3] ^= state[2];

    state[0] = (state[0] + state[3]) & MASK64;
    state[3] = rotl64(state[3], 21);
    state[3] ^= state[0];

    state[2] = (state[2] + state[1]) & MASK64;
    state[1] = rotl64(state[1], 17);
    state[1] ^= state[2];
  }

  state[0] ^= block;
}

/**
 * Convert a bigint to a little-endian byte array
 * @param {bigint} value - The value to convert
 * @returns {Buffer} - The 8-byte little-endian buffer
 */
function bigintToLE(value) {
  const buf = Buffer.alloc(8);
  for (let i = 0; i < 8; i++) {
    buf[i] = Number((value >> BigInt(i * 8)) & 0xFFn);
  }
  return buf;
}

/**
 * UEM256 class for streaming hash computation
 */
class UEM256 {
  constructor() {
    this._state = [...INITIAL_STATE];
    this._length = 0;
    this._buffer = Buffer.alloc(0);
  }

  /**
   * Update the hash with new data
   * @param {string|Buffer|Uint8Array} data - The data to hash
   * @returns {UEM256} - Returns this for chaining
   */
  update(data) {
    // Convert input to Buffer
    if (typeof data === 'string') {
      data = Buffer.from(data, 'utf8');
    } else if (data instanceof Uint8Array && !(data instanceof Buffer)) {
      data = Buffer.from(data);
    } else if (!Buffer.isBuffer(data) && data != null) {
      throw new TypeError('UEM256.update expects string or Buffer');
    }

    if (!data || data.length === 0) {
      return this;
    }

    this._length += data.length;

    // Fill the buffer to 8 bytes
    if (this._buffer.length > 0) {
      const toCopy = Math.min(8 - this._buffer.length, data.length);
      this._buffer = Buffer.concat([this._buffer, data.slice(0, toCopy)]);
      data = data.slice(toCopy);

      if (this._buffer.length === 8) {
        mix(this._state, load64LE(this._buffer, 0));
        this._buffer = Buffer.alloc(0);
      }
    }

    // Process full 8-byte blocks
    let i = 0;
    while (i + 8 <= data.length) {
      mix(this._state, load64LE(data, i));
      i += 8;
    }

    // Store remaining bytes
    if (i < data.length) {
      this._buffer = Buffer.concat([this._buffer, data.slice(i)]);
    }

    return this;
  }

  /**
   * Finalize and return the digest as a Buffer
   * @returns {Buffer} - The 32-byte hash digest
   */
  digest() {
    const state = [...this._state];
    const buffer = this._buffer;
    const totalLength = this._length;

    // Prepare the tail block
    const tail = Buffer.alloc(8);
    buffer.copy(tail, 0, 0, Math.min(buffer.length, 8));
    if (buffer.length < 8) {
      tail[buffer.length] = 0x80;
    }
    tail[7] = totalLength & 0xFF;

    mix(state, load64LE(tail, 0));

    // Finalization
    state[0] ^= BigInt(totalLength) & MASK64;
    for (let i = 0; i < 4; i++) {
      mix(state, state[i] ^ 0xFEDCBA9876543210n);
    }

    // Convert state to bytes
    const result = Buffer.alloc(32);
    for (let i = 0; i < 4; i++) {
      const buf = bigintToLE(state[i]);
      buf.copy(result, i * 8);
    }

    return result;
  }

  /**
   * Return the digest as a hex string
   * @returns {string} - The 64-character hex string
   */
  hexdigest() {
    return this.digest().toString('hex');
  }

  /**
   * Alias for digest() for compatibility
   * @returns {Buffer} - The 32-byte hash digest
   */
  final() {
    return this.digest();
  }

  /**
   * Create a copy of the current context
   * @returns {UEM256} - A new UEM256 instance with copied state
   */
  copy() {
    const clone = new UEM256();
    clone._state = [...this._state];
    clone._length = this._length;
    clone._buffer = Buffer.from(this._buffer);
    return clone;
  }
}

/**
 * Create a new UEM256 hash context
 * @returns {UEM256} - A new UEM256 instance
 */
function newHash() {
  return new UEM256();
}

/**
 * Compute the hash digest of data
 * @param {string|Buffer|Uint8Array} data - The data to hash
 * @returns {Buffer} - The 32-byte hash digest
 */
function digest(data) {
  return new UEM256().update(data).digest();
}

/**
 * Compute the hash digest of data as a hex string
 * @param {string|Buffer|Uint8Array} data - The data to hash
 * @returns {string} - The 64-character hex string
 */
function hexdigest(data) {
  return new UEM256().update(data).hexdigest();
}

/**
 * Compute the hash of data (alias for hexdigest)
 * @param {string|Buffer|Uint8Array} data - The data to hash
 * @returns {string} - The 64-character hex string
 */
function hash(data) {
  return hexdigest(data);
}

// CommonJS export
module.exports = {
  UEM256,
  new: newHash,
  digest,
  hexdigest,
  hash,
};
