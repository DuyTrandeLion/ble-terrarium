#include <math.h>

#include "peripherals.h"
#include "uv.h"

static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);


static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void uv_init(void)
{

}

void uv_get_data(uint8_t *uv_index)
{
    float uv_volt_f;

    uvi_read_voltage(&uv_volt_f);

    /* Mapping the UV_Voltage to intensity is straight forward.
       No UV light starts at 1V with a maximum of 15mW/cm2 at around 2.8V. */
    *uv_index = mapfloat(uv_volt_f, 0.90, 2.8, 0.0, 15.0) * 100;
}
