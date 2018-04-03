#ifndef __COLOR_TEMP_H__
 #define __COLOR_TEMP_H__

typedef struct color_temp {
  double kelvin;
  double intensity;
  double norm_red;
  double norm_green;
  double norm_blue;
  int byte_red;
  int byte_green;
  int byte_blue;
} color_temp_t;

extern int
interp_color_temp(
    color_temp_t *out
);

#endif //__COLOR_TEMP_H__
