from __future__ import annotations
from dataclasses import dataclass


@dataclass
class RGB:
    R: int
    G: int
    B: int

    def _min_max(self):
        self.R = max(min(self.R, 255), 0)
        self.G = max(min(self.G, 255), 0)
        self.B = max(min(self.B, 255), 0)

    def __post_init__(self):
        self._min_max()

    # our lights are bgr...
    def get_int_color(self) -> int:
        return (self.G << 16) | (self.B << 8) | self.R

    def off(self):
        self.R = 0
        self.G = 0
        self.B = 0

    def copy(self, other: RGB):
        self.R = other.R
        self.G = other.G
        self.B = other.B

    def fade_from(self, other: RGB, pct: float):
        self.R = int(other.R * (1 - pct))
        self.G = int(other.G * (1 - pct))
        self.B = int(other.B * (1 - pct))
        self._min_max()

    def fade_to(self, other: RGB, pct: float):
        self.R = int(other.R * pct)
        self.G = int(other.G * pct)
        self.B = int(other.B * pct)
        self._min_max()

    def fade_between(self, first: RGB, second: RGB, pct: float):
        self.R = int(first.R * (1 - pct) + second.R * pct)
        self.G = int(first.G * (1 - pct) + second.G * pct)
        self.B = int(first.B * (1 - pct) + second.B * pct)
        self._min_max()
