#include "peripherals.h"

#include "environmental.h"

static uint16_t environmental_spi_time;

static float m_temperature;
static float m_pressure;
static float m_altitude;

//static ICPPRess_Def_t m_environmental_def;

static void environmental_comm_polling_handle(void);
static void environmental_timer_event_handler(void);

static uint8_t environmental_comm_handle(uint8_t bme_event, uint16_t device_address, uint8_t *data_buffer, uint16_t data_buffer_size, void *context)
{
    uint8_t restart_i2c;
    switch (icp_event)
    {
        case EVENT_TRANSMIT:
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
        
        case EVENT_RECEIVE:
        {
            return peripherals_twi_rx(device_address, data_buffer, data_buffer_size);
            break;
        }

        case EVNT_TRANSMIT_RECEIVE:
        {
            break;
        }

        default: break;
    }
}


static uint8_t environmental_delay_handle(uint32_t delay_time_ms)
{
    peripherals_delay_ms(delay_time_ms);
}


static void environmental_comm_polling_handle(void)
{
    if (BAROMETER_TWI_PROCESS_DATA_PERIOD <= environmental_spi_time)
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
    //m_environmental_def.commHandle = environmental_comm_handle;
    //m_environmental_def.delayHandle = environmental_delay_handle;

    //peripherals_assign_comm_handle(BAROMETER_COMM, environmental_comm_polling_handle);
    //peripherals_assign_comm_handle(TIMER_BAROMETER, environmental_timer_event_handler);

    //APP_ERROR_CHECK(ICPPress_Init(&m_environmental_def));
    //ICPPress_SetMeasurementMode(&m_environmental_def, ICP_CMD_MEASURE_N_P_FIRST);

    //m_environmental_def.delayHandle(10);
}


void environmental_read_sensor_data(void)
{

}
