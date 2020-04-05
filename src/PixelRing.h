#pragma once

#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>

//--------------------------------------------------------------------------------------------------

template <uint16_t LED_COUNT = 16, uint8_t LED_PIN = D0> class PixelRing
{
public:
    enum class SceneMode
    {
        White,
        Red,
        Green,
        Blue,
        TheaterChaseWhite,
        TheaterChaseRed,
        TheaterChaseBlue,
        TheaterChaseRainbow,
        Rainbow,
        Off,
        None // does not touch anything but maintains the previous state
    };

    void setup();

    void process(SceneMode scene_mode = SceneMode::None);

    //! Increments the brightness by maximum +/-20 %
    //! \param increment percentage to in-/decrement
    void incrementBrightness(int8_t increment);

    void maxBrightness();

    //! toggles strip on and off
    //! \return true if strip is toggled on
    bool toggleOnOff();

    void off();

    void on();

    //! Increments the arc by maximum +/- strip.numPixels()
    //! \param pixels number of pixels to in-/decrement the arc width
    void incrementWidth(int8_t pixels);

    void fullWidth();

    //! Shifts (rotates) the arc.
    //! \param pixels number of pixels to shift for-/backward
    void shift(int8_t pixels);

    //! Scrolls to the next scene mode: White, Red, ..., Rainbow, White, ... etc.
    void nextScene();

private:
    //! Arc based abstraction of the strip.
    struct ArcBasedView
    {
        ArcBasedView(Adafruit_NeoPixel &strip);

        void process(uint32_t color);

        void process();

        void rotate(int8_t pixels = 1);

        void incrementArc(int8_t pixels = 1);

        void fullWidth();

    private:
        void incrementArcByOne(bool do_increment);

        Adafruit_NeoPixel &strip;
        uint32_t color{ 0 };
        uint8_t begin : 4;
        uint8_t end : 4;

        //! 4-bit pixel iterator with intended over-/underflow
        uint8_t pixel_iterator : 4;
        //! toggle bit to ensures alternate access (left, right
        uint8_t toggle : 1;
    };

    uint8_t overrideColorChannelBrightness(uint8_t color);

    uint32_t overrideColorBrightness(uint32_t color);

    //! Puts the given color on the whole strip wrt. to the current brightness.
    //! \param color the color on strip
    void colorWipe(uint32_t color);


    void theaterChase(uint32_t color, uint16_t wait_ms);

    void rainbow(uint16_t wait_ms);

    void theaterChaseRainbow(uint16_t wait_ms);

    using Strip = Adafruit_NeoPixel;
    Strip strip{ LED_COUNT, LED_PIN, (NEO_GRB) + NEO_KHZ800 };

    //! 0-100 [%]
    uint8_t brightness{ 100 };
    //! 0-1 (on, off)
    uint8_t brightness_override{ 1 };

    SceneMode last_scene_mode = { SceneMode::Rainbow };
    //! timer to measure elapsed time in [ms] since set to 0
    elapsedMillis time_elapsed{ 0 };

    //! arc based abstraction of the strip
    ArcBasedView arc_view{ strip };
};


// -------------------------------------------------------------------------------------------------
template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::setup()
{
    Serial.println("PixelRing::setup");
    strip.begin();
    strip.show();
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::process(PixelRing::SceneMode scene_mode)
{
    last_scene_mode = (scene_mode == SceneMode::None) ? last_scene_mode : scene_mode;
    switch(last_scene_mode)
    {
    case SceneMode::Off:
        colorWipe(Strip::Color(0, 0, 0));
        break;
    case SceneMode::Red:
        arc_view.process(overrideColorBrightness(Strip::Color(255, 0, 0)));
        break;
    case SceneMode::Green:
        arc_view.process(overrideColorBrightness(Strip::Color(0, 255, 0)));
        break;
    case SceneMode::Blue:
        arc_view.process(overrideColorBrightness(Strip::Color(0, 0, 255)));
        break;
    case SceneMode::White:
        arc_view.process(overrideColorBrightness(Strip::Color(255, 255, 255)));
        break;
    case SceneMode::TheaterChaseWhite:
        theaterChase(overrideColorBrightness(Strip::Color(127, 127, 127)), 50);
        break;
    case SceneMode::TheaterChaseRed:
        theaterChase(overrideColorBrightness(Strip::Color(127, 0, 0)), 50);
        break;
    case SceneMode::TheaterChaseBlue:
        theaterChase(overrideColorBrightness(Strip::Color(0, 0, 127)), 50);
        break;
    case SceneMode::Rainbow:
        rainbow(10);
        break;
    case SceneMode::TheaterChaseRainbow:
        theaterChaseRainbow(50);
        break;
    case SceneMode::None:
        break;
    }
}


// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::incrementBrightness(int8_t increment)
{
    const int8_t max_step = 20;
    auto cap = [](int8_t &value, int8_t min, int8_t max) {
        value = (value > max) ? max : value;
        value = (value < min) ? min : value;
        return value;
    };

    cap(increment, -max_step, max_step);

    int8_t new_brightness = brightness;
    new_brightness += increment;
    cap(new_brightness, 5, 100);

    brightness = static_cast<uint8_t>(new_brightness);
    Serial.print("PixelRing::incrementBrightness: ");
    Serial.println(brightness);
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::maxBrightness() { brightness = 100; }

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP>
uint8_t PixelRing<LC, LP>::overrideColorChannelBrightness(uint8_t color_value)
{
    uint16_t color =
    ((uint16_t)brightness_override * (uint16_t)brightness * (uint16_t)color_value) / (uint16_t)100;
    color = color > 255 ? 255 : color;
    return static_cast<uint8_t>(color);
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP>
uint32_t PixelRing<LC, LP>::overrideColorBrightness(uint32_t color)
{
    uint8_t r = static_cast<uint8_t>((color & 0x00ff0000) >> 16);
    uint8_t g = static_cast<uint8_t>((color & 0x0000ff00) >> 8);
    uint8_t b = static_cast<uint8_t>((color & 0x000000ff));

    return Strip::Color(overrideColorChannelBrightness(r), overrideColorChannelBrightness(g),
                        overrideColorChannelBrightness(b));
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::colorWipe(uint32_t color)
{
    for(uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, color);
        strip.show();
    }
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP>
void PixelRing<LC, LP>::theaterChase(uint32_t color, uint16_t wait_ms)
{
    if(time_elapsed < wait_ms)
        return;
    time_elapsed = 0;

    static uint16_t a = 0, a_max = 10; // outer loop
    static uint16_t b = 0, b_max = 3;  // inner loop

    {
        strip.clear(); //   Set all pixels in RAM to 0 (off)
        // 'c' counts up from 'b' to end of strip in steps of 3...
        for(uint16_t c = b; c < strip.numPixels(); c += 3)
        {
            strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
        }
        strip.show(); // Update strip with new contents
    }

    b++;
    if(b >= b_max)
    {
        a++;
        b = 0;
    }

    if(a >= a_max)
        a = 0;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::rainbow(uint16_t wait_ms)
{
    if(time_elapsed < wait_ms)
        return;
    time_elapsed = 0;

    // Hue of first pixel runs 3 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
    // means we'll make 3*65536/256 = 768 passes through this outer process:
    static uint32_t firstPixelHue = 0, firstPixelHue_max = 3 * 65536;

    {
        for(uint16_t i = 0; i < strip.numPixels(); i++)
        {
            // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            uint16_t pixelHue = static_cast<uint16_t>(firstPixelHue + (i * 65536L / strip.numPixels()));
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            uint32_t color = overrideColorBrightness(Strip::gamma32(Strip::ColorHSV(pixelHue)));
            strip.setPixelColor(i, color);
        }
        strip.show(); // Update strip with new contents
        // delay(wait);  // Pause for a moment
    }

    firstPixelHue += 256;
    if(firstPixelHue > firstPixelHue_max)
        firstPixelHue = 0;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::theaterChaseRainbow(uint16_t wait_ms)
{
    if(time_elapsed < wait_ms)
        return;
    time_elapsed = 0;

    static uint16_t a = 0, a_max = 30; // outer loop
    static uint16_t b = 0, b_max = 3;  // inner loop

    {
        static uint16_t firstPixelHue = 0; // First pixel starts at red (hue 0)
        strip.clear();                     //   Set all pixels in RAM to 0 (off)
        // 'c' counts up from 'b' to end of strip in increments of 3...
        for(uint16_t c = b; c < strip.numPixels(); c += 3)
        {
            // hue of pixel 'c' is offset by an amount to make one full
            // revolution of the color wheel (range 65536) along the length
            // of the strip (strip.numPixels() steps):
            uint16_t hue = static_cast<uint16_t>(firstPixelHue + c * 65536 / strip.numPixels());
            uint32_t color = overrideColorBrightness(Strip::gamma32(Strip::ColorHSV(hue))); // hue -> RGB
            strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
        }
        strip.show();                // Update strip with new contents
        firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }

    b++;
    if(b >= b_max)
    {
        a++;
        b = 0;
    }

    if(a >= a_max)
        a = 0;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> bool PixelRing<LC, LP>::toggleOnOff()
{
    if(brightness_override == 1)
    {
        off();
    }
    else
    {
        on();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::off()
{
    brightness_override = 0;
    Serial.println("PixelRing::off: turning off");
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::on()
{
    brightness_override = 1;
    Serial.println("PixelRing::on: turning on");
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::incrementWidth(int8_t pixels)
{
    arc_view.incrementArc(pixels);
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::fullWidth() { arc_view.fullWidth(); }

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::shift(int8_t pixels)
{
    arc_view.rotate(pixels);
}

// -------------------------------------------------------------------------------------------------
template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::nextScene()
{
    auto next = [&]() {
        return static_cast<PixelRing<LC, LP>::SceneMode>(static_cast<uint8_t>(last_scene_mode) + 1);
    };

    last_scene_mode = next();
    if(last_scene_mode == SceneMode::Off)
        last_scene_mode = next();

    if(last_scene_mode == SceneMode::None || last_scene_mode == SceneMode::Off)
        last_scene_mode = SceneMode::White;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP>
PixelRing<LC, LP>::ArcBasedView::ArcBasedView(Adafruit_NeoPixel &strip)
: strip(strip), begin(0), end(static_cast<uint8_t>(strip.numPixels() - 1)), pixel_iterator(0), toggle(0)
{
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::ArcBasedView::process(uint32_t new_color)
{
    this->color = new_color;
    process();
}

// -------------------------------------------------------------------------------------------------


template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::ArcBasedView::process()
{
    uint32_t *color_ptr = &color;
    uint32_t black = Strip::Color(0, 0, 0);

    pixel_iterator = begin;
    do
    {
        strip.setPixelColor(pixel_iterator, *color_ptr);
        strip.show();
        if(pixel_iterator == end)
            color_ptr = &black;
    } while(++pixel_iterator != begin);
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::ArcBasedView::rotate(int8_t pixels)
{
    begin += pixels;
    end += pixels;
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::ArcBasedView::incrementArc(int8_t pixels)
{
    Serial.print("PixelRing::ArcBasedView::incrementArc: ");
    Serial.println(pixels);

    while(pixels < 0)
    {
        incrementArcByOne(false);
        ++pixels;
    }

    while(pixels > 0)
    {
        incrementArcByOne(true);
        --pixels;
    }
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP>
void PixelRing<LC, LP>::ArcBasedView::incrementArcByOne(bool do_increment)
{
    int8_t increment = do_increment ? 1 : -1;

    // disallow underflow
    if(!do_increment && begin == end)
        return;

    uint8_t previous_begin = begin, previous_end = end;

    // do the increment
    if(toggle++ == 0)
    {
        begin -= increment;
    }
    else
    {
        end += increment;
    }

    // revert on overflow
    if(do_increment && begin == end)
    {
        end = previous_end;
        begin = previous_begin;
    }
}

// -------------------------------------------------------------------------------------------------

template <uint16_t LC, uint8_t LP> void PixelRing<LC, LP>::ArcBasedView::fullWidth()
{
    begin = 0;
    end = 0;
    --end;
}
