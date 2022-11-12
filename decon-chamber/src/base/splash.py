import os
import signal
import time
from subprocess import Popen, PIPE
from typing import Optional

import psutil


class Splash:

    proc: Optional[Popen] = None

    @classmethod
    def setup(cls):
        os.system('clear')
        os.system('tput civis')
        asset_path = os.path.join(os.path.dirname(__file__), '..', '..', 'assets', 'black_background.jpg')
        if os.environ['IS_LOCAL']:
            cls.proc = Popen(
                ['fbi', '--noverbose', '-a', asset_path],
                stdin=PIPE
            )
        else:
            cls.proc = Popen(
                ['fbi', '-T', '1', '-d', '/dev/fb0', '--noverbose', '-a', asset_path],
                stdin=PIPE
            )

    @classmethod
    def teardown(cls):
        if cls.proc is not None:
            for proc in psutil.process_iter():
                if 'fbi' in proc.name():
                    print(proc.pid)
                    os.kill(proc.pid, signal.SIGTERM)


if __name__ == '__main__':
    print("running splash screen")
    Splash.setup()
    print("waiting...")
    time.sleep(5.0)
    print("closing")
    Splash.teardown()
    time.sleep(1.0)
    print("exiting")
