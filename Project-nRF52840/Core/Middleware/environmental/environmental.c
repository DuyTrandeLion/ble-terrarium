#include <math.h>

#include "peripherals.h"
#include "environmental.h"

static uint16_t environmental_spi_time;

static float m_temperature;
static float m_pressure;
static float m_gas_resistance;
static float m_altitude;

static struct bme680_dev m_env_dev;
static struct bme680_field_data m_env_data;

static void environmental_comm_polling_handle(void);
static void environmental_timer_event_handler(void);

static int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    uint32_t ret;
    uint8_t receive_data[128] = {0};

    receive_data[0] = reg_addr;

    ret = env_peripherals_spi_rx(receive_data, len + 1);
    memcpy(reg_data, &receive_data[2], len);

    return ret;
}


static int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    uint8_t transmit_data[128] = {0};

    transmit_data[0] = reg_addr;
    memcpy(&transmit_data[1], reg_data, len);

    return env_peripherals_spi_tx(transmit_data, len + 1);
}


static void user_delay_ms(uint32_t delay_time_ms)
{
    peripherals_delay_ms(delay_time_ms);
}


static void environmental_comm_polling_handle(void)
{
    if (ENVIRONMENTAL_TWI_PROCESS_DATA_PERIOD <= environmental_spi_time)
    {
        environmental_read_sensor_data();
        environmental_spi_time = 0;
    }
}


static void environmental_timer_event_handler(void)
{
    if (0xFFFF != environmental_spi_time)
    {
        environmental_spi_time++;
    }
}


void environmental_init(void)
{
    uint8_t set_required_settings;

    m_env_dev.dev_id = 0;
    m_env_dev.intf = BME680_SPI_INTF;
    m_env_dev.read = user_spi_read;
    m_env_dev.write = user_spi_write;
    m_env_dev.delay_ms = user_delay_ms;
    /* amb_temp can be set to 25 prior to configuring the gas sensor 
     * or by performing a few temperature readings without operating the gas sensor.
     */
    m_env_dev.amb_temp = 25;

    APP_ERROR_CHECK(bme680_init(&m_env_dev));

    /* Set the temperature, pressure and humidity settings */
    m_env_dev.tph_sett.os_hum = BME680_OS_4X;
    m_env_dev.tph_sett.os_temp = BME680_OS_4X;
    m_env_dev.tph_sett.os_pres = BME680_OS_4X;
    m_env_dev.tph_sett.filter = BME680_FILTER_SIZE_3;

    /* Set the remaining gas sensor settings and link the heating profile */
    m_env_dev.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
    /* Create a ramp heat waveform in 3 steps */
    m_env_dev.gas_sett.heatr_temp = 320; /* degree Celsius */
    m_env_dev.gas_sett.heatr_dur = 120; /* milliseconds */

    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    m_env_dev.power_mode = BME680_FORCED_MODE; 

    /* Set the required sensor settings needed */
    set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL | BME680_GAS_SENSOR_SEL;

    /* Set the desired sensor configuration */
    APP_ERROR_CHECK(bme680_set_sensor_settings(set_required_settings, &m_env_dev));

    /* Set the power mode */
    APP_ERROR_CHECK(bme680_set_sensor_mode(&m_env_dev));

    peripherals_assign_comm_handle(ENVIRONMENTAL_COMM, environmental_comm_polling_handle);
    peripherals_assign_comm_handle(TIMER_ENVIRONMENTAL, environmental_timer_event_handler);

    m_env_dev.delay_ms(10);
}


void environmental_read_sensor_data(void)
{
    APP_ERROR_CHECK(bme680_get_sensor_data(&m_env_data, &m_env_dev));
    
    /* Trigger the next measurement if you would like to read data out continuously */
    if (m_env_dev.power_mode == BME680_FORCED_MODE)
    {
        APP_ERROR_CHECK(bme680_set_sensor_mode(&m_env_dev));
    }
}

void environmental_get_data(float *temperature, float *humidity, float *pressure, float *gas_resistance, float *altitude)
{
    float res;

    *temperature = ((float)m_env_data.temperature) / 100.0;
    *humidity = ((float)m_env_data.humidity) / 100.0;
    *gas_resistance = ((float)m_env_data.gas_resistance) / 100.0;
    *pressure = ((float)m_env_data.pressure) / 100.0;

    res = *pressure / 1013.96;
    res = pow(res, 0.19022);
    res = 1.0 - res;
    *altitude = res * ((*temperature + 273.15) / 0.0065);
}
