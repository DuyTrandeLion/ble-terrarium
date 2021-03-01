#ifndef BLE_ESS_H__
#define BLE_ESS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE      0x181A
#define BLE_UUID_ELEVATION                          0x2A6C
#define BLE_UUID_PRESSURE                           0x2A6D
#define BLE_UUID_TEMPERATURE                        0x2A6E
#define BLE_UUID_HUMIDITY                           0x2A6F
#define BLE_UUID_TRUE_WIND_SPEED                    0x2A70
#define BLE_UUID_TRUE_WIND_DIRECTION                0x2A71
#define BLE_UUID_APPARENT_WIND_SPEED                0x2A72
#define BLE_UUID_APPARENT_WIND_DIRECTION            0x2A73
#define BLE_UUID_GUST_FACTOR                        0x2A74
#define BLE_UUID_POLLEN_CONCENTRATION               0x2A75
#define BLE_UUID_UV_INDEX                           0x2A76
#define BLE_UUID_IRRADIANCE                         0x2A77
#define BLE_UUID_RAINFALL                           0x2A78
#define BLE_UUID_WIND_CHILL                         0x2A79
#define BLE_UUID_HEAT_INDEX                         0x2A7A
#define BLE_UUID_DEW_POINT                          0x2A7B
#define BLE_UUID_MAGNETIC_DECLINATION               0x2A7C
#define BLE_UUID_DESCRIPTOR_VALUE_CHANGED           0x2A7D
#define BLE_UUID_MAGNETIC_FLUX_DENSITY_2D           0x2AA0
#define BLE_UUID_MAGNETIC_FLUX_DENSITY_3D           0x2AA1
#define BLE_UUID_BAROMETRIC_PRESSURE_TREND          0x2AA3


#define BLE_ESS_BLE_OBSERVER_PRIO                   2

/**@brief Macro for defining a ble_env instance.
 *
 * @param   _name  Name of the instance.
 * @hideinitializer
 */
#define BLE_ESS_DEF(_name)                          \
    static ble_ess_t _name;                         \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,             \
                         BLE_ESS_BLE_OBSERVER_PRIO, \
                         ble_ess_on_ble_evt,        \
                         &_name)

/**@brief Environmental Sensing Service event type. */
typedef enum
{
    BLE_ESS_EVT_NOTIFICATION_ENABLED, /**< Battery value notification enabled event. */
    BLE_ESS_EVT_NOTIFICATION_DISABLED /**< Battery value notification disabled event. */
} ble_ess_evt_type_t;

/**@brief Environmental Sensing Service event. */
typedef struct
{
    ble_ess_evt_type_t evt_type;    /**< Type of event. */
} ble_ess_evt_t;

// Forward declaration of the ble_ess_t type.
typedef struct ble_ess_s ble_ess_t;

/**@brief Environmental Sensing Service event handler type. */
typedef void (* ble_ess_evt_handler_t) (ble_ess_t * p_ess, ble_ess_evt_t * p_evt);

/**@brief Environmental Sensing Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_ess_evt_handler_t   evt_handler;                    /**< Event handler to be called for handling events in the Battery Service. */
    bool                    support_dc_notification;
    bool                    support_dc_writable_aux;
    bool                    support_awd_notification;
    bool                    support_awd_writable_aux;
    bool                    support_aws_notification;
    bool                    support_aws_writable_aux;
    bool                    support_dp_notification;
    bool                    support_dp_writable_aux;
    bool                    support_el_notification;
    bool                    support_el_writable_aux;
    bool                    support_gf_notification;
    bool                    support_gf_writable_aux;
    bool                    support_hi_notification;
    bool                    support_hi_writable_aux;
    bool                    support_hum_notification;
    bool                    support_hum_writable_aux;
    bool                    support_ird_notification;
    bool                    support_ird_writable_aux;
    bool                    support_pc_notification;
    bool                    support_pc_writable_aux;
    bool                    support_rf_notification;
    bool                    support_rf_writable_aux;
    bool                    support_ps_notification;
    bool                    support_ps_writable_aux;
    bool                    support_tem_notification;
    bool                    support_tem_writable_aux;
    bool                    support_twd_notification;
    bool                    support_twd_writable_aux;
    bool                    support_tws_notification;
    bool                    support_tws_writable_aux;
    bool                    support_uvi_notification;
    bool                    support_uvi_writable_aux;
    bool                    support_wc_notification;
    bool                    support_wc_writable_aux;
    bool                    support_bpt_notification;
    bool                    support_bpt_writable_aux;
    bool                    support_md_notification;
    bool                    support_md_writable_aux;
    bool                    support_mfd2d_notification;
    bool                    support_mfd2d_writable_aux;
    bool                    support_mfd3d_notification;
    bool                    support_mfd3d_writable_aux;

    int8_t                  initial_dew_point;
    int8_t                  initial_heat_index;
    int8_t                  initial_wind_chill;
    uint8_t                 initial_gust_factor;
    uint8_t                 initial_uv_index;
    uint8_t                 initial_barometric_pressure_trend;
    int16_t                 initial_temperature;
    int16_t                 initial_magnetic_flux_density_2d;
    int16_t                 initial_magnetic_flux_density_3d;
    uint16_t                initial_apparent_wind_direction;
    uint16_t                initial_apparent_wind_speed;
    uint16_t                initial_humidity;
    uint16_t                initial_irradiance;
    uint16_t                initial_true_wind_direction;
    uint16_t                initial_true_wind_speed;
    uint16_t                initial_rainfall;
    uint16_t                initial_magnetic_declination;
    int32_t                 initial_elevation;
    uint32_t                initial_pollen_concentration;
    uint32_t                initial_pressure; 

    security_req_t          awd_cccd_wr_sec;                 /**< Security requirement for writing the AWD characteristic CCCD. */
    security_req_t          aws_cccd_wr_sec;                 /**< Security requirement for writing the AWD characteristic CCCD. */
    security_req_t          dp_cccd_wr_sec;
    security_req_t          el_cccd_wr_sec;
    security_req_t          gf_cccd_wr_sec;
    security_req_t          hi_cccd_wr_sec;
    security_req_t          hum_cccd_wr_sec;
    security_req_t          ird_cccd_wr_sec;
    security_req_t          pc_cccd_wr_sec;
    security_req_t          rf_cccd_wr_sec;
    security_req_t          ps_cccd_wr_sec;
    security_req_t          tem_cccd_wr_sec;
    security_req_t          twd_cccd_wr_sec;                 /**< Security requirement for writing the AWD characteristic CCCD. */
    security_req_t          tws_cccd_wr_sec; 
    security_req_t          uvi_cccd_wr_sec;
    security_req_t          wc_cccd_wr_sec;
    security_req_t          bpt_cccd_wr_sec;
    security_req_t          md_cccd_wr_sec;
    security_req_t          mfd2d_cccd_wr_sec;
    security_req_t          mfd3d_cccd_wr_sec;
} ble_ess_init_t;


/**@brief Environmental Sensing Service structure. This contains various status information for the service. */
struct ble_ess_s
{
    ble_ess_evt_handler_t     evt_handler;               /**< Event handler to be called for handling events in the Battery Service. */
    bool                      is_dc_notification_supported;
    bool                      is_dc_writable_aux_supported;
    bool                      is_awd_notification_supported;
    bool                      is_awd_writable_aux_supported;
    bool                      is_aws_notification_supported;
    bool                      is_aws_writable_aux_supported;
    bool                      is_dp_notification_supported;
    bool                      is_dp_writable_aux_supported;
    bool                      is_el_notification_supported;
    bool                      is_el_writable_aux_supported;
    bool                      is_gf_notification_supported;
    bool                      is_gf_writable_aux_supported;
    bool                      is_hi_notification_supported;
    bool                      is_hi_writable_aux_supported;
    bool                      is_hum_notification_supported;
    bool                      is_hum_writable_aux_supported;
    bool                      is_ird_notification_supported;
    bool                      is_ird_writable_aux_supported;
    bool                      is_pc_notification_supported;
    bool                      is_pc_writable_aux_supported;
    bool                      is_rf_notification_supported;
    bool                      is_rf_writable_aux_supported;
    bool                      is_ps_notification_supported;
    bool                      is_ps_writable_aux_supported;
    bool                      is_tem_notification_supported;
    bool                      is_tem_writable_aux_supported;
    bool                      is_twd_notification_supported;
    bool                      is_twd_writable_aux_supported;
    bool                      is_tws_notification_supported;
    bool                      is_tws_writable_aux_supported;
    bool                      is_uvi_notification_supported;
    bool                      is_uvi_writable_aux_supported;
    bool                      is_wc_notification_supported;
    bool                      is_wc_writable_aux_supported;
    bool                      is_bpt_notification_supported;
    bool                      is_bpt_writable_aux_supported;
    bool                      is_md_notification_supported;
    bool                      is_md_writable_aux_supported;
    bool                      is_mfd2d_notification_supported;
    bool                      is_mfd2d_writable_aux_supported;
    bool                      is_mfd3d_notification_supported;
    bool                      is_mfd3d_writable_aux_supported;
    uint16_t                  service_handle;            /**< Handle of Battery Service (as provided by the BLE stack). */
    uint16_t                  conn_handle;               /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    ble_gatts_char_handles_t  dc_handles;
    ble_gatts_char_handles_t  awd_handles;
    ble_gatts_char_handles_t  aws_handles;
    ble_gatts_char_handles_t  dp_handles;
    ble_gatts_char_handles_t  el_handles;
    ble_gatts_char_handles_t  gf_handles;
    ble_gatts_char_handles_t  hi_handles;
    ble_gatts_char_handles_t  hum_handles;
    ble_gatts_char_handles_t  ird_handles;
    ble_gatts_char_handles_t  pc_handles;
    ble_gatts_char_handles_t  rf_handles;
    ble_gatts_char_handles_t  ps_handles;
    ble_gatts_char_handles_t  tem_handles;
    ble_gatts_char_handles_t  twd_handles;
    ble_gatts_char_handles_t  tws_handles;
    ble_gatts_char_handles_t  uvi_handles;
    ble_gatts_char_handles_t  wc_handles;
    ble_gatts_char_handles_t  bpt_handles;
    ble_gatts_char_handles_t  md_handles;
    ble_gatts_char_handles_t  mfd2d_handles;
    ble_gatts_char_handles_t  mfd3d_handles;
};


/**@brief Function for initializing the Environmental Sensing Service.
 *
 * @param[out]  p_ess       Environmental Sensing Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_ess_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
ret_code_t ble_ess_init(ble_ess_t * p_ess, const ble_ess_init_t * p_ess_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Environmental Sensing Service.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   Environmental Sensing Service structure.
 */
void ble_ess_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_ESS_H__
