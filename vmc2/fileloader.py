import struct
from typing import List
from dataclasses import dataclass

@dataclass
class LoadedFile:
    bytes: bytes

class FileLoader:
    @staticmethod
    def load(path: str) -> LoadedFile:
        with open(path, "rb") as f:
            b = f.read()
        return LoadedFile(bytes=b)

    @staticmethod
    def validate_stak(b: bytes):
        if len(b) < 8:
            raise ValueError("File too small (need >=8 bytes header)")
        magic = b[0:4]
        if magic not in (b"STAK", b"KATS"):
            raise ValueError(f"Bad magic: {magic!r}, expected b'STAK' or b'KATS'")

    @staticmethod
    def read_le32_at(b: bytes, offset: int) -> int:
        # safe read little-endian 32-bit
        if offset + 4 > len(b):
            raise ValueError("read past EOF")
        return struct.unpack_from("<I", b, offset)[0]
