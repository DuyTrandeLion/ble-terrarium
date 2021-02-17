#ifndef _BAROMETER_H_
#define _BAROMETER_H_

#include <stdint.h>

#include "ICP101xx.h"
#include "Miscellaneous.h"

#ifdef __cplusplus
extern "C" {
#endif

void barometer_init(void);
void barometer_read_sensor_data(void);
void barometer_get_altitude(float *altitude);

#ifdef __cplusplus
}
#endif

#endif /* _BAROMETER_H_ */