#include "sdk_common.h"
#include "ble_ess.h"
#include <string.h>
#include "ble_srv_common.h"
#include "ble_conn_state.h"

#define MAX_WIN_DIRECTION_LENGTH  2
#define MAX_WIN_SPEED_LENGTH      2
#define MAX_ELEVATION_LENGTH      3
#define MAX_HUMIDITY_LENGTH       2
#define MAX_IRRADIANCE_LENGTH     2

static ret_code_t support_descriptor_add(uint16_t char_handle);


void ble_ess_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{

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
    uint8_t initial_gust_factor;
    uint8_t initial_apparent_wind_direction[MAX_WIN_DIRECTION_LENGTH];
    uint8_t initial_apparent_wind_speed[MAX_WIN_SPEED_LENGTH];
    uint8_t initial_elevation[MAX_ELEVATION_LENGTH];
    uint8_t initial_humidity[MAX_HUMIDITY_LENGTH];
    uint8_t initial_irradiance[MAX_IRRADIANCE_LENGTH];
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
    p_ess->conn_handle                    = BLE_CONN_HANDLE_INVALID;

    initial_dew_point               = p_ess_init->initial_dew_point;
    initial_gust_factor             = p_ess_init->initial_gust_factor;
    initial_heat_index              = p_ess_init->initial_heat_index;

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

    return err_code;
}