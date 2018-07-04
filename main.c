#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include "alarm_light.h"

static void
ctrl_c_handler(
    int signum
)
{
  (void)(signum);
  alarm_light_off();
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

static void
print_usage(
    void
)
{
  fprintf(stderr, "-o cannot be combined with other options.\n");
  fprintf(stderr, "Cannot combine -k with -rgbw. Specify color with either temperature or components.\n");
  exit(-1);
}

struct options {
  double bright;
  double kelvin;
  double red;
  double green;
  double blue;
  double white;
  bool   off;
};

struct options
parse_args(
    int argc,
    char *argv[]
)
{
  struct options opts = {
    .bright = 1.0,
    .kelvin = -1,
    .red    = -1,
    .green  = -1,
    .blue   = -1,
    .white  = -1,
    .off    = false
  };

  int c;
  static int off_flag = 0;
  while(true) {
    static struct option long_options[] = {
      {"off",    no_argument,       &off_flag, 0},
      {"bright", required_argument, 0,         'B'},
      {"kelvin", required_argument, 0,         'k'},
      {"red",    required_argument, 0,         'r'},
      {"green",  required_argument, 0,         'g'},
      {"blue",   required_argument, 0,         'b'},
      {"white",  required_argument, 0,         'w'},
      {0, 0, 0, 0}
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "B:k:r:g:b:w:o", long_options, &option_index);

    if (-1 == c)
      break;

    switch (c) {
      case 0:
      case 'o':
        if (opts.red   >= 0)
          print_usage();
        if (opts.green >= 0)
          print_usage();
        if (opts.blue  >= 0)
          print_usage();
        if (opts.white >= 0)
          print_usage();
        if (opts.kelvin >= 0)
          print_usage();
        if (opts.bright != 1.0)
          print_usage();
        opts.off = true;
        break;
      case 'B':
        if (opts.off)
          print_usage();
        opts.bright = strtod(optarg, 0);
        break;
      case 'k':
        if (opts.red   >= 0)
          print_usage();
        if (opts.green >= 0)
          print_usage();
        if (opts.blue  >= 0)
          print_usage();
        if (opts.white >= 0)
          print_usage();
        if (opts.off)
          print_usage();
        opts.kelvin = strtod(optarg, 0);
        break;
      case 'r':
        if (opts.kelvin >= 0)
          print_usage();
        if (opts.off)
          print_usage();
        opts.red = strtod(optarg, 0);
        break;
      case 'g':
        if (opts.kelvin >= 0)
          print_usage();
        if (opts.off)
          print_usage();
        opts.green = strtod(optarg, 0);
        break;
      case 'b':
        if (opts.kelvin >= 0)
          print_usage();
        if (opts.off)
          print_usage();
        opts.blue = strtod(optarg, 0);
        break;
      case 'w':
        if (opts.kelvin >= 0)
          print_usage();
        if (opts.off)
          print_usage();
        opts.white = strtod(optarg, 0);
        break;
      case '?':
        break;
      default:
        abort();
    }
  }

  return opts;
}

int
main(
    int argc,
    char *argv[]
)
{
  struct options opts = parse_args(argc, argv);
  //printf(" bright: %f\n", opts.bright);
  //printf(" kelvin: %f\n", opts.kelvin);
  //printf(" red   : %f\n", opts.red   );
  //printf(" green : %f\n", opts.green );
  //printf(" blue  : %f\n", opts.blue  );
  //printf(" white : %f\n", opts.white );
  //printf(" off   : %s\n", opts.off   ? "true" : "false");

  if (opts.off) {
    alarm_light_init();
    alarm_light_off();
    return 0;
  }

  if (opts.kelvin >= 0) {
    alarm_light_init();
    alarm_light_set_mono_kelvin(opts.kelvin, opts.bright);
    return 0;
  }

  if (opts.red >= 0 || opts.green >= 0 || opts.blue >= 0 || opts.white >= 0) {
    if (opts.red < 0)
      opts.red = 0;
    if (opts.red > 1)
      opts.red = 1;
    if (opts.green < 0)
      opts.green = 0;
    if (opts.green > 1)
      opts.green = 1;
    if (opts.blue < 0)
      opts.blue = 0;
    if (opts.blue > 1)
      opts.blue = 1;
    if (opts.white < 0)
      opts.white = 0;
    if (opts.white > 1)
      opts.white = 1;

    alarm_light_init();
    alarm_light_set_mono_rgbw(opts.red, opts.green, opts.blue, opts.white, opts.bright);
    return 0;
  }

  setup_handlers();
  alarm_light_init();
  alarm_light_wakeup(30, 0, 0.004, 1000, 50);

  return 0;
}

