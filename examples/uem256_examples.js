/**
 * UEM-256 JavaScript Examples
 * Demonstrates different usage patterns
 */

const { UEM256, hash, digest, hexdigest } = require('../uem256.js');

console.log('=== UEM-256 JavaScript Examples ===\n');

// Example 1: Simple one-shot hashing
console.log('1. Simple one-shot hashing:');
console.log('   Input: "hello world"');
console.log('   Output:', hash('hello world'));
console.log();

// Example 2: Different input types
console.log('2. Different input types:');
console.log('   String:', hash('hello'));
console.log('   Buffer:', hash(Buffer.from('hello')));
console.log('   Hex output:', hexdigest('hello'));
console.log('   Binary output length:', digest('hello').length, 'bytes');
console.log();

// Example 3: Streaming API with small chunks
console.log('3. Streaming API (small chunks):');
const ctx1 = new UEM256();
const payload = 'The quick brown fox jumps over the lazy dog';
for (let i = 0; i < payload.length; i += 5) {
  ctx1.update(payload.substring(i, i + 5));
}
console.log('   Chunked input (5 bytes at a time):', ctx1.hexdigest());
console.log('   Single shot:', hash(payload));
console.log('   Match:', ctx1.hexdigest() === hash(payload) ? '✓' : '✗');
console.log();

// Example 4: Chaining updates
console.log('4. Chaining updates:');
const result = new UEM256()
  .update('The ')
  .update('quick ')
  .update('brown ')
  .update('fox')
  .hexdigest();
console.log('   Chained: ', result);
console.log('   Expected:', hash('The quick brown fox'));
console.log('   Match:', result === hash('The quick brown fox') ? '✓' : '✗');
console.log();

// Example 5: Context copying
console.log('5. Context copying:');
const basectx = new UEM256().update('prefix');
const ctx2 = basectx.copy();
basectx.update('_one');
ctx2.update('_two');
console.log('   Context 1:', basectx.hexdigest());
console.log('   Context 2:', ctx2.hexdigest());
console.log('   Different:', basectx.hexdigest() !== ctx2.hexdigest() ? '✓' : '✗');
console.log();

// Example 6: Use case - file fingerprinting
console.log('6. Use case: File fingerprinting');
const fileContent = Buffer.from('Sample file content for fingerprinting');
console.log('   File size:', fileContent.length, 'bytes');
console.log('   Fingerprint:', hash(fileContent));
console.log();

// Example 7: Use case - data deduplication
console.log('7. Use case: Data deduplication');
const data1 = 'Some configuration data';
const data2 = 'Some configuration data';
const data3 = 'Different configuration';
console.log('   Data1 hash:', hash(data1).substring(0, 16) + '...');
console.log('   Data2 hash:', hash(data2).substring(0, 16) + '...');
console.log('   Data3 hash:', hash(data3).substring(0, 16) + '...');
console.log('   Data1 === Data2:', hash(data1) === hash(data2) ? '✓' : '✗');
console.log('   Data1 === Data3:', hash(data1) === hash(data3) ? '✗' : '✓');
console.log();

// Example 8: Use case - content-based caching
console.log('8. Use case: Content-based caching');
class CachedData {
  constructor(content) {
    this.content = content;
    this.hash = hash(content);
  }

  isSameAs(other) {
    return this.hash === hash(other);
  }
}

const cached = new CachedData('Important data');
console.log('   Cached hash:', cached.hash.substring(0, 16) + '...');
console.log('   Same data?', cached.isSameAs('Important data') ? '✓' : '✗');
console.log('   Different data?', cached.isSameAs('Other data') ? '✗' : '✓');
console.log();

// Example 9: Deterministic hashing
console.log('9. Deterministic hashing:');
const testData = 'Determinism test';
const hashes = [];
for (let i = 0; i < 3; i++) {
  hashes.push(hash(testData));
}
console.log('   Hash 1:', hashes[0].substring(0, 16) + '...');
console.log('   Hash 2:', hashes[1].substring(0, 16) + '...');
console.log('   Hash 3:', hashes[2].substring(0, 16) + '...');
console.log('   All identical:', hashes.every(h => h === hashes[0]) ? '✓' : '✗');
console.log();

// Example 10: Empty and edge cases
console.log('10. Edge cases:');
console.log('   Empty string:', hash('').substring(0, 16) + '...');
console.log('   Single char:', hash('a').substring(0, 16) + '...');
console.log('   Very long string:', hash('x'.repeat(10000)).substring(0, 16) + '...');
console.log('   Binary data:', hash(Buffer.from([0, 1, 2, 3, 4, 5])).substring(0, 16) + '...');
