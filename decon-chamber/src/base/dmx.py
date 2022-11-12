
from __future__ import annotations

import sys
from typing import List, Optional

from DMXEnttecPro import Controller

ADDR_BOOSH = 34
ADDR_PROJECTOR = 35


class Dmx:

    dmx: Optional[Controller] = None

    @classmethod
    def setup(cls):
        try:
            cls.dmx = Controller('/dev/ttyUSB0')
        except Exception as e:
            print(e)
            print("failed to start dmx controller")
            sys.exit(1)

    @classmethod
    def set_channel(cls, p: int, v: int):
        cls.dmx.set_channel(p + 1, v)

    @classmethod
    def update(cls):
        cls.dmx.submit()

    @classmethod
    def teardown(cls):
        if cls.dmx is not None:
            cls.dmx.clear_channels()
            cls.dmx.submit()
            cls.dmx.close()
            cls.dmx = None


