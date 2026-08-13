import unittest

import uem256


class UEM256LibraryTests(unittest.TestCase):
    def test_hash_matches_known_vectors(self):
        vectors = {
            "": "ff524be2b601b4c42fd991416c21387105df2a0436c7bb2981481b5e4b44f0a9",
            "a": "bb32eb5d34a3fcc5636f144012382083c49543d46c34fa9755225c4f063d0cdf",
            "hello world": "aceb882452062f9169f674aa6b03b8b326bada23ab625bc329b9c0775e536375",
            "UEM-256": "eb96958ffd15ab17f2f8af60f499cce94a82ff52eac1f3178256e20f643e5cb8",
        }

        for message, expected in vectors.items():
            self.assertEqual(uem256.hash(message), expected)
            self.assertEqual(uem256.digest(message), bytes.fromhex(expected))

    def test_hash_is_deterministic_and_length_safe(self):
        payload = b"The quick brown fox jumps over the lazy dog"
        first = uem256.hash(payload)
        second = uem256.hash(payload)

        self.assertEqual(first, second)
        self.assertEqual(len(first), 64)
        self.assertEqual(len(uem256.digest(payload)), 32)

    def test_update_api_matches_hash(self):
        payload = b"The quick brown fox jumps over the lazy dog"
        ctx = uem256.UEM256()
        for chunk in (1, 2, 7, 13, len(payload)):
            state = uem256.UEM256()
            for i in range(0, len(payload), chunk):
                state.update(payload[i : i + chunk])
            self.assertEqual(state.hexdigest(), uem256.hexdigest(payload))


if __name__ == "__main__":
    unittest.main()
