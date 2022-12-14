import os
import signal
import sys
import time
from subprocess import Popen, PIPE
from typing import Optional

import psutil

from src.base.dmx import Dmx, ADDR_PROJECTOR
from src.state.button import ButtonState, Buttons


class Video:

    video_p: Optional[Popen] = None

    @classmethod
    def start_video(cls):
        asset_path = os.path.join(os.path.dirname(__file__), '..', '..', 'assets', 'decon_video.mp4')
        cls.video_p = Popen(
            ['cvlc', '--no-osd', '--no-repeat', '--play-and-exit', '-f', asset_path],
            stdout=PIPE, preexec_fn=os.setsid
        )
        Dmx.set_channel(ADDR_PROJECTOR, 255)

    @classmethod
    def update(cls):
        if cls.video_p is not None:
            if cls.video_p.poll() is not None:
                # video is stopped
                cls.video_p = None
                # stop projector
                Dmx.set_channel(ADDR_PROJECTOR, 0)
                if Buttons.get_main_state() == ButtonState.SOLID:
                    Buttons.set_state(4, ButtonState.FADE_OUT)
                else:
                    Buttons.set_state(4, ButtonState.OFF)

    @classmethod
    def is_playing(cls):
        return cls.video_p is not None

    @classmethod
    def stop_video(cls):
        if cls.video_p is not None:
            for proc in psutil.process_iter():
                p_name = proc.name()
                if 'cvlc' in p_name or 'vlc' in p_name:
                    os.kill(proc.pid, signal.SIGTERM)
                    cls.video_p = None
            Dmx.set_channel(ADDR_PROJECTOR, 0)
            if Buttons.get_main_state() == ButtonState.SOLID:
                Buttons.set_state(4, ButtonState.FADE_OUT)
            else:
                Buttons.set_state(4, ButtonState.OFF)


if __name__ == '__main__':
    print("starting video")
    Video.start_video()
    for i in range(10):
        print("playing")
        Video.update()
        if not Video.is_playing():
            print("ahhhh shit, it quit early")
            sys.exit(1)
        time.sleep(1.0)

    print("killing in the name of")
    Video.stop_video()
    time.sleep(0.5)

    if Video.is_playing():
        print("couldn't pull the trigger D:")
    else:
        print("dunuh, derr, derrrurnnh")
