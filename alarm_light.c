#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "ws2811.h"
#include "color_temp.h"

#define WIDTH       (120)
#define HEIGHT      (1)
#define LED_COUNT   (WIDTH * HEIGHT)
#define FPS         (16)

#define LOG2_255p5  (7.997179481)
#define LOG2_511p5  (8.998590430)

static ws2811_t ledstring = {
  .freq = 800000,
  .dmanum = 10,
  .channel = {
    [0] = {
      .gpionum = 18,
      .count = LED_COUNT,
      .invert = 0,
      .brightness = 0,
      .strip_type = SK6812W_STRIP,
    },
    [1] = {
      .gpionum = 0,
      .count = 0,
      .invert = 0,
      .brightness = 0,
    },
  },
};
static bool running = false;

int
alarm_light_set_mono_rgbw(
  double red,
  double green,
  double blue,
  double white,
  double bright
)
{
  // synthesize colors & brightness
  uint32_t c_white = (uint32_t)(255.0 * white) << 24;
  uint32_t c_red   = (uint32_t)(255.0 * red  ) << 16;
  uint32_t c_green = (uint32_t)(255.0 * green) <<  8;
  uint32_t c_blue  = (uint32_t)(255.0 * blue ) <<  0;
  uint32_t composite = c_white | c_red | c_green | c_blue;

  int ii;
  for (ii=0; ii<LED_COUNT; ii++)
    ledstring.channel[0].leds[ii] = composite;
  ledstring.channel[0].brightness = (int)(255 * bright);

  if (WS2811_SUCCESS != ws2811_render(&ledstring))
    return -1;

  return 0;
}

int
alarm_light_set_mono_kelvin(
  double kelvin,
  double bright
)
{
  // color temp
  color_temp_t color;
  color.kelvin = kelvin;
  interp_color_temp(&color);

  return alarm_light_set_mono_rgbw(color.norm_red, color.norm_green, color.norm_blue, 0.0, bright);
}

int
alarm_light_set_rand_kelvin(
  double kelvin,
  double bright
)
{
  // color temp
  color_temp_t color;
  color.kelvin = kelvin;
  interp_color_temp(&color);

  // synthesize colors & brightness
  uint32_t red, green, blue, white;

  int ii;
  for (ii=0; ii<LED_COUNT; ii++) {
    white = (uint32_t)(255.0 * 0                * 0.5 * (1.0 + rand() / (double)RAND_MAX)) << 24;
    red   = (uint32_t)(255.0 * color.norm_red   * 0.5 * (1.0 + rand() / (double)RAND_MAX)) << 16;
    green = (uint32_t)(255.0 * color.norm_green * 0.5 * (1.0 + rand() / (double)RAND_MAX)) <<  8;
    blue  = (uint32_t)(255.0 * color.norm_blue  * 0.5 * (1.0 + rand() / (double)RAND_MAX)) <<  0;
    ledstring.channel[0].leds[ii] = white | red | green | blue;
  }
  ledstring.channel[0].brightness = (uint8_t)(255 * bright);

  if (WS2811_SUCCESS != ws2811_render(&ledstring))
    return -1;

  return 0;
}

int
alarm_light_off(
    void
)
{
  if (running) {
    running = false;
  } else {
    int ii;
    for (ii=0; ii<LED_COUNT; ii++)
      ledstring.channel[0].leds[ii] = 0;
    ledstring.channel[0].brightness = 0;
    ws2811_render(&ledstring);
    ws2811_fini(&ledstring);
  }

  return 0;
}

int
alarm_light_wakeup(
    double seconds,
    double bright_start,  //    0
    double bright_perSec, //    0.004
    double kelvin_start,  // 1000
    double kelvin_perSec  //   50
)
{

  // record startTime
  struct timespec startTime, now;
  clock_gettime(CLOCK_REALTIME, &startTime);

  // seed rand()
  srand(time(0));

  running = true;
  while (running) {
    // elapsed time
    clock_gettime(CLOCK_REALTIME, &now);
    double d_secSinceStart = (now.tv_sec - startTime.tv_sec) + 1e-9 * (now.tv_nsec - startTime.tv_nsec);

    // expire after a while
    if (d_secSinceStart > seconds)
      break;

    // brightness
    double d_maxBright = bright_start + bright_perSec * d_secSinceStart;
    d_maxBright = (d_maxBright < 0) ? 0 : (d_maxBright > 2.0) ? 2.0 : d_maxBright;
    double d_minBright;
    if (d_maxBright >= 1.0) {
      d_maxBright = 1.0;
      d_minBright = pow(2, 8 * (1 - d_maxBright));
    } else {
      d_maxBright = pow(2, 8 * (d_maxBright - 1));
      d_minBright = d_maxBright;
    }
    double bright = (d_minBright + ((d_maxBright - d_minBright) * (rand() / (double)RAND_MAX)));

    // color temp
    double kelvin = kelvin_start + kelvin_perSec * d_secSinceStart;

    alarm_light_set_rand_kelvin(kelvin, bright);

    usleep(1000000 / FPS);
  }

  // turn off LEDs & get out
  int ii;
  for (ii=0; ii<LED_COUNT; ii++)
    ledstring.channel[0].leds[ii] = 0;
  ledstring.channel[0].brightness = 0;
  ws2811_render(&ledstring);
  ws2811_fini(&ledstring);

  return 0;
}

int
alarm_light_init(
    void
)
{
  ws2811_return_t ret = 0;
  if (WS2811_SUCCESS != (ret = ws2811_init(&ledstring))) {
    fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
    return ret;
  }

  return ret;
}
