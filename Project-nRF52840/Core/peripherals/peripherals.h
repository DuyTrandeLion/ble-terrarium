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



#define BARO_TWI_INSTANCE               0
#define EEP_TWI_INSTANCE                1

#define ENV_SPI_INSTANCE                2

#define GENERAL_TIMER_INSTANACE         1

#define BARO_I2C_SDA_PIN                ARDUINO_A4_PIN
#define BARO_I2C_SCL_PIN                ARDUINO_A5_PIN

#define EEP_I2C_SDA_PIN                 ARDUINO_SDA_PIN
#define EEP_I2C_SCL_PIN                 ARDUINO_SCL_PIN

#define ENV_SCK_PIN                     ARDUINO_13_PIN
#define ENV_MOSI_PIN                    ARDUINO_11_PIN
#define ENV_MISO_PIN                    ARDUINO_12_PIN
#define ENV_CS_PIN                      ARDUINO_10_PIN

#ifndef GPIO_PIN_SET
#define GPIO_PIN_SET                    1
#endif

#ifndef GPIO_PIN_RESET
#define GPIO_PIN_RESET                  0
#endif

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(5000)                       /**< Battery level measurement interval (ticks). */

#define UART_TIMER_STEP                   (25)   /* ms */
#define TWI_TIMER_STEP                    (200)  /* ms */
#define UART_TIMEOUT_PERIOD               (30)
#define UART_FAULT_DATA_CLEAR             (10)
#define GNSS_TWI_PROCESS_DATA_PERIOD      (100)  /* TWI_TIMER_STEP */
#define BAROMETER_TWI_PROCESS_DATA_PERIOD (500)  /* TWI_TIMER_STEP */
#define ECOMPASS_TWI_PROCESS_DATA_PERIOD  (200)

#define BAROMETER_TRIGGER_PERIOD        3

#define BAROMETER_COMM                  (1)
#define ENVIRONMENTAL_COMM              (BAROMETER_COMM + 1)
#define EEP_COMM                        (ENVIRONMENTAL_COMM + 1)
#define TIMER_BAROMETER                 (EEP_COMM + 1)
#define TIMER_ENVIRONMENTAL             (TIMER_BAROMETER + 1)
#define TIMER_EEP                       (TIMER_ENVIRONMENTAL + 1)
#define TIMER_BATTERY                   (TIMER_EEP + 1)
#define TIMER_GENERAL                   (TIMER_BATTERY + 1)

typedef void (*comm_handle_fptr)(void);

void peripherals_init(void);
void peripherals_start_timers(void);

void peripherals_assign_comm_handle(uint8_t comm_handle_type, comm_handle_fptr comm_handle);

ret_code_t peripherals_twi_tx(uint16_t device_address, uint8_t *data, uint16_t data_size, bool no_stop);
ret_code_t peripherals_twi_rx(uint16_t device_address, uint8_t *data, uint16_t data_size);

ret_code_t peripherals_spi_tx(uint8_t *data, uint16_t data_size);
ret_code_t peripherals_spi_rx(uint8_t *data, uint16_t data_size);

void peripherals_delay_ms(uint32_t delay_time_ms);

void comm_handle_polling(void);


#ifdef __cplusplus
}
#endif

#endif /* _PERIPHERALS_H_ */