from enum import Enum
from typing import Optional

from src.base.dmx import Dmx, ADDR_PROJECTOR, ADDR_BOOSH
from src.state.button import ButtonState, Buttons
from src.state.cans import CanState, Cans
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
        print("waiting, enter")
        Cans.transition_to_state(CanState.ON)
        Buttons.set_state_numbered(ButtonState.OFF)
        Buttons.set_state(4, ButtonState.SELECT_ME)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if Buttons.get_main_push():
            return DeconState.PLAYING
        return None

    @classmethod
    def exit(cls):
        print("waiting, leave")
        # show we pushed the main button
        Buttons.set_state(4, ButtonState.SELECTED)


class PlayingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        print("playing, enter")
        Video.start_video()
        Cans.transition_to_state(CanState.OFF)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        Video.update()
        if not Video.is_playing():
            Dmx.set_channel(ADDR_PROJECTOR, 0)
            Buttons.set_state(4, ButtonState.FADE_OUT)
            return DeconState.SELECTING
        if millis - cls.enter_time > 36000:
            Buttons.set_state(0, ButtonState.SOLID)
        if millis - cls.enter_time > 37500:
            Buttons.set_state(1, ButtonState.SOLID)
        if millis - cls.enter_time > 39000:
            Buttons.set_state(2, ButtonState.SOLID)
        if millis - cls.enter_time > 40500:
            Buttons.set_state(3, ButtonState.SOLID)
        if millis - cls.enter_time > 43000:
            return DeconState.SELECTING
        return None

    @classmethod
    def exit(cls):
        print("playing, leave")
        pass


class SelectingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        print("selecting, enter")
        Buttons.set_state_numbered(ButtonState.SELECT_ME)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if Video.is_playing():
            Video.update()
            if not Video.is_playing():
                Buttons.set_state(4, ButtonState.FADE_OUT)
        selected_button = Buttons.get_numbered_push()
        if selected_button != -1:
            Buttons.select_numbered_button(selected_button)
            Cans.set_woosh(selected_button)
            Cans.transition_to_state(CanState.WOOSH)
            return DeconState.WOOSHING
        return None

    @classmethod
    def exit(cls):
        print("selecting, leave")
        # just in case, stop video and projector
        Video.stop_video()
        Buttons.set_state(4, ButtonState.FADE_OUT)


class WooshingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        print("wooshing, enter")
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if millis - cls.enter_time > 7300:
            print(millis, cls.enter_time, "exiting woosh")
            return DeconState.BOOSHING
        return None

    @classmethod
    def exit(cls):
        print("wooshing, leave")


class BooshingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        print("booshing, enter")
        Cans.transition_to_state(CanState.BOOSH)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if millis - cls.enter_time > 3000:
            return DeconState.EXITING
        return None

    @classmethod
    def exit(cls):
        print("booshing, leave")
        Dmx.dmx.set_channel(ADDR_BOOSH, 0)
        Cans.transition_to_state(CanState.OFF)


class ExitingState:

    enter_time: int

    @classmethod
    def enter(cls, millis: int):
        print("exiting, enter")
        Buttons.set_state_numbered(ButtonState.OFF)
        cls.enter_time = millis

    @classmethod
    def run(cls, millis: int) -> Optional[DeconState]:
        if millis - cls.enter_time > 3000:
            return DeconState.WAITING
        return None

    @classmethod
    def exit(cls):
        print("exiting, leave")
        Cans.transition_to_state(CanState.ON)


STATE_DICT = {
    DeconState.WAITING.value: WaitingState,
    DeconState.PLAYING.value: PlayingState,
    DeconState.SELECTING.value: SelectingState,
    DeconState.WOOSHING.value: WooshingState,
    DeconState.BOOSHING.value: BooshingState,
    DeconState.EXITING.value: ExitingState,
}