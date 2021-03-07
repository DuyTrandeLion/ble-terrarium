#include "sdk_common.h"
#include "ble_ess.h"
#include <string.h>
#include "ble_srv_common.h"
#include "ble_conn_state.h"

#define MAX_WIN_DIRECTION_LENGTH          2
#define MAX_WIN_SPEED_LENGTH              2
#define MAX_ELEVATION_LENGTH              3
#define MAX_HUMIDITY_LENGTH               2
#define MAX_IRRADIANCE_LENGTH             2
#define MAX_POLLEN_CONCENTRATION_LENGTH   3
#define MAX_RAINFALL_LENGTH               2
#define MAX_PRESSURE_LENGTH               4
#define MAX_TEMPERATURE_LENGTH            2
#define MAX_MAGNETIC_DECLINATION_LENGTH   2
#define MAX_MAGNETIC_FLUX_DENSITY_LENGTH  2

static ret_code_t support_descriptor_add(uint16_t char_handle);


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ess       Environmental Sensing Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_ess_t * p_ess, ble_evt_t const * p_ble_evt)
{
    p_ess->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ess       Environmental Sensing Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_ess_t * p_ess, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ess->conn_handle = BLE_CONN_HANDLE_INVALID;
}


void ble_ess_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_ess_t * p_ess = (ble_ess_t *) p_context;
    
    if (p_ess == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            on_connect(p_ess, p_ble_evt);
            break;
        }

        case BLE_GAP_EVT_DISCONNECTED:
        {
            on_disconnect(p_ess, p_ble_evt);
            break;
        }

        case BLE_GATTS_EVT_WRITE:
        {
           break;
        }

        default:
        {
            // No implementation needed.
            break;
        }
    }
}


static ret_code_t support_descriptor_add(uint16_t char_handle)
{
    ret_code_t err_code;

    ble_add_descr_params_t add_char_descriptor_params;

    // Add Environmental Sensing Measurement desriptor
    memset(&add_char_descriptor_params, 0, sizeof(add_char_descriptor_params));

    add_char_descriptor_params.uuid = 0x290C;
    add_char_descriptor_params.p_value = "ES Measurement";
    add_char_descriptor_params.read_access = 1;

    err_code = descriptor_add(char_handle,
                              &add_char_descriptor_params,
                              NULL);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Environmental Sensing Trigger Setting desriptor
    memset(&add_char_descriptor_params, 0, sizeof(add_char_descriptor_params));

    add_char_descriptor_params.uuid = 0x290D;
    add_char_descriptor_params.p_value = "ES Trigger Setting";
    add_char_descriptor_params.read_access = 1;

    err_code = descriptor_add(char_handle,
                              &add_char_descriptor_params,
                              NULL);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Environmental Sensing Configuration desriptor
    memset(&add_char_descriptor_params, 0, sizeof(add_char_descriptor_params));

    add_char_descriptor_params.uuid = 0x290B;
    add_char_descriptor_params.p_value = "ES Configuration";
    add_char_descriptor_params.read_access = 1;

    err_code = descriptor_add(char_handle,
                              &add_char_descriptor_params,
                              NULL);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Valid Range desriptor
    memset(&add_char_descriptor_params, 0, sizeof(add_char_descriptor_params));

    add_char_descriptor_params.uuid = 0x2906;
    add_char_descriptor_params.p_value = "Valid Range";
    add_char_descriptor_params.read_access = 1;

    err_code = descriptor_add(char_handle,
                              &add_char_descriptor_params,
                              NULL);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return err_code;
}


ret_code_t ble_ess_init(ble_ess_t * p_ess, const ble_ess_init_t * p_ess_init)
{
    int8_t  initial_dew_point;
    int8_t  initial_heat_index;
    int8_t  initial_wind_chill;
    uint8_t initial_gust_factor;
    uint8_t initial_uv_index;
    uint8_t initial_barometric_pressure_trend;
    uint8_t initial_apparent_wind_direction[MAX_WIN_DIRECTION_LENGTH];
    uint8_t initial_apparent_wind_speed[MAX_WIN_SPEED_LENGTH];
    uint8_t initial_elevation[MAX_ELEVATION_LENGTH];
    uint8_t initial_humidity[MAX_HUMIDITY_LENGTH];
    uint8_t initial_irradiance[MAX_IRRADIANCE_LENGTH];
    uint8_t initial_pollen_concentration[MAX_POLLEN_CONCENTRATION_LENGTH];
    uint8_t initial_rainfall[MAX_RAINFALL_LENGTH];
    uint8_t initial_pressure[MAX_PRESSURE_LENGTH];
    uint8_t initial_temperature[MAX_TEMPERATURE_LENGTH];
    uint8_t initial_true_wind_direction[MAX_WIN_DIRECTION_LENGTH];
    uint8_t initial_true_wind_speed[MAX_WIN_SPEED_LENGTH];
    uint8_t initial_magnetic_declination[MAX_MAGNETIC_DECLINATION_LENGTH];
    uint8_t initial_magnetic_flux_density_2d[MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 2];
    uint8_t initial_magnetic_flux_density_3d[MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 3];
    ret_code_t err_code;
    ble_uuid_t ble_uuid;
    ble_add_char_params_t add_char_params;
    ble_add_char_user_desc_t user_descr_params;

    if (p_ess == NULL || p_ess_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize service structure
    p_ess->evt_handler                    = p_ess_init->evt_handler;
    p_ess->is_dc_notification_supported   = p_ess_init->support_dc_notification;
    p_ess->is_dc_writable_aux_supported   = p_ess_init->support_dc_writable_aux;
    p_ess->is_awd_notification_supported  = p_ess_init->support_awd_notification;
    p_ess->is_awd_writable_aux_supported  = p_ess_init->support_awd_writable_aux;
    p_ess->is_aws_notification_supported  = p_ess_init->support_aws_notification;
    p_ess->is_aws_writable_aux_supported  = p_ess_init->support_aws_writable_aux;
    p_ess->is_dp_notification_supported   = p_ess_init->support_dp_notification;
    p_ess->is_dp_writable_aux_supported   = p_ess_init->support_dp_writable_aux;
    p_ess->is_el_notification_supported   = p_ess_init->support_el_notification;
    p_ess->is_el_writable_aux_supported   = p_ess_init->support_el_writable_aux;
    p_ess->is_gf_notification_supported   = p_ess_init->support_gf_notification;
    p_ess->is_gf_writable_aux_supported   = p_ess_init->support_gf_writable_aux;
    p_ess->is_hi_notification_supported   = p_ess_init->support_hi_notification;
    p_ess->is_hi_writable_aux_supported   = p_ess_init->support_hi_writable_aux;
    p_ess->is_hum_notification_supported  = p_ess_init->support_hum_notification;
    p_ess->is_hum_writable_aux_supported  = p_ess_init->support_hum_writable_aux;
    p_ess->is_ird_notification_supported  = p_ess_init->support_ird_notification;
    p_ess->is_ird_writable_aux_supported  = p_ess_init->support_ird_writable_aux;
    p_ess->is_pc_notification_supported   = p_ess_init->support_pc_notification;
    p_ess->is_pc_writable_aux_supported   = p_ess_init->support_pc_writable_aux;
    p_ess->is_rf_notification_supported   = p_ess_init->support_rf_notification;
    p_ess->is_rf_writable_aux_supported   = p_ess_init->support_rf_writable_aux;
    p_ess->is_ps_notification_supported   = p_ess_init->support_ps_notification;
    p_ess->is_ps_writable_aux_supported   = p_ess_init->support_ps_writable_aux;
    p_ess->is_tem_notification_supported  = p_ess_init->support_tem_notification;
    p_ess->is_tem_writable_aux_supported  = p_ess_init->support_tem_writable_aux;
    p_ess->is_twd_notification_supported  = p_ess_init->support_twd_notification;
    p_ess->is_twd_writable_aux_supported  = p_ess_init->support_twd_writable_aux;
    p_ess->is_tws_notification_supported  = p_ess_init->support_tws_notification;
    p_ess->is_tws_writable_aux_supported  = p_ess_init->support_tws_writable_aux;
    p_ess->is_uvi_notification_supported  = p_ess_init->support_uvi_notification;
    p_ess->is_uvi_writable_aux_supported  = p_ess_init->support_uvi_writable_aux;
    p_ess->is_wc_notification_supported   = p_ess_init->support_wc_notification;
    p_ess->is_wc_writable_aux_supported   = p_ess_init->support_wc_writable_aux;
    p_ess->is_bpt_notification_supported  = p_ess_init->support_bpt_notification;
    p_ess->is_bpt_writable_aux_supported  = p_ess_init->support_bpt_writable_aux;
    p_ess->is_md_notification_supported   = p_ess_init->support_md_notification;
    p_ess->is_md_writable_aux_supported   = p_ess_init->support_md_writable_aux;
    p_ess->is_mfd2d_notification_supported  = p_ess_init->support_mfd2d_notification;
    p_ess->is_mfd2d_writable_aux_supported  = p_ess_init->support_mfd2d_writable_aux;
    p_ess->is_mfd3d_notification_supported  = p_ess_init->support_mfd3d_notification;
    p_ess->is_mfd3d_writable_aux_supported  = p_ess_init->support_mfd3d_writable_aux;
    p_ess->conn_handle                    = BLE_CONN_HANDLE_INVALID;

    initial_dew_point               = p_ess_init->initial_dew_point;
    initial_gust_factor             = p_ess_init->initial_gust_factor;
    initial_heat_index              = p_ess_init->initial_heat_index;
    initial_uv_index                = p_ess_init->initial_uv_index;
    initial_wind_chill              = p_ess_init->initial_wind_chill;
    initial_barometric_pressure_trend = p_ess_init->initial_barometric_pressure_trend;

    uint16_encode(p_ess_init->initial_apparent_wind_direction, initial_apparent_wind_direction);
    uint16_encode(p_ess_init->initial_apparent_wind_speed, initial_apparent_wind_speed);
    uint24_encode(p_ess_init->initial_elevation, initial_elevation);
    uint16_encode(p_ess_init->initial_humidity, initial_humidity);
    uint16_encode(p_ess_init->initial_irradiance, initial_irradiance);
    uint24_encode(p_ess_init->initial_pollen_concentration, initial_pollen_concentration);
    uint16_encode(p_ess_init->initial_rainfall, initial_rainfall);
    uint32_encode(p_ess_init->initial_pressure, initial_pressure);
    uint16_encode(p_ess_init->initial_temperature, initial_temperature);
    uint16_encode(p_ess_init->initial_true_wind_direction, initial_true_wind_direction);
    uint16_encode(p_ess_init->initial_true_wind_speed, initial_true_wind_speed);
    uint16_encode(p_ess_init->initial_magnetic_declination, initial_magnetic_declination);
    uint16_encode(p_ess_init->initial_magnetic_flux_density_2d.magnetic_flux_density_x, &initial_magnetic_flux_density_2d[2]);
    uint16_encode(p_ess_init->initial_magnetic_flux_density_2d.magnetic_flux_density_y, &initial_magnetic_flux_density_2d[0]);
    uint16_encode(p_ess_init->initial_magnetic_flux_density_3d.magnetic_flux_density_y, &initial_magnetic_flux_density_3d[4]);
    uint16_encode(p_ess_init->initial_magnetic_flux_density_3d.magnetic_flux_density_x, &initial_magnetic_flux_density_3d[2]);
    uint16_encode(p_ess_init->initial_magnetic_flux_density_3d.magnetic_flux_density_z, &initial_magnetic_flux_density_3d[0]);

    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ess->service_handle);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Descriptor Value Changed characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_DESCRIPTOR_VALUE_CHANGED;
    add_char_params.char_props.notify     = p_ess->is_dc_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_dc_writable_aux_supported;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->dc_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Configure User Descriptor parameters
    memset(&user_descr_params, 0, sizeof(user_descr_params));
    user_descr_params.p_char_user_desc = "Characteristic User Description";
    user_descr_params.read_access = SEC_OPEN;

    // Add Apparent Wind Direction characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_APPARENT_WIND_DIRECTION;
    add_char_params.max_len               = MAX_WIN_DIRECTION_LENGTH;
    add_char_params.init_len              = MAX_WIN_DIRECTION_LENGTH;
    add_char_params.p_init_value          = initial_apparent_wind_direction;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_awd_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_awd_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->awd_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->awd_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->awd_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Apparent Wind Speed characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_APPARENT_WIND_SPEED;
    add_char_params.max_len               = MAX_WIN_SPEED_LENGTH;
    add_char_params.init_len              = MAX_WIN_SPEED_LENGTH;
    add_char_params.p_init_value          = initial_apparent_wind_speed;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_aws_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_aws_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->aws_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->aws_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->aws_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Dew Point characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_DEW_POINT;
    add_char_params.max_len               = sizeof(int8_t);
    add_char_params.init_len              = sizeof(int8_t);
    add_char_params.p_init_value          = &initial_dew_point;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_dp_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_dp_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->dp_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->dp_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->dp_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Elevation characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_ELEVATION;
    add_char_params.max_len               = MAX_ELEVATION_LENGTH;
    add_char_params.init_len              = MAX_ELEVATION_LENGTH;
    add_char_params.p_init_value          = initial_elevation;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_el_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_el_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->el_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->el_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->el_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Gust Factor characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_GUST_FACTOR;
    add_char_params.max_len               = sizeof(uint8_t);
    add_char_params.init_len              = sizeof(uint8_t);
    add_char_params.p_init_value          = &initial_gust_factor;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_gf_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_gf_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->gf_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->gf_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->gf_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Heat Index characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_HEAT_INDEX;
    add_char_params.max_len               = sizeof(int8_t);
    add_char_params.init_len              = sizeof(int8_t);
    add_char_params.p_init_value          = &initial_heat_index;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_hi_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_hi_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->hi_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->hi_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->hi_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Humidity characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_HUMIDITY;
    add_char_params.max_len               = MAX_HUMIDITY_LENGTH;
    add_char_params.init_len              = MAX_HUMIDITY_LENGTH;
    add_char_params.p_init_value          = initial_humidity;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_hum_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_hum_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->hum_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->hum_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->hum_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Irradiance characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_IRRADIANCE;
    add_char_params.max_len               = MAX_IRRADIANCE_LENGTH;
    add_char_params.init_len              = MAX_IRRADIANCE_LENGTH;
    add_char_params.p_init_value          = initial_irradiance;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_ird_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_ird_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->ird_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->ird_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->ird_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Pollen Concentration characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_POLLEN_CONCENTRATION;
    add_char_params.max_len               = MAX_POLLEN_CONCENTRATION_LENGTH;
    add_char_params.init_len              = MAX_POLLEN_CONCENTRATION_LENGTH;
    add_char_params.p_init_value          = initial_pollen_concentration;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_pc_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_pc_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->pc_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->pc_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->pc_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Rainfall characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_RAINFALL;
    add_char_params.max_len               = MAX_RAINFALL_LENGTH;
    add_char_params.init_len              = MAX_RAINFALL_LENGTH;
    add_char_params.p_init_value          = initial_rainfall;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_rf_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_rf_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->rf_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->rf_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->rf_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Pressure characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_PRESSURE;
    add_char_params.max_len               = MAX_PRESSURE_LENGTH;
    add_char_params.init_len              = MAX_PRESSURE_LENGTH;
    add_char_params.p_init_value          = initial_pressure;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_ps_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_ps_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->ps_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->ps_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->ps_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Temperature characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_TEMPERATURE;
    add_char_params.max_len               = MAX_TEMPERATURE_LENGTH;
    add_char_params.init_len              = MAX_TEMPERATURE_LENGTH;
    add_char_params.p_init_value          = initial_temperature;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_tem_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_tem_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->tem_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->tem_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->tem_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add True Wind Direction characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_TRUE_WIND_DIRECTION;
    add_char_params.max_len               = MAX_WIN_DIRECTION_LENGTH;
    add_char_params.init_len              = MAX_WIN_DIRECTION_LENGTH;
    add_char_params.p_init_value          = initial_true_wind_direction;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_twd_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_twd_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->twd_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->twd_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->twd_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add True Wind Speed characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_TRUE_WIND_SPEED;
    add_char_params.max_len               = MAX_WIN_SPEED_LENGTH;
    add_char_params.init_len              = MAX_WIN_SPEED_LENGTH;
    add_char_params.p_init_value          = initial_true_wind_speed;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_tws_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_tws_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->tws_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->tws_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->tws_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add UV Index characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_UV_INDEX;
    add_char_params.max_len               = sizeof(uint8_t);
    add_char_params.init_len              = sizeof(uint8_t);
    add_char_params.p_init_value          = &initial_uv_index;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_uvi_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_uvi_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->uvi_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->uvi_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->uvi_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Wind Chill characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_WIND_CHILL;
    add_char_params.max_len               = sizeof(int8_t);
    add_char_params.init_len              = sizeof(int8_t);
    add_char_params.p_init_value          = &initial_wind_chill;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_wc_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_wc_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->wc_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->wc_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->wc_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Barometric Pressure Trend characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_BAROMETRIC_PRESSURE_TREND;
    add_char_params.max_len               = sizeof(uint8_t);
    add_char_params.init_len              = sizeof(uint8_t);
    add_char_params.p_init_value          = &initial_barometric_pressure_trend;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_bpt_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_bpt_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->bpt_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->bpt_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->bpt_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Magnetic Declination characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_MAGNETIC_DECLINATION;
    add_char_params.max_len               = MAX_MAGNETIC_DECLINATION_LENGTH;
    add_char_params.init_len              = MAX_MAGNETIC_DECLINATION_LENGTH;
    add_char_params.p_init_value          = initial_magnetic_declination;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_md_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_md_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->md_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->md_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->md_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Magnetic Flux Density 2D characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_MAGNETIC_FLUX_DENSITY_2D;
    add_char_params.max_len               = MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 2;
    add_char_params.init_len              = MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 2;
    add_char_params.p_init_value          = initial_magnetic_flux_density_2d;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_mfd2d_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_mfd2d_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->mfd2d_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->mfd2d_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->mfd2d_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Magnetic Flux Density 3D characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid                  = BLE_UUID_MAGNETIC_FLUX_DENSITY_3D;
    add_char_params.max_len               = MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 3;
    add_char_params.init_len              = MAX_MAGNETIC_FLUX_DENSITY_LENGTH * 3;
    add_char_params.p_init_value          = initial_magnetic_flux_density_3d;
    add_char_params.char_props.read       = 1;
    add_char_params.char_props.notify     = p_ess->is_mfd3d_notification_supported;
    add_char_params.char_ext_props.wr_aux = p_ess->is_mfd3d_writable_aux_supported;
    add_char_params.p_user_descr          = &user_descr_params;
    add_char_params.read_access           = p_ess_init->mfd3d_cccd_rd_sec;

    err_code = characteristic_add(p_ess->service_handle,
                                  &add_char_params,
                                  &(p_ess->mfd3d_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = support_descriptor_add(p_ess->mfd3d_handles.value_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return err_code;
}


ret_code_t ble_ess_apparent_wind_direction_update(ble_ess_t * p_ess,
                                                  uint16_t    apparent_wind_direction,
                                                  uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_apparent_wind_speed_update(ble_ess_t * p_ess,
                                              uint16_t    apparent_wind_speed,
                                              uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_dew_point_update(ble_ess_t * p_ess,
                                    int8_t      dew_point,
                                    uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_elevation_update(ble_ess_t * p_ess,
                                    int32_t     elevation,
                                    uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_gust_factor_update(ble_ess_t * p_ess,
                                      uint8_t     gust_factor,
                                      uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_heat_index_update(ble_ess_t * p_ess,
                                     int8_t      heat_index,
                                     uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_humidity_update(ble_ess_t * p_ess,
                                   uint16_t    humidity,
                                   uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_irradiance_update(ble_ess_t * p_ess,
                                     uint16_t    irradiance,
                                     uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_pollen_concentration_update(ble_ess_t * p_ess,
                                               uint32_t    pollen_concentration,
                                               uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_rainfall_update(ble_ess_t * p_ess,
                                   uint16_t    rainfall,
                                   uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_pressure_update(ble_ess_t * p_ess,
                                   uint32_t    pressure,
                                   uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_temperature_update(ble_ess_t * p_ess,
                                      int16_t     temperature,
                                      uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t              err_code = NRF_SUCCESS;
    ble_gatts_value_t       gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint8_t encodded_temperature[MAX_TEMPERATURE_LENGTH];

        // Initialize value struct.
        memset(&gatts_value, 0, sizeof(gatts_value));

        uint16_encode(temperature, encodded_temperature);
        
        gatts_value.len     = MAX_TEMPERATURE_LENGTH;
        gatts_value.offset  = 0;
        gatts_value.p_value = encodded_temperature;
        
        // Update database.
        err_code = sd_ble_gatts_value_set(p_ess->conn_handle,
                                          p_ess->tem_handles.value_handle,
                                          &gatts_value);

        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        // Send value if connected and notifying.
        if (p_ess->is_twd_notification_supported)
        {
            ble_gatts_hvx_params_t  hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));

            hvx_params.handle = p_ess->tem_handles.value_handle;
            hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = gatts_value.offset;
            hvx_params.p_len  = &gatts_value.len;
            hvx_params.p_data = gatts_value.p_value;

            err_code = sd_ble_gatts_hvx(p_ess->conn_handle, &hvx_params);
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_true_wind_direction_update(ble_ess_t * p_ess,
                                              uint16_t    true_wind_direction,
                                              uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_true_wind_speed_update(ble_ess_t * p_ess,
                                          uint16_t    true_wind_speed,
                                          uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_uv_index_update(ble_ess_t * p_ess,
                                   uint8_t     uv_index,
                                   uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_wind_chill_update(ble_ess_t * p_ess,
                                     int8_t      wind_chill,
                                     uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_barometric_pressure_trend_update(ble_ess_t * p_ess,
                                                    uint8_t     barometric_pressure_trend,
                                                    uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_magnetic_declination_update(ble_ess_t * p_ess,
                                               uint16_t    magnetic_declination,
                                               uint16_t    conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_mfd2d_update(ble_ess_t                  * p_ess,
                                magnetic_flux_density_2d_t   mdf2d,
                                uint16_t                     conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}


ret_code_t ble_ess_mfd3d_update(ble_ess_t                  * p_ess,
                                magnetic_flux_density_3d_t   mdf3d,
                                uint16_t                     conn_handle)
{
    if (p_ess == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t         err_code = NRF_SUCCESS;
    ble_gatts_value_t  gatts_value;

    if (p_ess->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}