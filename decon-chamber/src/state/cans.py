from enum import Enum

from src.base.dmx import Dmx
from src.base.ticker import Ticker
from src.types_ import RGB


class CanState(Enum):
    ON = 1
    OFF = 2
    WOOSH = 3
    BOOSH = 4


COLORS = [
    RGB(0, 255, 0),
    RGB(255, 255, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 0)
]


class Cans:
    state: CanState = CanState.ON
    base_color: RGB = RGB(255, 197, 143)
    current_color: RGB = RGB(0, 0, 0)
    last_timestamp: int = 0
    woosh_position: int = -1

    @classmethod
    def set_woosh(cls, position: int):
        Cans.woosh_position = position

    @classmethod
    def transition_to_state(cls, state: CanState):
        cls.last_timestamp = Ticker.millis
        cls.state = state

    @classmethod
    def write_all(cls, color: RGB):
        for i in range(4):
            Dmx.set_channel(i * 8, color.R)
            Dmx.set_channel(i * 8 + 1, color.G)
            Dmx.set_channel(i * 8 + 2, color.B)

    @classmethod
    def write_woosh(cls, position: int):
        color = COLORS[cls.woosh_position]
        Dmx.set_channel(position * 8, color.R)
        Dmx.set_channel(position * 8 + 1, color.G)
        Dmx.set_channel(position * 8 + 2, color.B)
        # Dmx.set_channel(position * 8 + 7, 0)

    @classmethod
    def update(cls):
        if cls.state == CanState.ON:
            cls.write_all(cls.base_color)
        elif cls.state == CanState.OFF:
            cls.current_color.off()
            cls.write_all(cls.current_color)
        elif cls.state == CanState.WOOSH:
            cls.run_woosh()
        elif cls.state == CanState.BOOSH:
            cls.run_boosh()

    @classmethod
    def run_boosh(cls):
        color = COLORS[cls.woosh_position]
        cls.current_color.off()
        cls.write_all(cls.current_color)
        Dmx.set_channel(3 * 8, color.R)
        Dmx.set_channel(3 * 8 + 1, color.G)
        Dmx.set_channel(3 * 8 + 2, color.B)
        # Dmx.set_channel(3 * 8 + 7, 127)

    @classmethod
    def run_woosh(cls):
        cls.current_color.off()
        cls.write_all(cls.current_color)
        diff = Ticker.millis - cls.last_timestamp
        if diff < 1600:
            cls.write_woosh(int(diff / 400))
        elif diff < 3600:
            diff -= 1600
            cls.write_woosh(int(diff / 250) % 4)
        elif diff < 5200:
            diff -= 3600
            cls.write_woosh(int(diff / 200) % 4)
        else:
            diff -= 5200
            cls.write_woosh(int(diff / 175) % 4)



