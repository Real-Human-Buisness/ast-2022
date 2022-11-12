
import sys
import time

import Adafruit_MPR121.MPR121 as MPR121


class Touch:
    cap: MPR121.MPR121
    is_touched: tuple[bool, bool, bool, bool, bool]

    @classmethod
    def setup(cls):
        cls.cap = MPR121.MPR121()
        if not cls.cap.begin():
            print('Error initializing MPR121.  Check your wiring!')
            sys.exit(1)
        cls.update()

    @classmethod
    def update(cls):
        is_touched = cls.cap.touched()
        cls.is_touched = tuple(bool(is_touched >> i & 1) for i in range(5))


if __name__ == '__main__':
    Touch.setup()
    last_touched = Touch.is_touched
    while True:
        Touch.update()
        current_touched = Touch.is_touched
        for i in range(5):
            if current_touched[i] and not last_touched[i]:
                print('{0} touched!'.format(i))
            if not current_touched[i] and last_touched[i]:
                print('{0} released!'.format(i))
        last_touched = current_touched
        time.sleep(0.1)
