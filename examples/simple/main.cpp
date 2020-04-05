#include <PixelRing.h>

PixelRing<24> strip;

void setup()
{
    strip.setup();
    strip.on();
    strip.maxBrightness();
}

void loop()
{
   strip.process(SceneMode scene_mode = SceneMode::Rainbow);
}
