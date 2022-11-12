from enum import Enum
from typing import Optional

from src.base.dmx import Dmx
from src.state.button import ButtonState, Buttons
from src.state.video import Video


class DeconState(Enum):
    WAITING = 1
    PLAYING = 2
    SELECTING = 3
    WOOSHING = 4
    BOOSHING = 5
    EXITING = 6


class WaitingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        # set cans to chill
        Buttons.set_state_numbered(ButtonState.OFF)
        Buttons.set_state(4, ButtonState.SELECT_ME)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if Buttons.get_main_push():
            return DeconState.PLAYING

    @classmethod
    def exit(cls):
        # show we pushed the main button
        Buttons.set_state(4, ButtonState.SELECTED)


class PlayingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        Video.start_video()
        # starts projector
        Dmx.dmx.set_channel(65, 255)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        Video.update()
        # after n seconds, turn each button to solid
        # afterwards, transition to selecting while video is still rolling
        return None

    @classmethod
    def exit(cls):
        pass


class SelectingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        Buttons.set_state_numbered(ButtonState.SELECT_ME)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        selected_button = Buttons.get_numbered_push()
        if selected_button != -1:
            # set cans to run routine n
            return DeconState.WOOSHING

    @classmethod
    def exit(cls):
        # just in case, stop video and projector
        Video.stop_video()
        Dmx.dmx.set_channel(65, 0)


class WooshingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        pass

    @classmethod
    def exit(cls):
        pass


class BooshingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        return None

    @classmethod
    def exit(cls):
        # stop the boosh
        Dmx.dmx.set_channel(64, 0)


class ExitingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        pass

    @classmethod
    def exit(cls):
        pass


STATE_DICT = {
    DeconState.WAITING: WaitingState,
    DeconState.PLAYING: PlayingState,
    DeconState.SELECTING: SelectingState,
    DeconState.WOOSHING: WooshingState,
    DeconState.BOOSHING: BooshingState,
    DeconState.EXITING: ExitingState,
}