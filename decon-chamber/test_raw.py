import sys

from DMXEnttecPro import Controller
import time
import Adafruit_MPR121.MPR121 as MPR121
from rpi_ws281x import PixelStrip, Color

import os
import signal
from subprocess import Popen, PIPE, call
import psutil

# LED strip configuration:
LED_COUNT = 16  # Number of LED pixels.
# LED_PIN = 10          # GPIO pin connected to the pixels (18 uses PWM!).
LED_PIN = 10  # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10  # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255  # Set to 0 for darkest and 255 for brightest
LED_INVERT = False  # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0  # set to '1' for GPIOs 13, 19, 41, 45 or 53


def test_dmx():
    dmx = Controller('/dev/ttyUSB0')  # Typical of Linux
    last_up = False
    dmx.clear_channels()
    while True:
        print("turning on" if not last_up else "turning off")
        # dmx.set_channel(0, 255 if not last_up else 0)
        dmx.set_channel(1, 255 if not last_up else 0)
        dmx.set_channel(2, 255 if not last_up else 0)
        dmx.submit()
        last_up = not last_up
        time.sleep(1.0)
        
def test_dmx_addr():
    dmx = Controller('/dev/ttyUSB0')  # Typical of Linux
    last_up = False
    dmx.clear_channels()
    while True:
      dmx.set_channel(35, 180)
      dmx.submit()



def test_touch():
    cap = MPR121.MPR121()
    if not cap.begin():
        print('Error initializing MPR121.  Check your wiring!')
        sys.exit(1)
    last_touched = cap.touched()
    while True:
        current_touched = cap.touched()
        for i in range(5):
            pin_bit = 1 << i
            if current_touched & pin_bit and not last_touched & pin_bit:
                print('{0} touched!'.format(i))
            if not current_touched & pin_bit and last_touched & pin_bit:
                print('{0} released!'.format(i))
        last_touched = current_touched
        time.sleep(0.5)


def wheel(pos):
    """Generate rainbow colors across 0-255 positions."""
    if pos < 85:
        return Color(pos * 3, 255 - pos * 3, 0)
    elif pos < 170:
        pos -= 85
        return Color(255 - pos * 3, 0, pos * 3)
    else:
        pos -= 170
        return Color(0, pos * 3, 255 - pos * 3)


def rainbowCycle(strip, wait_ms=20, iterations=5):
    """Draw rainbow that uniformly distributes itself across all pixels."""
    for j in range(256 * iterations):
        for i in range(strip.numPixels()):
            strip.setPixelColor(i, wheel(
                (int(i * 256 / strip.numPixels()) + j) & 255)
                                )
        strip.show()
        time.sleep(wait_ms / 1000.0)


def test_neopixels():
    strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
    strip.begin()
    while True:
        rainbowCycle(strip)


def test_framebuff():
    print("Starting framebuff")
    splashproc = Popen(['fbi', '-T', '1', '-d', '/dev/fb0', '--noverbose', '-a', 'assets/black_background.jpg'],
                       stdin=PIPE)
    print("waiting")
    time.sleep(5.0)
    print("stopping framebuff")
    for proc in psutil.process_iter():
        if 'fbi' in proc.name():
            print(proc.pid)
            os.kill(proc.pid, signal.SIGTERM)
    print("exiting")


def test_vlc():
    os.system('clear')
    os.system('tput civis')
    splashproc = Popen(['fbi', '-T', '1', '-d', '/dev/fb0', '--noverbose', '-a', 'assets/black_background.jpg'],
                       stdin=PIPE)
    print("waiting to start vid")
    vid = Popen(['cvlc', '--no-osd', '--no-repeat', '--play-and-exit', '-f', 'assets/decon_video.mp4'], stdout=PIPE,
                preexec_fn=os.setsid)
    print("vid started")
    while vid.poll() is None:
        time.sleep(1.0)
        print("waiting for vid to end")
    print("stopping framebuff")
    for proc in psutil.process_iter():
        if 'fbi' in proc.name():
            print(proc.pid)
            os.kill(proc.pid, signal.SIGTERM)
    print("exiting")


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    test_dmx_addr()
