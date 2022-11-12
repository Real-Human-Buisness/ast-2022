from enum import Enum
from typing import List

from src.base.leds import Leds
from src.base.ticker import Ticker
from src.base.touch import Touch
from src.types_ import RGB


class ButtonState(Enum):
    OFF = 1
    SELECT_ME = 2
    SELECTED = 3
    SOLID = 4
    FADE_OUT = 6


class Button:

    def __init__(self, position: int, start_pixel: int, end_pixel: int, color: RGB):
        self.state = ButtonState.OFF
        self.is_touched = False
        self.position = position
        self.start = start_pixel
        self.end = end_pixel
        self.color = color
        self.current_color = RGB(0, 0, 0)
        self.last_color = RGB(0, 0, 0)
        self.last_timestamp = 0

    def update(self):
        self.is_touched = Touch.is_touched[self.position]
        self.run_state()
        Leds.set_group_color(self.start, self.end, self.current_color)

    def transition_to_state(self, state: ButtonState):
        self.last_timestamp = Ticker.millis
        self.last_color = self.current_color
        self.state = state

    def run_state(self):
        if self.state == ButtonState.OFF:
            self.current_color.off()
        elif self.state == ButtonState.SOLID:
            self.current_color.copy(self.color)
        elif self.state == ButtonState.FADE_OUT:
            self.run_fade_out()
        elif self.state == ButtonState.SELECT_ME:
            self.run_select_me()
        elif self.state == ButtonState.SELECTED:
            self.run_selected()

    def run_fade_out(self):
        pass

    def run_select_me(self):
        pass

    def run_selected(self):
        pass


class Buttons:
    main_button: Button
    numbered_buttons: List[Button]

    @classmethod
    def setup(cls):
        cls.main_button = Button(4, 17, 19, RGB(255, 197, 143))
        cls.numbered_buttons = [
            Button(0, 0, 2, RGB(255, 0, 0)),
            Button(1, 4, 6, RGB(255, 255, 0)),
            Button(2, 8, 10, RGB(0, 255, 0)),
            Button(3, 12, 14, RGB(0, 0, 255)),
        ]

    @classmethod
    def update(cls):
        cls.main_button.update()
        for b in cls.numbered_buttons:
            b.update()

    @classmethod
    def get_main_push(cls) -> bool:
        return cls.main_button.is_touched

    @classmethod
    def get_numbered_push(cls) -> int:
        for i, b in enumerate(cls.numbered_buttons):
            if b.is_touched:
                return i
        return -1

    @classmethod
    def set_state(cls, position: int, state: ButtonState):
        if position == 4:
            cls.main_button.transition_to_state(state)
        else:
            cls.numbered_buttons[position].transition_to_state(state)

    @classmethod
    def select_numbered_button(cls, position: int):
        for i, b in enumerate(cls.numbered_buttons):
            if i == position:
                b.transition_to_state(ButtonState.SELECTED)
            else:
                b.transition_to_state(ButtonState.FADE_OUT)

    @classmethod
    def set_state_numbered(cls, state: ButtonState):
        for b in cls.numbered_buttons:
            b.transition_to_state(state)