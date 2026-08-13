#!/usr/bin/env python3
"""
ULTIMATE UEM-256 TEST SUITE
Demonstrates all available functions and capabilities.
"""

import uem256
import sys

def print_section(title):
    """Print a formatted section header."""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


def test_one_shot_functions():
    """Test all one-shot hashing functions."""
    print_section("ONE-SHOT FUNCTIONS")
    
    data = "Hello, World!"
    
    # hexdigest - hex string output
    print("1. hexdigest(data) → hex string (256-bit)")
    result = uem256.hexdigest(data)
    print(f"   Input:  {data!r}")
    print(f"   Output: {result}")
    print(f"   Length: {len(result)} characters (64 hex chars = 32 bytes)")
    
    # hash - alias for hexdigest
    print("\n2. hash(data) → hex string (alias for hexdigest)")
    result = uem256.hash(data)
    print(f"   Input:  {data!r}")
    print(f"   Output: {result}")
    
    # digest - raw bytes output
    print("\n3. digest(data) → raw bytes (32-byte digest)")
    result = uem256.digest(data)
    print(f"   Input:  {data!r}")
    print(f"   Output: {result}")
    print(f"   Length: {len(result)} bytes")


def test_streaming_api():
    """Test the streaming context API."""
    print_section("STREAMING API (Chunked Hashing)")
    
    # Basic streaming
    print("1. Streaming with multiple updates")
    ctx = uem256.UEM256()
    ctx.update("Hello")
    ctx.update(", ")
    ctx.update("World!")
    streaming_result = ctx.hexdigest()
    
    single_result = uem256.hexdigest("Hello, World!")
    
    print(f"   Streaming:      {streaming_result}")
    print(f"   Single-shot:    {single_result}")
    print(f"   ✓ Match: {streaming_result == single_result}")
    
    # Chaining
    print("\n2. Method chaining with .update()")
    ctx = uem256.UEM256()
    result = ctx.update("Chunk1").update("Chunk2").update("Chunk3").hexdigest()
    print(f"   Chained result: {result}")
    
    # Different data types
    print("\n3. Handling different input types")
    test_inputs = [
        ("String", "test string"),
        ("Bytes", b"test bytes"),
        ("Bytearray", bytearray(b"test bytearray")),
    ]
    
    for input_type, data in test_inputs:
        ctx = uem256.UEM256()
        result = ctx.update(data).hexdigest()
        print(f"   {input_type:15} → {result}")


def test_context_methods():
    """Test context-specific methods."""
    print_section("CONTEXT METHODS")
    
    # new() function
    print("1. new() → Create fresh context")
    ctx = uem256.new()
    result = ctx.update("fresh context").hexdigest()
    print(f"   Result: {result}")
    
    # digest() vs hexdigest()
    print("\n2. digest() vs hexdigest()")
    ctx = uem256.UEM256()
    ctx.update("data")
    
    digest_result = ctx.digest()
    ctx = uem256.UEM256()
    ctx.update("data")
    hexdigest_result = ctx.hexdigest()
    
    print(f"   digest():     {type(digest_result).__name__} ({len(digest_result)} bytes) = {digest_result[:16]}...")
    print(f"   hexdigest():  {type(hexdigest_result).__name__} ({len(hexdigest_result)} chars) = {hexdigest_result}")
    
    # final() - alias for digest()
    print("\n3. final() → Alias for digest()")
    ctx = uem256.UEM256().update("test")
    result = ctx.final()
    print(f"   Result: {type(result).__name__} ({len(result)} bytes) = {result[:16]}...")
    
    # copy() - clone context
    print("\n4. copy() → Clone context at any point")
    ctx = uem256.UEM256()
    ctx.update("initial")
    
    ctx_copy = ctx.copy()
    ctx.update("_branch1")
    ctx_copy.update("_branch2")
    
    print(f"   Original branch: {ctx.hexdigest()}")
    print(f"   Copy branch:     {ctx_copy.hexdigest()}")
    print(f"   ✓ Different: {ctx.hexdigest() != ctx_copy.hexdigest()}")


def test_determinism():
    """Test that hashing is deterministic."""
    print_section("DETERMINISM & CONSISTENCY")
    
    test_data = [
        "Hello",
        "",
        "🚀 Unicode test",
        "x" * 1000,
        "\n\t special\r\ncharacters\0here",
    ]
    
    for data in test_data:
        results = [uem256.hexdigest(data) for _ in range(3)]
        is_consistent = all(r == results[0] for r in results)
        
        display = data if len(data) <= 30 else data[:27] + "..."
        print(f"   {display:35} → Consistent: {is_consistent} ✓")


def test_edge_cases():
    """Test edge cases and special inputs."""
    print_section("EDGE CASES")
    
    # Empty string
    print("1. Empty string")
    result = uem256.hexdigest("")
    print(f"   Result: {result}")
    
    # Large input
    print("\n2. Large input (1MB)")
    large_data = "x" * (1024 * 1024)
    result = uem256.hexdigest(large_data)
    print(f"   Result: {result}")
    
    # Unicode
    print("\n3. Unicode handling")
    unicode_tests = [
        "Hello 世界",
        "🎉🎊🎈",
        "Привет мир",
        "مرحبا العالم",
    ]
    for text in unicode_tests:
        result = uem256.hexdigest(text)
        print(f"   {text:20} → {result}")


def test_practical_use_cases():
    """Demonstrate practical applications."""
    print_section("PRACTICAL USE CASES")
    
    # Cache key generation
    print("1. Cache key generation")
    urls = ["api.example.com/users/1", "api.example.com/users/2"]
    for url in urls:
        cache_key = uem256.hexdigest(url)
        print(f"   {url} → cache_{cache_key[:8]}")
    
    # Deduplication
    print("\n2. Deduplication pipeline")
    documents = ["doc1", "doc2", "doc1"]  # doc1 appears twice
    hashes = {uem256.hexdigest(doc): doc for doc in documents}
    print(f"   Input:  {documents}")
    print(f"   Unique: {list(hashes.values())} (deduplicated)")
    
    # File fingerprinting
    print("\n3. File fingerprinting")
    files = {"main.py": "import os\nprint('hello')", "config.json": '{"version": 1}'}
    for name, content in files.items():
        fp = uem256.hexdigest(content)
        print(f"   {name:20} → {fp}")


def test_comparison_with_different_inputs():
    """Show how similar inputs produce different hashes."""
    print_section("HASH SENSITIVITY")
    
    base = "password"
    variants = [
        base,
        base + " ",
        base.capitalize(),
        base[::-1],
    ]
    
    print(f"   Base: {base!r}\n")
    for variant in variants:
        h = uem256.hexdigest(variant)
        print(f"   {variant:20} → {h}")


def main():
    """Run all tests."""
    print("\n" + "🔐 "*20)
    print("ULTIMATE UEM-256 TEST SUITE - All Features & Functions")
    print("🔐 "*20)
    
    try:
        test_one_shot_functions()
        test_streaming_api()
        test_context_methods()
        test_determinism()
        test_edge_cases()
        test_practical_use_cases()
        test_comparison_with_different_inputs()
        
        print_section("✓ ALL TESTS COMPLETED SUCCESSFULLY")
        return 0
    except Exception as e:
        print(f"\n❌ ERROR: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())