import os
import signal
import sys
import time

from src.run import Decon


def signal_handler(sig, frame):
    Decon.teardown()
    sys.exit(0)


def main():
    Decon.setup()
    signal.signal(signal.SIGINT, signal_handler)
    while True:
        Decon.run()


if __name__ == '__main__':
    main()
