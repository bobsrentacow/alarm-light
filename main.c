#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include "ws2811.h"
#include "color_temp.h"

#define WIDTH                   120
#define HEIGHT                  1
#define LED_COUNT               (WIDTH * HEIGHT)
#define FPS                     16

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

uint32_t *color_table;

static int
color_table_init(
		int size,
    double kMin,
    double kMax
)
{
  if (color_table > 0)
		free(color_table);
	color_table = malloc(size * sizeof(color_table[0]));
	if (color_table <= 0) {
		fprintf(stderr, "failed to allocate color table\n");
		return -1;
	}

	int ii;
	double step = (kMax - kMin) / (size - 1);
	color_temp_t ct;
	for (ii=0; ii<size; ii++) {
		ct.kelvin = step * ii + kMin;
		interp_color_temp(&ct);
		color_table[ii] = ((uint32_t)ct.byte_red) << 16;
		color_table[ii] |= ((uint32_t)ct.byte_green) << 8;
		color_table[ii] |= ((uint32_t)ct.byte_blue) << 0;
    //printf("%04d kelvin: %08x\n", (int)nearbyint(ct.kelvin), color_table[ii]);
	}

	return 0;
}

int
main(
    int argc,
    char *argv[]
)
{
  setup_handlers();

  ws2811_return_t ret;
  if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS) {
    fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
    return ret;
  }

  const int table_size = 256;
	if (color_table_init(table_size, 2500, 5500))
		return -1;

 	int ii;
  int shift = 0;
  double dub_idx;
  while (running) {
		for (ii=0; ii<LED_COUNT; ii++) {
      dub_idx = (shift + ii) * table_size / (double)LED_COUNT;
			ledstring.channel[0].leds[ii] = color_table[(int)nearbyint(dub_idx) % table_size];
    }
    ledstring.channel[0].brightness = 0x20;
    shift = (shift + 1) % table_size;

    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS) {
      fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
      break;
    }

    usleep(1000000 / FPS);
  }

  for (ii=0; ii<LED_COUNT; ii++)
    ledstring.channel[0].leds[ii] = 0;
  ws2811_render(&ledstring);
  ws2811_fini(&ledstring);

  return ret;
}

