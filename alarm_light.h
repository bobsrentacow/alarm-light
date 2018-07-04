#ifndef __ALARM_LIGHT_H__
#define __ALARM_LIGHT_H__

extern int alarm_light_set_mono_rgbw(double red, double green, double blue, double white, double bright);
extern int alarm_light_set_mono_kelvin(double kelvin, double white, double bright);
extern int alarm_light_set_rand_kelvin(double kelvin, double white, double bright);
extern int alarm_light_off(void);
extern int alarm_light_wakeup(
    double seconds,
    double bright_start,  //    0
    double bright_perSec, //    0.004
    double kelvin_start,  // 1000
    double kelvin_perSec  //   50
);
extern int alarm_light_init(void);

#endif //__ALARM_LIGHT_H__
