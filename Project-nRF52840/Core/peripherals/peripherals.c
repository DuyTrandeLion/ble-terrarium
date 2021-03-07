#include "peripherals.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_timer.h"


static const nrf_drv_twi_t m_baro_twi       = NRF_DRV_TWI_INSTANCE(BARO_TWI_INSTANCE);
static const nrf_drv_twi_t m_eep_twi        = NRF_DRV_TWI_INSTANCE(EEP_TWI_INSTANCE);
static const nrf_drv_spi_t m_env_spi        = NRF_DRV_SPI_INSTANCE(ENV_SPI_INSTANCE);
static const nrf_drv_timer_t m_gen_timer    = NRF_DRV_TIMER_INSTANCE(GENERAL_TIMER_INSTANACE);

static comm_handle_fptr m_barometer_comm_handler;
static comm_handle_fptr m_environmental_comm_handler;
static comm_handle_fptr m_eeprom_comm_handler;
static comm_handle_fptr m_timer_barometer_handler;
static comm_handle_fptr m_timer_environmental_handler;
static comm_handle_fptr m_timer_eeprom_handler;
static comm_handle_fptr m_timer_ble_update_handler;
static comm_handle_fptr m_timer_general_handler;

APP_TIMER_DEF(m_ble_timer_id);                                                  /**< BLE timer. */


static void baro_twi_event_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);
static void eep_twi_event_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);
static void env_spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context);
static void ble_update_timeout_handler(void * p_context);
static void general_timer_event_handler(nrf_timer_event_t event_type, void* p_context);
static void timer_ubx_event_handler(nrf_timer_event_t event_type, void* p_context);

static void gpio_init(void);
static void pwm_init(void);
static void saadc_init(void);
static void twi_init(void);
static void spi_init(void);
static void timer_init(void);


static void gpio_init(void)
{
    nrf_gpio_cfg_output(ENV_CS_PIN);
}


static void pwm_init(void)
{

}


static void saadc_init(void)
{

}


static void twi_init(void)
{
    ret_code_t err_code;

    nrf_drv_twi_config_t baro_twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;

    baro_twi_config.sda = BARO_I2C_SDA_PIN;
    baro_twi_config.scl = BARO_I2C_SCL_PIN;

#if TWI_USE_INTERRUPT
    nrf_drv_twi_init(&m_baro_twi, &baro_twi_config, baro_twi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
#else
    nrf_drv_twi_init(&m_baro_twi, &baro_twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
#endif
    nrf_drv_twi_enable(&m_baro_twi);
}


static void spi_init(void)
{
    ret_code_t err_code;

    nrf_drv_spi_config_t env_spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    env_spi_config.frequency  = NRF_DRV_SPI_FREQ_8M;
    env_spi_config.ss_pin     = ENV_CS_PIN;
    env_spi_config.mosi_pin   = ENV_MOSI_PIN;
    env_spi_config.miso_pin   = ENV_MISO_PIN;
    env_spi_config.sck_pin    = ENV_SCK_PIN;

#if SPI_USE_INTERRUPT
    err_code = nrf_drv_spi_init(&m_env_spi, &env_spi_config, env_spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
#else
    err_code = nrf_drv_spi_init(&m_env_spi, &env_spi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
#endif
}


static void timer_init(void)
{
    ret_code_t err_code;

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_ble_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                ble_update_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_timer_init(&m_gen_timer, &timer_cfg, general_timer_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&m_gen_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   nrf_drv_timer_ms_to_ticks(&m_gen_timer, GENERAL_TIMER_STEP),
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   true);
}


void peripherals_init(void)
{
    gpio_init();
    nrf_delay_ms(10);

    twi_init();
    nrf_delay_ms(10);

    spi_init();
    nrf_delay_ms(10);

    timer_init();
    nrf_delay_ms(10);
}


/**@brief Function for starting application timers.
 */
void peripherals_start_timers(void)
{
    ret_code_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_ble_timer_id, BLE_UPDATE_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_enable(&m_gen_timer);
}


ret_code_t baro_peripherals_twi_tx(uint16_t device_address, uint8_t *data, uint16_t data_size, bool no_stop)
{
    return nrf_drv_twi_tx(&m_baro_twi, device_address, data, data_size, no_stop);
}


ret_code_t baro_peripherals_twi_rx(uint16_t device_address, uint8_t *data, uint16_t data_size)
{
    return nrf_drv_twi_rx(&m_baro_twi, device_address, data, data_size);
}

ret_code_t env_peripherals_spi_tx(uint8_t *data, uint16_t data_size)
{
    return nrf_drv_spi_transfer(&m_env_spi, data, data_size, NULL, 0);
}

ret_code_t env_peripherals_spi_rx(uint8_t *data, uint16_t data_size)
{
    return nrf_drv_spi_transfer(&m_env_spi, &data[0], 1, &data[1], data_size);
}


void peripherals_delay_ms(uint32_t delay_time_ms)
{
    nrf_delay_ms(delay_time_ms);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void ble_update_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    if (NULL != m_timer_ble_update_handler)
    {
        m_timer_ble_update_handler();
    }
}


void baro_twi_event_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
        {
            break;
        }

        default: break;
    }
}


void eep_twi_event_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
        {
            break;
        }

        default: break;
    }
}


static void env_spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{

}


/**
 * @brief Handler for timer events.
 */
static void general_timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
        {
            if (NULL != m_timer_general_handler)
            {
                m_timer_general_handler();
            }

            if (NULL != m_timer_barometer_handler)
            {
                m_timer_barometer_handler();
            }

            if (NULL != m_timer_environmental_handler)
            {
                m_timer_environmental_handler();
            }
            break;
        }

        default:
        {
            //Do nothing.
            break;
        }
    }
}


void peripherals_assign_comm_handle(uint8_t comm_handle_type, comm_handle_fptr comm_handle)
{
    if (NULL != comm_handle)
    {
        switch (comm_handle_type)
        {
            case BAROMETER_COMM:
            {
                m_barometer_comm_handler = comm_handle;           
                break;
            }

            case ENVIRONMENTAL_COMM:
            {
                m_environmental_comm_handler = comm_handle; 
                break;
            }

            case EEP_COMM:
            {
                m_eeprom_comm_handler = comm_handle; 
                break;
            }

            case TIMER_BAROMETER:
            {
                m_timer_barometer_handler = comm_handle;
                break;
            }

            case TIMER_ENVIRONMENTAL:
            {
                m_timer_environmental_handler = comm_handle;
                break;
            }

            case TIMER_EEP:
            {
                m_timer_eeprom_handler = comm_handle;
                break;
            }

            case TIMER_BLE_UPDATE:
            {
                m_timer_ble_update_handler = comm_handle;
                break;
            }

            case TIMER_GENERAL:
            {
                m_timer_general_handler = comm_handle;
                break;
            }

            default: break;
        }
    }
}


void comm_handle_polling(void)
{
    if (NULL != m_barometer_comm_handler)
    {
        m_barometer_comm_handler();
    }

    if (NULL != m_environmental_comm_handler)
    {
        m_environmental_comm_handler();
    }
}