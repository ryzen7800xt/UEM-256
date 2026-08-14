/**
 * UEM-256 TypeScript type definitions
 */

/**
 * UEM256 class for streaming hash computation
 */
export class UEM256 {
  constructor();

  /**
   * Update the hash with new data
   * @param data The data to hash (string or Buffer)
   * @returns Returns this for chaining
   */
  update(data: string | Buffer | Uint8Array): UEM256;

  /**
   * Finalize and return the digest as a Buffer
   * @returns The 32-byte hash digest
   */
  digest(): Buffer;

  /**
   * Return the digest as a hex string
   * @returns The 64-character hex string
   */
  hexdigest(): string;

  /**
   * Alias for digest() for compatibility
   * @returns The 32-byte hash digest
   */
  final(): Buffer;

  /**
   * Create a copy of the current context
   * @returns A new UEM256 instance with copied state
   */
  copy(): UEM256;
}

/**
 * Create a new UEM256 hash context
 * @returns A new UEM256 instance
 */
export function new(): UEM256;

/**
 * Compute the hash digest of data
 * @param data The data to hash
 * @returns The 32-byte hash digest
 */
export function digest(data: string | Buffer | Uint8Array): Buffer;

/**
 * Compute the hash digest of data as a hex string
 * @param data The data to hash
 * @returns The 64-character hex string
 */
export function hexdigest(data: string | Buffer | Uint8Array): string;

/**
 * Compute the hash of data (alias for hexdigest)
 * @param data The data to hash
 * @returns The 64-character hex string
 */
export function hash(data: string | Buffer | Uint8Array): string;
