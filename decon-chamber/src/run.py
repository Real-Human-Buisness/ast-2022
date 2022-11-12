
from enum import Enum
from src.base.dmx import Dmx
from src.base.leds import Leds
from src.base.splash import Splash
from src.base.ticker import Ticker
from src.base.touch import Touch
from src.state.button import Buttons
from src.state.video import Video
from src.states import DeconState, STATE_DICT


class Decon:

    state: DeconState = DeconState.WAITING

    @classmethod
    def setup(cls):
        Splash.setup()
        Dmx.setup()
        Leds.setup()
        Touch.setup()
        Ticker.setup()
        STATE_DICT[cls.state].enter(Ticker.millis)

    @classmethod
    def update_inputs(cls):
        Touch.update()
        Ticker.update()
        Buttons.update()

    @classmethod
    def run(cls):
        cls.update_inputs()
        next_state = STATE_DICT[cls.state].run(Ticker.millis)
        if next_state is not None:
            cls.transition_state(next_state)
        cls.update_outputs()

    @classmethod
    def transition_state(cls, new_state: DeconState):
        STATE_DICT[cls.state].exit()
        cls.state = new_state
        STATE_DICT[cls.state].enter()

    @classmethod
    def update_outputs(cls):
        Dmx.update()
        Leds.update()

    @classmethod
    def teardown(cls):
        Dmx.teardown()
        Leds.teardown()
        Splash.teardown()
