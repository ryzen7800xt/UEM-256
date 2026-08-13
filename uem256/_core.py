from __future__ import annotations

from typing import Union

_MASK64 = (1 << 64) - 1
_INITIAL_STATE = (
    0x6D6574615F6D6172,
    0x7465735F7169735F,
    0x6E69755F4D654F6D,
    0x646F6D5F656E6F6D,
)


def _rotl64(value: int, shift: int) -> int:
    shift &= 63
    return ((value << shift) | (value >> (64 - shift))) & _MASK64


def _load64_le(data: bytes) -> int:
    return int.from_bytes(data[:8], byteorder="little", signed=False)


def _mix(state: list[int], block: int) -> None:
    state[0] ^= block

    for _ in range(2):
        state[0] = (state[0] + state[1]) & _MASK64
        state[1] = _rotl64(state[1], 13)
        state[1] ^= state[0]

        state[2] = (state[2] + state[3]) & _MASK64
        state[3] = _rotl64(state[3], 16)
        state[3] ^= state[2]

        state[0] = (state[0] + state[3]) & _MASK64
        state[3] = _rotl64(state[3], 21)
        state[3] ^= state[0]

        state[2] = (state[2] + state[1]) & _MASK64
        state[1] = _rotl64(state[1], 17)
        state[1] ^= state[2]

    state[0] ^= block


class UEM256:
    """Streaming UEM-256 hash context."""

    def __init__(self) -> None:
        self._state = list(_INITIAL_STATE)
        self._length = 0
        self._buffer = bytearray()

    def update(self, data: Union[str, bytes, bytearray, memoryview]) -> "UEM256":
        if isinstance(data, str):
            data = data.encode("utf-8")
        elif isinstance(data, memoryview):
            data = data.tobytes()
        elif isinstance(data, bytearray):
            data = bytes(data)
        elif data is None:
            return self
        elif not isinstance(data, (bytes, bytearray)):
            raise TypeError("UEM256.update expects bytes-like data or str")

        if not data:
            return self

        self._length += len(data)
        buffer = self._buffer

        if buffer:
            to_copy = min(8 - len(buffer), len(data))
            buffer.extend(data[:to_copy])
            data = data[to_copy:]
            if len(buffer) == 8:
                _mix(self._state, _load64_le(bytes(buffer)))
                buffer.clear()

        while len(data) >= 8:
            _mix(self._state, _load64_le(data[:8]))
            data = data[8:]

        if data:
            buffer.extend(data)

        return self

    def digest(self) -> bytes:
        state = self._state.copy()
        buffer = bytes(self._buffer)
        total_length = self._length

        tail = bytearray(8)
        tail[: len(buffer)] = buffer
        tail[len(buffer)] = 0x80
        tail[7] = total_length & 0xFF
        _mix(state, _load64_le(bytes(tail)))

        state[0] ^= total_length & _MASK64
        for i in range(4):
            _mix(state, state[i] ^ 0xFEDCBA9876543210)

        result = bytearray()
        for value in state:
            result.extend(value.to_bytes(8, byteorder="little", signed=False))
        return bytes(result)

    def hexdigest(self) -> str:
        return self.digest().hex()

    def copy(self) -> "UEM256":
        clone = UEM256()
        clone._state = self._state.copy()
        clone._length = self._length
        clone._buffer = bytearray(self._buffer)
        return clone

    def final(self) -> bytes:
        return self.digest()


def new() -> UEM256:
    return UEM256()


def digest(data: Union[str, bytes, bytearray, memoryview]) -> bytes:
    return UEM256().update(data).digest()


def hexdigest(data: Union[str, bytes, bytearray, memoryview]) -> str:
    return UEM256().update(data).hexdigest()


def hash(data: Union[str, bytes, bytearray, memoryview]) -> str:
    return UEM256().update(data).hexdigest()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Compute a UEM-256 digest.")
    parser.add_argument("message", nargs="?", default="", help="Text to hash")
    args = parser.parse_args()
    print(hash(args.message))
