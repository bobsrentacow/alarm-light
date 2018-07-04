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

ws2811_t ledstring = {
  .freq = 800000,
  .dmanum = 10,
  .channel = {
    [0] = {
      .gpionum = 18,
      .count = LED_COUNT,
      .invert = 0,
      .brightness = 255,
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

static bool running = true;
static void
ctrl_c_handler(
    int signum
)
{
  (void)(signum);
  running = false;
}

static void
setup_handlers(
    void
)
{
  struct sigaction sa = {
    .sa_handler = ctrl_c_handler,
  };

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

int
main(
    int argc,
    char *argv[]
)
{
  // catch signals
  setup_handlers();

  int ii;

  // record startTime
  struct timespec startTime, now;
  clock_gettime(CLOCK_REALTIME, &startTime);

  ws2811_return_t ret;
  if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS) {
    fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
    return ret;
  }

  // seed rand()
  srand(time(0));

  while (running) {
    // elapsed time
    clock_gettime(CLOCK_REALTIME, &now);
    double d_secSinceStart = (now.tv_sec - startTime.tv_sec) + 1e-9 * (now.tv_nsec - startTime.tv_nsec);

    // brightness
    double d_maxBright = 0 + 0.033 * d_secSinceStart;
    d_maxBright = (d_maxBright < 0) ? 0 : (d_maxBright > LOG2_511p5) ? LOG2_511p5 : d_maxBright;
    int minBright;
    int maxBright;
    if (d_maxBright >= LOG2_255p5) {
      maxBright = 255;
      minBright = (int)pow(2.0, LOG2_511p5 - d_maxBright);
    } else {
      maxBright = (int)pow(2.0, d_maxBright);
      minBright = maxBright;
    }

    // color temp
    color_temp_t color;
    color.kelvin = 1000 +  10.0 * d_secSinceStart;
    interp_color_temp(&color);
    //printf("r%0.3f g%0.3f b%0.3f w%0.3f\n", color.norm_red, color.norm_green, color.norm_blue, 0.0);

    // synthesize colors & brightness
    uint32_t red, green, blue, white;
    for (ii=0; ii<LED_COUNT; ii++) {
      white = (uint32_t)(255.0 * 0                * 0.5 * (1.0 + rand() / (double)RAND_MAX)) << 24;
      red   = (uint32_t)(255.0 * color.norm_red   * 0.5 * (1.0 + rand() / (double)RAND_MAX)) << 16;
      green = (uint32_t)(255.0 * color.norm_green * 0.5 * (1.0 + rand() / (double)RAND_MAX)) <<  8;
      blue  = (uint32_t)(255.0 * color.norm_blue  * 0.5 * (1.0 + rand() / (double)RAND_MAX)) <<  0;
      ledstring.channel[0].leds[ii] = white | red | green | blue;
    }
    ledstring.channel[0].brightness = (uint8_t) (minBright + ((maxBright - minBright) * (rand() / (double)RAND_MAX)));

    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS) {
      fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
      break;
    }

    usleep(1000000 / FPS);
  }

  // turn off LEDs & get out
  for (ii=0; ii<LED_COUNT; ii++)
    ledstring.channel[0].leds[ii] = 0;
  ws2811_render(&ledstring);
  ws2811_fini(&ledstring);

  return ret;
}

