import time
from datetime import datetime, timedelta


class Ticker:

    start_time: datetime
    millis: int

    @classmethod
    def setup(cls):
        cls.start_time = datetime.now()
        cls.millis = 0

    @classmethod
    def update(cls):
        cls.millis = int((datetime.now() - cls.start_time) / timedelta(milliseconds=1))


if __name__ == '__main__':
    Ticker.setup()
    while True:
        time.sleep(0.333)
        Ticker.update()
        print(Ticker.millis)
