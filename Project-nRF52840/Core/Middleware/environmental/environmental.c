#include "peripherals.h"

#include "environmental.h"

static uint16_t environmental_spi_time;

static float m_temperature;
static float m_pressure;
static float m_altitude;

//static ICPPRess_Def_t m_environmental_def;

static void environmental_comm_polling_handle(void);
static void environmental_timer_event_handler(void);

static int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    /*
     * The parameter dev_id can be used as a variable to select which Chip Select pin has
     * to be set low to activate the relevant device on the SPI bus
     */

    /*
     * Data on the bus should be like
     * |----------------+---------------------+-------------|
     * | MOSI           | MISO                | Chip Select |
     * |----------------+---------------------|-------------|
     * | (don't care)   | (don't care)        | HIGH        |
     * | (reg_addr)     | (don't care)        | LOW         |
     * | (don't care)   | (reg_data[0])       | LOW         |
     * | (....)         | (....)              | LOW         |
     * | (don't care)   | (reg_data[len - 1]) | LOW         |
     * | (don't care)   | (don't care)        | HIGH        |
     * |----------------+---------------------|-------------|
     */

    return rslt;
}


int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    /*
     * The parameter dev_id can be used as a variable to select which Chip Select pin has
     * to be set low to activate the relevant device on the SPI bus
     */

    /*
     * Data on the bus should be like
     * |---------------------+--------------+-------------|
     * | MOSI                | MISO         | Chip Select |
     * |---------------------+--------------|-------------|
     * | (don't care)        | (don't care) | HIGH        |
     * | (reg_addr)          | (don't care) | LOW         |
     * | (reg_data[0])       | (don't care) | LOW         |
     * | (....)              | (....)       | LOW         |
     * | (reg_data[len - 1]) | (don't care) | LOW         |
     * | (don't care)        | (don't care) | HIGH        |
     * |---------------------+--------------|-------------|
     */

    return rslt;
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
