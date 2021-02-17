#include "peripherals.h"

#include "barometer.h"

static uint16_t barometer_twi_time;

static float m_temperature;
static float m_pressure;
static float m_altitude;

static ICPPRess_Def_t m_barometer_def;

static void barometer_comm_polling_handle(void);
static void barometer_timer_event_handler(void);

static ICPPress_State_t barometer_comm_handle(ICPPress_Event_t icp_event, uint16_t device_address, uint8_t *data_buffer, uint16_t data_buffer_size, void *context)
{
    uint8_t restart_i2c;
    switch (icp_event)
    {
        case I2C_EVENT_TRANSMIT:
        {
            if (NULL == context)
            {
                return peripherals_twi_tx(device_address, data_buffer, data_buffer_size, false);
            }
            else
            {
                restart_i2c = *((uint8_t *)context);
                return peripherals_twi_tx(device_address, data_buffer, data_buffer_size, restart_i2c);
            }
            break;
        }
        
        case I2C_EVENT_RECEIVE:
        {
            return peripherals_twi_rx(device_address, data_buffer, data_buffer_size);
            break;
        }

        default: break;
    }
}


static uint8_t barometer_delay_handle(uint32_t delay_time_ms)
{
    peripherals_delay_ms(delay_time_ms);
}


static void barometer_comm_polling_handle(void)
{
    if (BAROMETER_TWI_PROCESS_DATA_PERIOD <= barometer_twi_time)
    {
        barometer_read_sensor_data();
        barometer_twi_time = 0;
    }
}


static void barometer_timer_event_handler(void)
{
    if (0xFFFF != barometer_twi_time)
    {
        barometer_twi_time++;
    }
}


void barometer_init(void)
{
    m_barometer_def.commHandle = barometer_comm_handle;
    m_barometer_def.delayHandle = barometer_delay_handle;

    peripherals_assign_comm_handle(BAROMETER_COMM, barometer_comm_polling_handle);
    peripherals_assign_comm_handle(TIMER_BAROMETER, barometer_timer_event_handler);

    APP_ERROR_CHECK(ICPPress_Init(&m_barometer_def));
    ICPPress_SetMeasurementMode(&m_barometer_def, ICP_CMD_MEASURE_N_P_FIRST);

    m_barometer_def.delayHandle(10);
}


void barometer_read_sensor_data(void)
{
    APP_ERROR_CHECK(ICPPress_GetProcessedData(&m_barometer_def, &m_temperature, &m_pressure, &m_altitude));
}


void barometer_get_altitude(float *altitude)
{
    *altitude = m_altitude;
}
