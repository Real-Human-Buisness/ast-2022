from src.base.dmx import Dmx
from src.base.ticker import Ticker


class Boosher:
    is_booshing: False
    boosh_start = 0

    @classmethod
    def start_booshing(cls):
        cls.is_booshing = True
        cls.boosh_start = Ticker.millis

    @classmethod
    def update(cls):
        if cls.is_booshing:
            # do boosh
            pass

    @classmethod
    def stop_booshing(cls):
        cls.is_booshing = False
        Dmx.dmx.set_channel(64, 0)
