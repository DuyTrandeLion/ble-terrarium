#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_error.h"
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define GNSS_UART_INSTANCE              0

#define GBC_TWI_INSTANCE                0
#define ECOMPASS_TWI_INSTANCE           1

#define GNSS_TIMER_INSTANCE             1
#define GNSS_UBX_TIMER_INSTANCE         2

#define OLED_SPI_INSTANCE               2

#define GNSS_UART_RX_PIN                ARDUINO_0_PIN
#define GNSS_UART_TX_PIN                ARDUINO_1_PIN
#define GNSS_UART_CTS_PIN               SER_CON_CTS_PIN
#define GNSS_UART_RTS_PIN               SER_CON_RTS_PIN

#define GBC_I2C_SDA_PIN                 ARDUINO_A4_PIN
#define GBC_I2C_SCL_PIN                 ARDUINO_A5_PIN

#define ECOMPASS_I2C_SDA_PIN            ARDUINO_SDA_PIN
#define ECOMPASS_I2C_SCL_PIN            ARDUINO_SCL_PIN

#define INTERFACE_SELECT_PIN            ARDUINO_A0_PIN

#define OLED_SCK_PIN                    ARDUINO_11_PIN
#define OLED_MOSI_PIN                   ARDUINO_12_PIN
#define OLED_SS_PIN                     ARDUINO_A2_PIN
#define OLED_DC_PIN                     ARDUINO_9_PIN
#define OLED_RES_PIN                    ARDUINO_13_PIN

#ifndef GPIO_PIN_SET
#define GPIO_PIN_SET                    1
#endif

#ifndef GPIO_PIN_RESET
#define GPIO_PIN_RESET                  0
#endif

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(5000)                       /**< Battery level measurement interval (ticks). */
#define LOC_AND_NAV_DATA_INTERVAL       APP_TIMER_TICKS(1000)                       /**< Location and Navigation data interval (ticks). */

#define UART_TIMER_STEP                   (25)   /* ms */
#define TWI_TIMER_STEP                    (200)  /* ms */
#define UART_TIMEOUT_PERIOD               (30)
#define UART_FAULT_DATA_CLEAR             (10)
#define GNSS_TWI_PROCESS_DATA_PERIOD      (100)  /* TWI_TIMER_STEP */
#define BAROMETER_TWI_PROCESS_DATA_PERIOD (500)  /* TWI_TIMER_STEP */
#define ECOMPASS_TWI_PROCESS_DATA_PERIOD  (200)
#define ECOMPASS_CALIB_CHECK_PERIOD       (500)


#define MAX_RECEIVED_SENTENCE_SIZE      (UBXGNSS_NMEA_MAX_LENGTH)

#define GNSS_UART_TX_BUFF_SIZE		(512)
#define GNSS_UART_RX_BUFF_SIZE		(512)

#define BAROMETER_TRIGGER_PERIOD        3

#define GNSS_COMM                       1
#define BAROMETER_COMM                  (GNSS_COMM + 1)
#define ECOMPASS_COMM                   (BAROMETER_COMM + 1)
#define CRYPTO_COMM                     (ECOMPASS_COMM + 1)
#define OLED_COMM                       (CRYPTO_COMM + 1)
#define UART_DATA_READY                 (OLED_COMM + 1)
#define TIMER_GNSS                      (UART_DATA_READY + 1)
#define TIMER_UBX                       (TIMER_GNSS + 1)
#define TIMER_BAROMETER                 (TIMER_UBX + 1)
#define TIMER_ECOMPASS                  (TIMER_BAROMETER + 1)
#define TIMER_BATTERY                   (TIMER_ECOMPASS + 1)
#define TIMER_LNS                       (TIMER_BATTERY + 1)
#define TIMER_GENERAL                   (TIMER_LNS + 1)

typedef void (*comm_handle_fptr)(void);

void peripherals_init(void);
void peripherals_start_timers(void);

void peripherals_assign_comm_handle(uint8_t comm_handle_type, comm_handle_fptr comm_handle);

void peripherals_app_uart_get(uint8_t * p_byte);
ret_code_t peripherals_twi_tx(uint16_t device_address, uint8_t *data, uint16_t data_size, bool no_stop);
ret_code_t peripherals_twi_rx(uint16_t device_address, uint8_t *data, uint16_t data_size);

void peripherals_oled_reset(void);
void peripherals_oled_write_command(uint8_t *data, uint16_t data_size);
void peripherals_oled_write_data(uint8_t *data, uint16_t data_size);
void peripherals_delay_ms(uint32_t delay_time_ms);

void comm_handle_polling(void);


#ifdef __cplusplus
}
#endif

#endif /* _PERIPHERALS_H_ */