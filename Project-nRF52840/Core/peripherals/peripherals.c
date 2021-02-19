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

static comm_handle_fptr m_barometer_comm_handle;
static comm_handle_fptr m_timer_barometer_read_sensor_handle;
static comm_handle_fptr m_timer_battery_handler;
static comm_handle_fptr m_timer_ecompass_handler;
static comm_handle_fptr m_timer_general_handler;

APP_TIMER_DEF(m_battery_timer_id);                                                  /**< Battery timer. */
APP_TIMER_DEF(m_loc_and_nav_timer_id);                                              /**< Location and navigation measurement timer. */


static void gnss_uart_evt_handler(app_uart_evt_t * p_event);
static void battery_level_meas_timeout_handler(void * p_context);
static void general_timer_event_handler(nrf_timer_event_t event_type, void* p_context);
static void timer_ubx_event_handler(nrf_timer_event_t event_type, void* p_context);

static void gpio_init(void);
static void twi_init(void);
static void spi_init(void);
static void timer_init(void);


static void gpio_init(void)
{
//    nrf_gpio_cfg_output(INTERFACE_SELECT_PIN);
//    nrf_gpio_pin_set(INTERFACE_SELECT_PIN);

#if defined(SSD1309_USE_SPI)
    nrf_gpio_cfg_output(OLED_RES_PIN);
    nrf_gpio_cfg_output(OLED_DC_PIN);
#endif
}


static void twi_init(void)
{
    ret_code_t err_code;

    nrf_drv_twi_config_t gbc_twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;

    gbc_twi_config.sda = GBC_I2C_SDA_PIN;
    gbc_twi_config.scl = GBC_I2C_SCL_PIN;

    nrf_drv_twi_config_t ecompass_twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;

    ecompass_twi_config.sda = ECOMPASS_I2C_SDA_PIN;
    ecompass_twi_config.scl = ECOMPASS_I2C_SCL_PIN;

    nrf_drv_twi_init(&m_gbc_twi, &gbc_twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_gbc_twi);

    nrf_drv_twi_init(&m_ecompass_twi, &ecompass_twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_ecompass_twi);
}


static void spi_init(void)
{
    ret_code_t err_code;

#if defined(SSD1309_USE_SPI)
    nrf_drv_spi_config_t oled_spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    oled_spi_config.ss_pin   = OLED_SS_PIN;
    oled_spi_config.mosi_pin = OLED_MOSI_PIN;
    oled_spi_config.sck_pin  = OLED_SCK_PIN;
#endif

#if SPI_USE_INTERRUPT
    err_code = nrf_drv_spi_init(&m_oled_spi, &spi_config, oled_spi_callback, NULL);
    APP_ERROR_CHECK(err_code);
#else
    err_code = nrf_drv_spi_init(&m_oled_spi, &oled_spi_config, NULL, NULL);
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

    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_timer_init(&m_gen_timer, &timer_cfg, general_timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_timer_init(&m_ubx_timer, &timer_cfg, timer_ubx_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&m_gen_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   nrf_drv_timer_ms_to_ticks(&m_gen_timer, UART_TIMER_STEP),
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
    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_enable(&m_gen_timer);
    nrf_drv_timer_enable(&m_ubx_timer);
}


ret_code_t peripherals_twi_tx(uint16_t device_address, uint8_t *data, uint16_t data_size, bool no_stop)
{
    return nrf_drv_twi_tx(&m_gbc_twi, device_address, data, data_size, no_stop);
}


ret_code_t peripherals_twi_rx(uint16_t device_address, uint8_t *data, uint16_t data_size)
{
    return nrf_drv_twi_rx(&m_gbc_twi, device_address, data, data_size);
}

ret_code_t peripherals_spi_tx(uint8_t *data, uint16_t data_size)
{

}

ret_code_t peripherals_spi_rx(uint8_t *data, uint16_t data_size)
{

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
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    if (NULL != m_timer_battery_handler)
    {
        m_timer_battery_handler();
    }
}


void baro_twi_evt_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
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


void eep_twi_evt_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
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
            break;
        }

        default:
        {
            //Do nothing.
            break;
        }
    }
}


/**
 * @brief Handler for timer events.
 */
static void timer_ubx_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE1:
        {
            if (NULL != m_timer_ubx_handle)
            {
                m_timer_ubx_handle();
            }

            if (NULL != m_timer_barometer_read_sensor_handle)
            {
                m_timer_barometer_read_sensor_handle();
            }

            if (NULL != m_timer_ecompass_handler)
            {
                m_timer_ecompass_handler();
            }
            break;
        }

        default:
            //Do nothing.
            break;
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
                m_barometer_comm_handle = comm_handle;           
                break;
            }

            case ENVIRONMENTAL_COMM:
            {
                m_ecompass_comm_handle = comm_handle; 
                break;
            }

            case EEP_COMM:
            {
                break;
            }

            case TIMER_GNSS:
            {
                m_timer_gnss_handle = comm_handle;
                break;
            }

            case TIMER_UBX:
            {
                m_timer_ubx_handle = comm_handle;
                break;
            }

            case TIMER_BAROMETER:
            {
                m_timer_barometer_read_sensor_handle = comm_handle;
                break;
            }

            case TIMER_BATTERY:
            {
                m_timer_battery_handler = comm_handle;
                break;
            }

            case TIMER_ECOMPASS:
            {
                m_timer_ecompass_handler = comm_handle;
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
    if (NULL != m_gnss_comm_handle)
    {
        m_gnss_comm_handle();
    }

    if (NULL != m_barometer_comm_handle)
    {
        m_barometer_comm_handle();
    }

    if (NULL != m_ecompass_comm_handle)
    {
        m_ecompass_comm_handle();
    }
}