/**
 * Test suite for UEM-256 JavaScript implementation
 * Uses the same test vectors as the Python implementation
 */

const { UEM256, new: newHash, digest, hexdigest, hash } = require('./uem256.js');

const VECTORS = {
  '': 'ff524be2b601b4c42fd991416c21387105df2a0436c7bb2981481b5e4b44f0a9',
  'a': 'bb32eb5d34a3fcc5636f144012382083c49543d46c34fa9755225c4f063d0cdf',
  'hello world': 'aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375',
  'UEM-256': 'eb96958ffd15ab17f2f8af60f499cce94a82ff52eac1f3178256e20f643e5cb8',
};

function test(name, fn) {
  try {
    fn();
    console.log(`✓ ${name}`);
  } catch (err) {
    console.error(`✗ ${name}`);
    console.error(`  ${err.message}`);
    process.exit(1);
  }
}

function assertEqual(actual, expected, msg) {
  if (actual !== expected) {
    throw new Error(`${msg || 'Assertion failed'}\n  Expected: ${expected}\n  Got:      ${actual}`);
  }
}

test('hash() matches known vectors', () => {
  for (const [message, expected] of Object.entries(VECTORS)) {
    const result = hash(message);
    assertEqual(result, expected, `hash('${message}')`);
  }
});

test('digest() returns Buffer with correct hex', () => {
  for (const [message, expected] of Object.entries(VECTORS)) {
    const result = digest(message);
    assertEqual(result.toString('hex'), expected, `digest('${message}')`);
  }
});

test('hexdigest() returns correct hex string', () => {
  for (const [message, expected] of Object.entries(VECTORS)) {
    const result = hexdigest(message);
    assertEqual(result, expected, `hexdigest('${message}')`);
  }
});

test('hash is deterministic', () => {
  const payload = 'The quick brown fox jumps over the lazy dog';
  const first = hash(payload);
  const second = hash(payload);
  assertEqual(first, second, 'Hash should be deterministic');
});

test('output length is correct', () => {
  const payload = 'The quick brown fox jumps over the lazy dog';
  const hexResult = hash(payload);
  const digestResult = digest(payload);
  
  assertEqual(hexResult.length, 64, 'Hex digest should be 64 characters');
  assertEqual(digestResult.length, 32, 'Binary digest should be 32 bytes');
});

test('update() API produces same result as direct hash', () => {
  const payload = 'The quick brown fox jumps over the lazy dog';
  const chunks = [1, 2, 7, 13, payload.length];
  
  for (const chunkSize of chunks) {
    const ctx = new UEM256();
    for (let i = 0; i < payload.length; i += chunkSize) {
      ctx.update(payload.substring(i, i + chunkSize));
    }
    const chunkResult = ctx.hexdigest();
    const directResult = hexdigest(payload);
    assertEqual(chunkResult, directResult, `update() with chunk size ${chunkSize}`);
  }
});

test('update() can be chained', () => {
  const ctx = new UEM256();
  const result = ctx.update('hello').update(' ').update('world').hexdigest();
  const expected = hash('hello world');
  assertEqual(result, expected, 'Chained update() calls');
});

test('copy() creates independent context', () => {
  const ctx1 = new UEM256().update('hello');
  const ctx2 = ctx1.copy();
  ctx1.update(' world');
  ctx2.update(' there');
  
  const result1 = ctx1.hexdigest();
  const result2 = ctx2.hexdigest();
  
  if (result1 === result2) {
    throw new Error('copy() contexts should be independent');
  }
});

test('handles empty string', () => {
  const result = hash('');
  assertEqual(result, VECTORS[''], 'Empty string hash');
});

test('handles Buffer input', () => {
  const buf = Buffer.from('hello world');
  const result = hash(buf);
  assertEqual(result, hash('hello world'), 'Buffer input should work');
});

test('new() function creates new instance', () => {
  const ctx = newHash();
  if (!(ctx instanceof UEM256)) {
    throw new Error('new() should return UEM256 instance');
  }
  const result = ctx.update('test').hexdigest();
  const expected = hash('test');
  assertEqual(result, expected, 'new() creates working instance');
});

console.log('\n✓ All tests passed!');
