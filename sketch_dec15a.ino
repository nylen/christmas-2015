#include <FastLED.h>

#define NUM_LEDS 480

#define X_SIZE 24
#define Y_SIZE 20

CRGB leds[NUM_LEDS];

/**
 * Pattern options
 */

enum SnakeBackground {
  BLACK,
  RAINBOW,
  NOISE
};

/**
 * Utility functions
 */

struct sprite {
  byte size = 0;
  uint64_t data = 0;
  sprite(byte w, byte h, uint64_t d) {
    size = h << 4 | w;
    data = d;
  }
  byte width() {
    return size & 0xf;
  }
  byte height() {
    return size >> 4;
  }
  bool hasPixel(byte x, byte y) {
    if (x < 0 || y < 0 || x >= width() || y >= height()) {
      return false;
    }
    byte dataBit = x + y * width();
    return (data >> dataBit) & 1;
  }
};

void drawSprite(byte x, byte y, sprite * s, CRGB color) {
  for (int sx = 0; sx < s->width(); sx++) {
    for (int sy = 0; sy < s->height(); sy++) {
      if (s->hasPixel(sx, sy)) {
        leds[px(x + sx, y + sy)] = color;
      }
    }
  }
}

int px(byte x, byte y) {
  // Screen is inverted
  x = X_SIZE - x - 1;
  y = Y_SIZE - y - 1;
  // Now act as though LED 0 is top left (X=0)
  // LED y_size is where X=1 starts
  // Calculate the position along the strand due to the X offset
  int pos_X = x * Y_SIZE;
  // LED 0 is Y=0
  // LED y_size-1 is Y=y_size-1
  // LED y_size   is Y=y_size-1
  // LED 2*y_size-1 is Y=0
  // LED 2*y_size   is Y=0
  // If on an odd X coordinate, then Y is inverted
  int pos_Y = (x % 2 ? Y_SIZE - y - 1 : y);
  return pos_X + pos_Y;
}

void rect(byte x, byte y, byte w, byte h, CRGB color) {
  for (byte i = x; i < x + w; i++) {
    for (byte j = y; j < y + h; j++) {
      leds[px(i, j)] = color;
    }
  }
}

int setAll(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

int frame(int delayMs) {
  FastLED.show();
  delay(delayMs);
}

/**
 * Sprites
 */

sprite s_Ho(8, 7, 0x006909999f990969); /*
  1  0  0  1  0  1  1  0
  1  0  0  1  0  0  0  0
  1  0  0  1  1  0  0  1
  1  1  1  1  1  0  0  1
  1  0  0  1  1  0  0  1
  1  0  0  1  0  0  0  0
  1  0  0  1  0  1  1  0
*/

sprite s_Tree1(8, 8, 0x240018007e240018); /*
  0  0  0  1  1  0  0  0
  0  0  0  0  0  0  0  0
  0  0  1  0  0  1  0  0
  0  1  1  1  1  1  1  0
  0  0  0  0  0  0  0  0
  0  0  0  1  1  0  0  0
  0  0  0  0  0  0  0  0
  0  0  1  0  0  1  0  0
*/
sprite s_Tree2(8, 6, 0x0000181800ff4200); /*
  0  0  0  0  0  0  0  0
  0  1  0  0  0  0  1  0
  1  1  1  1  1  1  1  1
  0  0  0  0  0  0  0  0
  0  0  0  1  1  0  0  0
  0  0  0  1  1  0  0  0
*/

/**
 * Pattern: Snakes
 * Snakes that move around the screen and shift hue.  Several different backgrounds.
 */

void Snakes(SnakeBackground background) {
static byte hue = 0;
static int offset = 0;
static int num_snakes = 7;
static int snake_len = 8;
  hue++;
  offset = (offset + 1) % NUM_LEDS;
  switch (background) {
    case SnakeBackground::BLACK:
      setAll(CRGB::Black);
      break;
    case SnakeBackground::RAINBOW:
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(sin8(i * 2 + hue), 192, 64);
      }
      break;
    case SnakeBackground::NOISE:
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(random8(), random8() / 2 + 96, random8() / 4);
      }
      break;
  }
  for (int j = 0; j < num_snakes; j++) {
    for (int i = offset; i < offset + snake_len; i++) {
      leds[(i + NUM_LEDS * j / num_snakes) % NUM_LEDS] = CHSV(hue, 255, 255);
    }
  }
  frame(30);
}

/**
 * Pattern: TestPattern
 * Red, Green, Blue, White
 */

void TestPattern(int delayMs, byte level) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(level, 0, 0);
  }
  frame(delayMs);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, level, 0);
  }
  frame(delayMs);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, level);
  }
  frame(delayMs);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 0, level);
  }
  frame(delayMs);
}

/**
 * Pattern: Noise
 * Needs hue adjustment for more red/yellow + maybe some more saturation/value adjustments.
 */

void Noise(int delayMs) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random8(), random8() / 4 + 96, random8() / 4);
  }
  frame(delayMs);
}

/**
 * Pattern: Bounce
 * Bouncing square
 */
void Bounce(int delayMs, byte value) {
static byte x = 0;
static byte y = 0;
static byte dx = 1;
static byte dy = 2;
static byte w = 3;
static byte h = 4;
  rect(x, y, w, h, CRGB::Black);
  x += dx;
  if (x >= X_SIZE - w || x == 0) {
    dx = -dx;
  }
  y += dy;
  if (y == Y_SIZE - h || y == 0) {
    dy = -dy;
  }
  rect(x, y, w, h, CRGB(value, value, value / 4));
  frame(delayMs);
}

/**
 * Pattern: Snow
 */
void Snow(int delayMs, int chance, int value) {
#define snowflakes 40
static byte xs[snowflakes];
static byte ys[snowflakes];
static byte vs[snowflakes];
static bool init = true;
  if (init) {
    for (int i = 0; i < snowflakes; i++) {
      ys[i] = Y_SIZE;
    }
    init = false;
  }
  for (int i = 0; i < snowflakes; i++) {
    if (ys[i] < Y_SIZE) {
      leds[px(xs[i], ys[i])] = CRGB::Black;
    }
    if (ys[i] < Y_SIZE) {
      ys[i]++;
    } else if (random16() < chance) {
      ys[i] = 0;
      xs[i] = random8(X_SIZE);
      vs[i] = random8(10, value + 1);
    }
    if (ys[i] < Y_SIZE) {
      byte v = vs[i];
      leds[px(xs[i], ys[i])] = CRGB(v, v * 7 / 10, v / 5);
    }
  }
  frame(delayMs);
}

/**
 * Pattern: SnowHoHo
 */
bool SnowHoHo_init = true;
bool SnowHoHo(int delayMs, byte framesPerStep, int chance, int value) {
#define SnowHoHo_snowflakes 40
static byte xs[SnowHoHo_snowflakes];
static byte ys[SnowHoHo_snowflakes];
static byte vs[SnowHoHo_snowflakes];
static byte cyclePos = 0;
static byte frameNum = 0;
if (SnowHoHo_init) {
  for (int i = 0; i < SnowHoHo_snowflakes; i++) {
    ys[i] = Y_SIZE;
    cyclePos = 14;
  }
  SnowHoHo_init = false;
}

  // Erase snowflakes from last iteration
  for (int i = 0; i < SnowHoHo_snowflakes; i++) {
    if (ys[i] < Y_SIZE) {
      leds[px(xs[i], ys[i])] = CRGB::Black;
    }
  }

  // Draw sprites, if any
  // 8 steps in a cycle:
  // 0: "HO" "--" "--"
  // 1: "HO" "HO" "--"
  // 2: "HO" "HO" "HO"
  // 3: "HO" "HO" "HO"
  // 4: "--" "HO" "HO"
  // 5: "--" "--" "HO"
  // 6: "--" "--" "--"
  // 7: "--" "--" "--"
  byte spriteStep = cyclePos % 8;
  if (cyclePos < 8) {
    // "HO HO HO" cycle
    CRGB color = CRGB(value, 0, 0);
    byte y = (Y_SIZE - s_Ho.height()) / 2;
    if (spriteStep <= 4) {
      drawSprite(0, y, &s_Ho, (spriteStep < 4 ? color : CRGB::Black));
    }
    if (spriteStep <= 5 && spriteStep > 0) {
      drawSprite(8, y, &s_Ho, (spriteStep < 5 ? color : CRGB::Black));
    }
    if (spriteStep <= 6 && spriteStep > 1) {
      drawSprite(16, y, &s_Ho, (spriteStep < 6 ? color : CRGB::Black));
    }
  } else {
    // Christmas trees cycle
    CRGB color = CRGB(value / 8, value / 4, 0);
    byte y = (Y_SIZE - (s_Tree1.height() + s_Tree2.height())) / 2;
    if (spriteStep <= 4) {
      drawSprite(0, y + 0, &s_Tree1, (spriteStep < 4 ? color : CRGB::Black));
      drawSprite(0, y + 8, &s_Tree2, (spriteStep < 4 ? color : CRGB::Black));
    }
    if (spriteStep <= 5 && spriteStep > 0) {
      drawSprite(8, y + 0, &s_Tree1, (spriteStep < 5 ? color : CRGB::Black));
      drawSprite(8, y + 8, &s_Tree2, (spriteStep < 5 ? color : CRGB::Black));
    }
    if (spriteStep <= 6 && spriteStep > 1) {
      drawSprite(16, y + 0, &s_Tree1, (spriteStep < 6 ? color : CRGB::Black));
      drawSprite(16, y + 8, &s_Tree2, (spriteStep < 6 ? color : CRGB::Black));
    }
  }
  
  // Advance snowflakes or randomly create new ones
  for (int i = 0; i < SnowHoHo_snowflakes; i++) {
    if (ys[i] < Y_SIZE) {
      ys[i]++;
    } else if (random16() < chance) {
      ys[i] = 0;
      xs[i] = random8(X_SIZE);
      vs[i] = random8(10, value + 1);
    }
  }

  // Draw snowflakes
  for (int i = 0; i < SnowHoHo_snowflakes; i++) {
    if (ys[i] < Y_SIZE) {
      byte v = vs[i];
      CRGB snowColor = CRGB(v, v * 7 / 10, v / 5);
      int pixel = px(xs[i], ys[i]);
      CRGB pixelColor = leds[pixel];
      if (pixelColor == (CRGB)CRGB::Black) {
        leds[pixel] = snowColor;
      } else {
        leds[pixel] = blend(
          snowColor,
          pixelColor,
          (cyclePos < 8 ? 128 : 64)
        );
      }
    }
  }

  // Draw stuff
  frame(delayMs);

  // Count frames and advance sprite cycle
  bool didFullCycle = false;
  frameNum = (frameNum + 1) % framesPerStep;
  if (frameNum == 0) {
    cyclePos = (cyclePos + 1) % 16;
    didFullCycle = (cyclePos == 0);
  }

  // Let the caller know if we did a full cycle
  return didFullCycle;
}

/**
 * Pattern: Swatches
 * X: Green (8 different levels)
 * Y: Red (10 different levels)
 * Grid from top left: Blue (6 different levels)
 */
void Swatches(byte multiplier) {
#define rCount 10
#define gCount 8
#define bCount 6
static byte rs[rCount] = { 0, 2, 4, 8, 16, 24, 32, 40, 48, 64 };
static byte gs[gCount] = { 0, 2, 3, 4, 8, 16, 24, 32 };
static byte bs[bCount] = { 0, 2, 3, 4, 8, 12 };
  for (byte b = 0; b < 6; b++) {
    for (byte g = 0; g < 8; g++) {
      for (byte r = 0; r < 10; r++) {
        byte x = (b / 2) * 8 + g;
        byte y = (b % 2) * 10 + r;
        leds[px(x, y)] = CRGB(
          min(rs[r] * multiplier, 255),
          min(gs[g] * multiplier, 255),
          min(bs[b] * multiplier, 255)
        );
      }
    }
  }
  frame(1000);
}

/**
 * Pattern: CycleColors
 */
void CycleColors(int delayMs) {
static CRGB yellow(40, 16, 0);
static CRGB red(24, 0, 0);
static CRGB white(32, 16, 3); // Daytime: 255, 96, 20
static CRGB green(8, 16, 0);
#define num_colors 4
static CRGB colors[num_colors] = { red, yellow, white, green };
static byte offset = 0;
  byte i = offset;
  byte j = random8(3, num_colors);
  CRGB tmp = colors[i];
  colors[i] = colors[j];
  colors[j] = tmp;
  rect( 0, 0, 8, 20, colors[0]);
  rect( 8, 0, 8, 20, colors[1]);
  rect(16, 0, 8, 20, colors[2]);
  frame(delayMs);
  offset = (offset + 1) % 3;
}

/**
 * Pattern: RainbowGradient
 */
void RainbowGradient(int value) {
static int hue = 0;
  for (int x = 0; x < X_SIZE; x++) {
    for (int y = 0; y < Y_SIZE; y++) {
      leds[px(x, y)] = CHSV((hue + x*4 + y*2) & 0xff, 255, value);
    }
  }
  hue++;
  frame(10);
}

void setup() {
  FastLED.addLeds<WHATEVER_THESE_ARE, 2>(leds, NUM_LEDS);
}

bool isRainbow = false;
int cycleCount = 0;
#define SNOW_CYCLES 3
#define RAINBOW_FRAMES 500
#define RAINBOW_FADE_FRAMES 50
#define RAINBOW_VALUE 100

void loop() {
  if (isRainbow) {
    cycleCount++;
    if (cycleCount < RAINBOW_FADE_FRAMES) {
      RainbowGradient(cycleCount * RAINBOW_VALUE / RAINBOW_FADE_FRAMES);
    } else if (cycleCount >= RAINBOW_FRAMES - RAINBOW_FADE_FRAMES) {
      RainbowGradient((RAINBOW_FRAMES - cycleCount) * RAINBOW_VALUE / RAINBOW_FADE_FRAMES);
      if (cycleCount == RAINBOW_FRAMES) {
        cycleCount = 0;
        isRainbow = false;
        rect(0, 0, X_SIZE, Y_SIZE, CRGB::Black);
        SnowHoHo_init = true;
      }
    } else {
      RainbowGradient(RAINBOW_VALUE);
    }
  } else {
    if (SnowHoHo(90, 8, 3000, 100)) {
      cycleCount++;
      // 1 extra because the first "cycle" is just a few frames of snow
      if (cycleCount == SNOW_CYCLES + 1) {
        isRainbow = true;
      }
    }
  }
  
  //Snow(100, 2000, 100);
  //CycleColors(1000);
  //TestPattern(1000, 255);
  //RainbowGradient(200);
}

