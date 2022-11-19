
from src.base.dmx import Dmx
from src.base.leds import Leds
from src.base.splash import Splash
from src.base.ticker import Ticker
from src.base.touch import Touch
from src.state.button import Buttons
from src.state.cans import Cans
from src.state.resetter import Resetter
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
        Buttons.setup()
        STATE_DICT[cls.state.value].enter(Ticker.millis)

    @classmethod
    def update_inputs(cls):
        Touch.update()
        Ticker.update()
        Buttons.update()
        Resetter.update()
        Cans.update()
        if Video.is_playing():
            Video.update()

    @classmethod
    def run(cls):
        cls.update_inputs()
        next_state = STATE_DICT[cls.state.value].run(Ticker.millis)
        if next_state is not None:
            cls.transition_state(next_state)
        cls.update_outputs()

    @classmethod
    def transition_state(cls, new_state: DeconState):
        STATE_DICT[cls.state.value].exit()
        cls.state = new_state
        STATE_DICT[cls.state.value].enter(Ticker.millis)

    @classmethod
    def update_outputs(cls):
        Dmx.update()
        Leds.update()

    @classmethod
    def teardown(cls):
        Dmx.teardown()
        Leds.teardown()
        Splash.teardown()
        Video.stop_video()
