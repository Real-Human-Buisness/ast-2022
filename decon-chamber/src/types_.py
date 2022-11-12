from __future__ import annotations
from dataclasses import dataclass


@dataclass
class RGB:
    R: int
    G: int
    B: int

    def __post_init__(self):
        self.R = max(min(self.R, 255), 0)
        self.G = max(min(self.G, 255), 0)
        self.B = max(min(self.B, 255), 0)

    # our lights are bgr...
    def get_int_color(self) -> int:
        return (self.B << 16) | (self.G << 8) | self.R

    def off(self):
        self.R = 0
        self.G = 0
        self.B = 0

    def copy(self, other: RGB):
        self.R = other.R
        self.G = other.G
        self.B = other.B
