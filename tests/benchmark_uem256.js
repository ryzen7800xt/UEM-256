/**
 * UEM-256 JavaScript Performance Benchmark
 */

const { UEM256, hash, digest } = require('../uem256.js');

function benchmark(name, fn, iterations = 1000) {
  const start = process.hrtime.bigint();
  
  for (let i = 0; i < iterations; i++) {
    fn();
  }
  
  const end = process.hrtime.bigint();
  const duration = Number(end - start) / 1_000_000; // Convert to milliseconds
  const opsPerSec = (iterations / duration * 1000).toFixed(0);
  
  console.log(`${name.padEnd(40)} ${duration.toFixed(2)}ms (${opsPerSec} ops/sec)`);
}

console.log('=== UEM-256 JavaScript Benchmark ===\n');
console.log('Test'.padEnd(40), 'Time         Performance');
console.log('-'.repeat(70));

// Benchmark 1: Small strings
benchmark('Small string (10 bytes)', () => {
  hash('hello123');
}, 10000);

// Benchmark 2: Medium strings
benchmark('Medium string (100 bytes)', () => {
  hash('a'.repeat(100));
}, 5000);

// Benchmark 3: Large strings
benchmark('Large string (10KB)', () => {
  hash('x'.repeat(10240));
}, 500);

// Benchmark 4: Very large strings
benchmark('Very large string (1MB)', () => {
  hash('y'.repeat(1024 * 1024));
}, 10);

// Benchmark 5: Streaming - small chunks
benchmark('Streaming (8 byte chunks)', () => {
  const ctx = new UEM256();
  for (let i = 0; i < 100; i++) {
    ctx.update('12345678');
  }
  ctx.digest();
}, 1000);

// Benchmark 6: Streaming - large chunks
benchmark('Streaming (1KB chunks)', () => {
  const ctx = new UEM256();
  ctx.update('x'.repeat(1024 * 10));
  ctx.digest();
}, 1000);

// Benchmark 7: Binary data (Buffers)
benchmark('Buffer input (100 bytes)', () => {
  const buf = Buffer.alloc(100);
  hash(buf);
}, 5000);

// Benchmark 8: hex output
benchmark('Hexdigest (100 bytes)', () => {
  const ctx = new UEM256();
  ctx.update('x'.repeat(100));
  ctx.hexdigest();
}, 5000);

// Benchmark 9: Context copying
benchmark('Context copy & update', () => {
  const ctx = new UEM256().update('data');
  const copy = ctx.copy();
  copy.update('more').digest();
}, 5000);

// Benchmark 10: Multiple updates
benchmark('Multiple updates (10 total)', () => {
  const ctx = new UEM256();
  for (let i = 0; i < 10; i++) {
    ctx.update('chunk');
  }
  ctx.digest();
}, 5000);

console.log('-'.repeat(70));
console.log('\nNote: Performance will vary based on:');
console.log('  - Node.js version and optimization level');
console.log('  - System CPU speed and load');
console.log('  - BigInt implementation efficiency');
console.log('  - Memory pressure and garbage collection');
