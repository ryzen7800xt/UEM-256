#!/usr/bin/env python3
"""
UEM-256 Fuzz Tester for Robustness

This script performs fuzz testing on the UEM-256 hash algorithm by:
- Generating random input data of various lengths
- Testing edge cases (empty, large, repetitive patterns)
- Verifying deterministic output for same inputs
- Checking for crashes or unexpected behavior

Usage: python tests/fuzz_uem256.py
"""

import random
import sys
import time


def rotl64(value, shift):
    return ((value << shift) & ((1 << 64) - 1)) | (value >> (64 - shift))


def load64_le(data):
    return int.from_bytes(data, 'little')


def uem256_mix(state, block):
    state[0] ^= block
    for _ in range(2):
        state[0] = (state[0] + state[1]) & ((1 << 64) - 1)
        state[1] = rotl64(state[1], 13)
        state[1] ^= state[0]
        state[2] = (state[2] + state[3]) & ((1 << 64) - 1)
        state[3] = rotl64(state[3], 16)
        state[3] ^= state[2]
        state[0] = (state[0] + state[3]) & ((1 << 64) - 1)
        state[3] = rotl64(state[3], 21)
        state[3] ^= state[0]
        state[2] = (state[2] + state[1]) & ((1 << 64) - 1)
        state[1] = rotl64(state[1], 17)
        state[1] ^= state[2]
    state[0] ^= block


def uem256_hash(data):
    state = [0x6d6574615f6d6172, 0x7465735f7169735f,
             0x6e69755f4d654f6d, 0x646f6d5f656e6f6d]
    buf = bytearray()
    total_len = len(data)
    i = 0
    while i < len(data):
        if len(buf) > 0:
            to_copy = min(8 - len(buf), len(data) - i)
            buf.extend(data[i:i + to_copy])
            i += to_copy
            if len(buf) == 8:
                uem256_mix(state, load64_le(buf))
                buf.clear()
            continue
        if i + 8 <= len(data):
            uem256_mix(state, load64_le(data[i:i + 8]))
            i += 8
        else:
            buf.extend(data[i:])
            i = len(data)
    tail = bytearray(8)
    tail[:len(buf)] = buf
    tail[len(buf)] = 0x80
    tail[7] = total_len & 0xFF
    uem256_mix(state, load64_le(tail))
    state[0] ^= total_len
    for j in range(4):
        uem256_mix(state, state[j] ^ 0xFEDCBA9876543210)
    return b''.join(x.to_bytes(8, 'little') for x in state)


def generate_random_data(length):
    """Generate random bytes of specified length."""
    return bytes(random.randint(0, 255) for _ in range(length))


def generate_edge_case_data():
    """Generate edge case inputs."""
    cases = [
        b'',  # Empty
        b'\x00',  # Single null
        b'\xFF',  # Single FF
        b'A' * 1,  # Single repeated char
        b'A' * 8,  # Block size
        b'A' * 64,  # Larger block
        b'A' * 1000,  # Large input
        bytes(range(256)),  # All byte values
        b'\x00' * 100,  # Null bytes
        b'\xFF' * 100,  # FF bytes
    ]
    return cases


def fuzz_test(iterations=10000):
    """Run fuzz tests."""
    print(f"Starting UEM-256 fuzz test with {iterations} iterations...")
    start_time = time.time()

    # Test deterministic behavior
    test_data = generate_random_data(100)
    hash1 = uem256_hash(test_data)
    hash2 = uem256_hash(test_data)
    if hash1 != hash2:
        print("FAIL: Non-deterministic output for same input")
        return False

    # Edge cases
    for i, data in enumerate(generate_edge_case_data()):
        try:
            hash_result = uem256_hash(data)
            if len(hash_result) != 32:
                print(
                    f"FAIL: Invalid hash length for edge case {i}: {len(hash_result)}")
                return False
        except Exception as e:
            print(f"FAIL: Exception in edge case {i}: {e}")
            return False

    # Random fuzzing
    for i in range(iterations):
        length = random.randint(0, 10000)  # Up to 10KB
        data = generate_random_data(length)
        try:
            hash_result = uem256_hash(data)
            if len(hash_result) != 32:
                print(
                    f"FAIL: Invalid hash length at iteration {i}: {len(hash_result)}")
                return False
            # Verify deterministic
            hash_again = uem256_hash(data)
            if hash_result != hash_again:
                print(f"FAIL: Non-deterministic at iteration {i}")
                return False
        except Exception as e:
            print(f"FAIL: Exception at iteration {i}: {e}")
            return False

        if (i + 1) % 1000 == 0:
            print(f"Completed {i + 1}/{iterations} iterations...")

    elapsed = time.time() - start_time
    print(".2f")
    return True


if __name__ == "__main__":
    success = fuzz_test()
    if success:
        print("PASS: All fuzz tests completed successfully.")
        sys.exit(0)
    else:
        print("FAIL: Fuzz tests failed.")
        sys.exit(1)
