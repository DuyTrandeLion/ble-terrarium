#include "peripherals.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "nrf_drv_timer.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif
#include "app_uart.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_timer.h"


static const nrf_drv_twi_t m_gbc_twi      = NRF_DRV_TWI_INSTANCE(GBC_TWI_INSTANCE);
static const nrf_drv_twi_t m_ecompass_twi = NRF_DRV_TWI_INSTANCE(ECOMPASS_TWI_INSTANCE);
static const nrf_drv_timer_t m_gnss_timer = NRF_DRV_TIMER_INSTANCE(GNSS_TIMER_INSTANCE);
static const nrf_drv_timer_t m_ubx_timer  = NRF_DRV_TIMER_INSTANCE(GNSS_UBX_TIMER_INSTANCE);
static const nrf_drv_spi_t m_oled_spi     = NRF_DRV_SPI_INSTANCE(OLED_SPI_INSTANCE);

static comm_handle_fptr m_gnss_comm_handle;
static comm_handle_fptr m_barometer_comm_handle;
static comm_handle_fptr m_uart_data_ready_handle;
static comm_handle_fptr m_ecompass_comm_handle;
static comm_handle_fptr m_timer_gnss_handle;
static comm_handle_fptr m_timer_ubx_handle;
static comm_handle_fptr m_timer_barometer_read_sensor_handle;
static comm_handle_fptr m_timer_battery_handler;
static comm_handle_fptr m_timer_lns_handler;
static comm_handle_fptr m_timer_ecompass_handler;
static comm_handle_fptr m_timer_general_handler;

APP_TIMER_DEF(m_battery_timer_id);                                                  /**< Battery timer. */
APP_TIMER_DEF(m_loc_and_nav_timer_id);                                              /**< Location and navigation measurement timer. */


static void gnss_uart_evt_handler(app_uart_evt_t * p_event);
static void battery_level_meas_timeout_handler(void * p_context);
static void loc_and_nav_timeout_handler(void * p_context);
static void timer_gnss_event_handler(nrf_timer_event_t event_type, void* p_context);
static void timer_ubx_event_handler(nrf_timer_event_t event_type, void* p_context);

static void gpio_init(void);
static void uart_init(void);
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


static void uart_init(void)
{
    ret_code_t err_code;

    const app_uart_comm_params_t comm_params =
    {
        GNSS_UART_RX_PIN,
        GNSS_UART_TX_PIN,
        GNSS_UART_RTS_PIN,
        GNSS_UART_CTS_PIN,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
#if defined (UART_PRESENT)
        NRF_UART_BAUDRATE_115200
#else
        NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                         GNSS_UART_RX_BUFF_SIZE,
                         GNSS_UART_TX_BUFF_SIZE,
                         gnss_uart_evt_handler,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);
    APP_ERROR_CHECK(err_code);
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

    err_code = app_timer_create(&m_loc_and_nav_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                loc_and_nav_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_timer_init(&m_gnss_timer, &timer_cfg, timer_gnss_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_timer_init(&m_ubx_timer, &timer_cfg, timer_ubx_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&m_gnss_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   nrf_drv_timer_ms_to_ticks(&m_gnss_timer, UART_TIMER_STEP),
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   true);

    nrf_drv_timer_extended_compare(&m_ubx_timer,
                                   NRF_TIMER_CC_CHANNEL1,
                                   nrf_drv_timer_ms_to_ticks(&m_ubx_timer, TWI_TIMER_STEP),
                                   NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,
                                   true);
}


void peripherals_init(void)
{
    gpio_init();
    nrf_delay_ms(10);

    uart_init();
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

    err_code = app_timer_start(m_loc_and_nav_timer_id, LOC_AND_NAV_DATA_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_enable(&m_gnss_timer);
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


void peripherals_oled_reset(void)
{
    /* CS = High (not selected) */
    nrf_gpio_pin_write(OLED_SS_PIN, GPIO_PIN_SET);

    /* Reset the OLED */
    nrf_gpio_pin_write(OLED_RES_PIN, GPIO_PIN_RESET);
    nrf_delay_ms(10);
    nrf_gpio_pin_write(OLED_RES_PIN, GPIO_PIN_SET);
    nrf_delay_ms(10);
}

void peripherals_oled_write_command(uint8_t *data, uint16_t data_size)
{
    nrf_gpio_pin_write(OLED_DC_PIN, GPIO_PIN_RESET);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_oled_spi, data, data_size, NULL, NULL));
}


void peripherals_oled_write_data(uint8_t *data, uint16_t data_size)
{
    nrf_gpio_pin_write(OLED_DC_PIN, GPIO_PIN_SET);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_oled_spi, data, data_size, NULL, NULL));
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


/**@brief Location and navigation time-out handler.
 *
 * @details This function will be called each time the location and navigation measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the time-out handler.
 */
static void loc_and_nav_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    if (NULL != m_timer_lns_handler)
    {
        m_timer_lns_handler();
    }
}


/**
 * @brief Handler for serial events.
 */
static void gnss_uart_evt_handler(app_uart_evt_t * p_event)
{
    switch (p_event->evt_type)
    {
	case APP_UART_DATA_READY:
	{
            if (NULL != m_uart_data_ready_handle)
            {
                m_uart_data_ready_handle();
            }
	    break;
	}

	case APP_UART_COMMUNICATION_ERROR:
	{
//            APP_ERROR_HANDLER(p_event->data.error_communication);
	    break;
	}

	case APP_UART_FIFO_ERROR:
	{
            APP_ERROR_HANDLER(p_event->data.error_code);
	    break;
	}

	default: break;
    }
}


void gnss_i2c_evt_handler(nrf_drv_twi_evt_t const * p_event,
                                           void * p_context)
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
static void timer_gnss_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
        {
            if (NULL != m_timer_gnss_handle)
            {
                m_timer_gnss_handle();
            }

            if (NULL != m_timer_general_handler)
            {
                m_timer_general_handler();
            }
            break;
        }

        default:
            //Do nothing.
            break;
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
            case GNSS_COMM:
            {
                m_gnss_comm_handle = comm_handle;
                break;
            }

            case BAROMETER_COMM:
            {
                m_barometer_comm_handle = comm_handle;           
                break;
            }

            case ECOMPASS_COMM:
            {
                m_ecompass_comm_handle = comm_handle; 
                break;
            }

            case UART_DATA_READY:
            {
                m_uart_data_ready_handle = comm_handle;
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

            case TIMER_LNS:
            {
                m_timer_lns_handler = comm_handle;
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


void peripherals_app_uart_get(uint8_t * p_byte)
{
    app_uart_get(p_byte);
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