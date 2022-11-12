import os
import signal
import sys
import time

from src.run import Decon


def signal_handler(sig, frame):
    Decon.teardown()
    sys.exit(0)


def main():
    if os.environ['IS_LOCAL']:
        print("waiting ghettoly for fb")
        time.sleep(10.0)
        print("done waiting")
    Decon.setup()
    signal.signal(signal.SIGINT, signal_handler)
    while True:
        Decon.run()


if __name__ == '__main__':
    main()
