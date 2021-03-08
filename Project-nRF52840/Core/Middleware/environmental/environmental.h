#ifndef _ENVIRONMENTAL_H_
#define _ENVIRONMENTAL_H_

#include <stdint.h>

#include "bme680.h"
#include "bme680_defs.h"
#include "Miscellaneous.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t temperature;
    uint32_t humidity;
    uint32_t pressure;
    uint32_t gas_resistance;
    uint32_t altitude;
} env_data_t;

void environmental_init(void);
void environmental_read_sensor_data(void);
void environmental_get_data(env_data_t *env_data);

#ifdef __cplusplus
}
#endif

#endif /* _ENVIRONMENTAL_H_ */