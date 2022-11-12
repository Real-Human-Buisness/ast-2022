import os

from src.base.ticker import Ticker
from src.base.touch import Touch


class Resetter:

    is_armed = False
    armed_at = 0

    @classmethod
    def update(cls):
        if Touch.is_touched == (True, True, True, True, False):
            if not cls.is_armed:
                print("armed")
                cls.is_armed = True
                cls.armed_at = Ticker.millis
            else:
                if Ticker.millis - cls.armed_at > 5000:
                    os.system('reboot now')
        else:
            cls.is_armed = False
