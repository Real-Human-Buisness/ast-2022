import sys
import time
from typing import Optional
from rpi_ws281x import PixelStrip, Color

from src.types_ import RGB


# LED strip configuration:

LED_COUNT = 16  # Number of LED pixels.
LED_PIN = 10  # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10  # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255  # Set to 0 for darkest and 255 for brightest
LED_INVERT = False  # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0  # set to '1' for GPIOs 13, 19, 41, 45 or 53


class Leds:

    strip: Optional[PixelStrip] = None

    @classmethod
    def setup(cls):
        cls.strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
        try:
            cls.strip.begin()
        except Exception as e:
            print(e)
            print("failed to start leds")
            sys.exit(1)

    @classmethod
    def set_group_color(cls, start: int, end: int, color: RGB):
        for i in range(start, end + 1):
            cls.strip.setPixelColor(i, color.get_int_color())

    @classmethod
    def update(cls):
        cls.strip.show()

    @classmethod
    def teardown(cls):
        if cls.strip is not None:
            for i in range(cls.strip.numPixels()):
                cls.strip.setPixelColor(i, 0)


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


def rainbowCycle(pos):
    """Draw rainbow that uniformly distributes itself across all pixels."""
    for i in range(Leds.strip.numPixels()):
        Leds.strip.setPixelColor(i, wheel(
            (int(i * 256 / Leds.strip.numPixels()) + pos) & 255)
        )


if __name__ == '__main__':
    Leds.setup()
    position = 0
    while True:
        rainbowCycle(position)
        Leds.update()
        time.sleep(20 / 1000.0)
        position = (position + 1) % 256
