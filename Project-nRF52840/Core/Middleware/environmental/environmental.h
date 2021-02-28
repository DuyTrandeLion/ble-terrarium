#ifndef _ENVIRONMENTAL_H_
#define _ENVIRONMENTAL_H_

#include <stdint.h>

#include "bme680.h"
#include "bme680_defs.h"
#include "Miscellaneous.h"

#ifdef __cplusplus
extern "C" {
#endif

void environmental_init(void);
void environmental_read_sensor_data(void);
void environmental_get_data(float *temperature, float *humidity, float *pressure, float *gas_resistance, float *altitude);

#ifdef __cplusplus
}
#endif

#endif /* _ENVIRONMENTAL_H_ */