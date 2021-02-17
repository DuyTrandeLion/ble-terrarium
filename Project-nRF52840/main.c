/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_app_lns_main main.h
 * @{
 * @ingroup ble_app_lns_main
 * @brief Health Thermometer Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Health Thermometer service
 * It also includes the sample code for Battery and Device Information services.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_dis.h"
#include "ble_bas.h"
#include "ble_lns.h"
#include "ble_ans_c.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "sensorsim.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "nrf_delay.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_lesc.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "peripherals.h"
#include "barometer.h"
#include "environmental.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define MANUFACTURER_NAME               "NordicSemiconductor"                       /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUMBER                    "nRF52"                                     /**< Model Number string. Will be passed to Device Information Service. */
#define MANUFACTURER_ID                 0x55AA55AA55                                /**< DUMMY Manufacturer ID. Will be passed to Device Information Service. You shall use the ID for your Company*/
#define ORG_UNIQUE_ID                   0xEEBBEE                                    /**< DUMMY Organisation Unique ID. Will be passed to Device Information Service. You shall use the Organisation Unique ID relevant for your Company */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL                40                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_BATTERY_LEVEL               81                                          /**< Minimum battery level as returned by the simulated measurement function. */
#define MAX_BATTERY_LEVEL               100                                         /**< Maximum battery level as returned by the simulated measurement function. */
#define BATTERY_LEVEL_INCREMENT         1                                           /**< Value by which the battery level is incremented/decremented for each call to the simulated measurement function. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(10, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (10 ms). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (100 ms) */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAM_UPDATE_COUNT     3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define MESSAGE_BUFFER_SIZE             24                                          /**< Size of buffer holding optional messages in notifications. */
#define BLE_ANS_NB_OF_CATEGORY_ID       10                                          /**< Number of categories. */

#define LESC_DEBUG_MODE                 0                                           /**< Set to 1 to use LESC debug keys, allows you to use a sniffer to inspect traffic. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  1                                           /**< Man In The Middle protection required (applicable when display module is detected). */
#define SEC_PARAM_LESC                  1                                           /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_DISPLAY_ONLY                /**< Display I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define PASSKEY_TXT                     "Passkey:"                                  /**< Message to be displayed together with the pass-key. */
#define PASSKEY_TXT_LENGTH              8                                           /**< Length of message to be displayed together with the pass-key. */
#define PASSKEY_LENGTH                  6                                           /**< Length of pass-key received by the stack for display. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define TRAVEL_READY                    0
#define TRAVEL_STARTED                  1
#define TRAVEL_PAUSED                   2

BLE_ANS_C_DEF(m_ans_c);                                                             /**< Structure used to identify the Alert Notification Service Client. */
BLE_BAS_DEF(m_bas);                                                                 /**< Structure used to identify the battery service. */
BLE_LNS_DEF(m_lns);                                                                 /**< Location and navigation service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);                                           /**< DB discovery module instance. */


NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                                    /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_PERIPHERAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);

static pm_peer_id_t m_peer_to_be_deleted = PM_PEER_ID_INVALID;
static uint8_t      m_alert_message_buffer[MESSAGE_BUFFER_SIZE];                    /**< Message buffer for optional notify messages. */
static uint16_t     m_conn_handle        = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static ble_uuid_t   m_adv_uuids[] =                                                 /**< Universally unique service identifiers. */
{
    {BLE_UUID_LOCATION_AND_NAVIGATION_SERVICE, BLE_UUID_TYPE_BLE},
    {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE},
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};
static sensorsim_cfg_t   m_battery_sim_cfg;                                         /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t m_battery_sim_state;                                       /**< Battery Level sensor simulator state. */

static ble_lns_loc_speed_t   m_location_speed;                                      /**< Location and speed. */
static ble_lns_pos_quality_t m_position_quality;                                    /**< Position measurement quality. */
static ble_lns_navigation_t  m_navigation;                                          /**< Navigation data structure. */

static uint8_t m_accel_calib_status;
static uint8_t m_gyro_calib_status;
static uint8_t m_mag_calib_status;
static uint8_t m_system_calib_status;
static uint8_t m_display_screen_select;
static uint8_t m_travelling_status;
static volatile uint16_t m_screen_timeout = 0xFFFF;

static float m_ecompass_heading;

/**@brief String literals for the iOS notification categories. used then printing to UART. */
static char const *lit_catid[BLE_ANS_NB_OF_CATEGORY_ID] =
{
    "Simple alert",
    "Email",
    "News",
    "Incoming call",
    "Missed call",
    "SMS/MMS",
    "Voice mail",
    "Schedule",
    "High prioritized alert",
    "Instant message"
};

static const ble_lns_loc_speed_t initial_lns_location_speed =
{
    .instant_speed_present   = false,
    .total_distance_present  = true,
    .location_present        = true,
    .elevation_present       = true,
    .heading_present         = true,
    .rolling_time_present    = true,
    .utc_time_time_present   = true,
    .position_status         = BLE_LNS_NO_POSITION,
    .data_format             = BLE_LNS_SPEED_DISTANCE_FORMAT_2D,
    .elevation_source        = BLE_LNS_ELEV_SOURCE_BAROMETRIC,
    .heading_source          = BLE_LNS_HEADING_SOURCE_COMPASS,
    .instant_speed           = 12,         // = 1.2 meter/second
    .total_distance          = 0,          // = 2356 meters
    .latitude                = -103123567, // = -10.3123567 degrees
    .longitude               = 601234567,  // = 60.1234567 degrees
    .elevation               = 1200,       // = 12.00 meter
    .heading                 = 2123,       // = 21.23 degrees
    .rolling_time            = 0,          // = 1 second
    .utc_time                =
                               {
                                   .year    = 2020,
                                   .month   = 1,
                                   .day     = 1,
                                   .hours   = 0,
                                   .minutes = 0,
                                   .seconds = 0
                                }
};

static const ble_lns_pos_quality_t initial_lns_pos_quality =
{
    .number_of_satellites_in_solution_present = true,
    .number_of_satellites_in_view_present     = true,
    .time_to_first_fix_present                = true,
    .ehpe_present                             = false,
    .evpe_present                             = false,
    .hdop_present                             = true,
    .vdop_present                             = true,
    .position_status                          = BLE_LNS_NO_POSITION,
    .number_of_satellites_in_solution         = 5,
    .number_of_satellites_in_view             = 6,
    .time_to_first_fix                        = 63,  // = 6.3 seconds
    .hdop                                     = 123,
    .vdop                                     = 143
};

static const ble_lns_navigation_t initial_lns_navigation =
{
    .remaining_dist_present       = false,
    .remaining_vert_dist_present  = false,
    .eta_present                  = false,
    .position_status              = BLE_LNS_NO_POSITION,
    .heading_source               = BLE_LNS_HEADING_SOURCE_COMPASS,
    .navigation_indicator_type    = BLE_LNS_NAV_TO_WAYPOINT,
    .waypoint_reached             = false,
    .destination_reached          = false,
    .bearing                      = 1234,   // = 12.34 degrees
    .heading                      = 2123,   // = 21.23 degrees
    .eta                          =
                                    {
                                       .year    = 2020,
                                       .month   = 1,
                                       .day     = 1,
                                       .hours   = 0,
                                       .minutes = 0,
                                       .seconds = 0
                                    }
};


static void advertising_start(bool erase_bonds);


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    pm_handler_on_pm_evt(p_evt);
    pm_handler_disconnect_on_sec_failure(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            pm_conn_sec_status_t conn_sec_status;

            // Check if the link is authenticated (meaning at least MITM).
            err_code = pm_conn_sec_status_get(p_evt->conn_handle, &conn_sec_status);
            APP_ERROR_CHECK(err_code);

            if (conn_sec_status.mitm_protected)
            {
                NRF_LOG_INFO("Link secured. Role: %d. conn_handle: %d, Procedure: %d",
                             ble_conn_state_role(p_evt->conn_handle),
                             p_evt->conn_handle,
                             p_evt->params.conn_sec_succeeded.procedure);
            }
            else
            {
                // The peer did not use MITM, disconnect.
                NRF_LOG_INFO("Collector did not use MITM, disconnecting");
                err_code = pm_peer_id_get(m_conn_handle, &m_peer_to_be_deleted);
                APP_ERROR_CHECK(err_code);
                err_code = sd_ble_gap_disconnect(m_conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
            }

            break;
        }

        case PM_EVT_CONN_SEC_FAILED:
        {
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        }

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
            break;
        }

        default:
            break;
    }
}


/**@brief Callback function for errors in the Location Navigation Service.
 *
 * @details This function will be called in case of an error in the Location Navigation Service.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 */
void lns_error_handler(uint32_t err_code)
{
    app_error_handler(DEAD_BEEF, 0, 0);
}


/**@brief Location Navigation event handler.
 *
 * @details This function will be called for all events of the Location Navigation Module that
 *          are passed to the application.
 *
 * @param[in]   p_evt   Event received from the Location Navigation Module.
 */
static void on_lns_evt(ble_lns_t const * p_lns, ble_lns_evt_t const * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_LNS_CTRLPT_EVT_INDICATION_ENABLED:
            NRF_LOG_INFO("Control Point: Indication enabled");
            break;

        case BLE_LNS_CTRLPT_EVT_INDICATION_DISABLED:
            NRF_LOG_INFO("Control Point: Indication disabled");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("Location/Speed: Notification enabled");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("Location/Speed: Notification disabled");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("Navigation: Notification enabled");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("Navigation: Notification disabled");
            break;

        default:
            break;
    }
}


ble_lncp_rsp_code_t on_ln_ctrlpt_evt(ble_lncp_t const * p_lncp, ble_lncp_evt_t const * p_evt)
{
    switch (p_evt->evt_type)
    {
        case LNCP_EVT_MASK_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Feature mask set");
            break;

        case LNCP_EVT_TOTAL_DISTANCE_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Set total distance: %d", p_evt->params.total_distance);
            m_location_speed.total_distance = p_evt->params.total_distance;
            break;

        case LNCP_EVT_ELEVATION_SET:
            NRF_LOG_INFO("LOC_SPEED_EVT: Set elevation: %d", p_evt->params.elevation);
            break;

        case LNCP_EVT_FIX_RATE_SET:
            NRF_LOG_INFO("POS_QUAL_EVT: Fix rate set to %d", p_evt->params.fix_rate);
            break;

        case LNCP_EVT_NAV_COMMAND:
            NRF_LOG_INFO("NAV_EVT: Navigation state changed to %d", p_evt->params.nav_cmd);
            break;

        case LNCP_EVT_ROUTE_SELECTED:
            NRF_LOG_INFO("NAV_EVT: Route selected %d", p_evt->params.selected_route);
            break;


        default:
            break;
    }

    return (LNCP_RSP_SUCCESS);
}


/**@brief Function for setup of alert notifications in central.
 *
 * @details This function will be called when a successful connection has been established.
 */
static void alert_notification_setup(void)
{
    ret_code_t err_code;

    err_code = ble_ans_c_enable_notif_new_alert(&m_ans_c);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("New Alert State: Enabled.");

    err_code = ble_ans_c_enable_notif_unread_alert(&m_ans_c);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Unread Alert State: Enabled.");

    err_code = ble_ans_c_unread_alert_notify(&m_ans_c, ANS_TYPE_ALL_ALERTS);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_HANDLER(err_code);
    }
    NRF_LOG_INFO("Notify the Unread Alert characteristic for all categories.");

//    err_code = ble_ans_c_new_alert_notify(&m_ans_c, ANS_TYPE_ALL_ALERTS);
//    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
//    {
//        APP_ERROR_HANDLER(err_code);
//    }
//    NRF_LOG_INFO("Notify the New Alert characteristic for all categories.");

    NRF_LOG_DEBUG("Notifications enabled.");
}


/**@brief Function for setup of alert notifications in central.
 *
 * @details This function will be called when supported alert notification and
 *          supported unread alert notifications has been fetched.
 *
 * @param[in] p_evt  Event containing the response with supported alert types.
 */
static void control_point_setup(ble_ans_c_evt_t * p_evt)
{
    uint32_t                err_code;
    ble_ans_control_point_t setting;

    if (p_evt->uuid.uuid == BLE_UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR)
    {
        setting.command  = ANS_ENABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION;
        setting.category = (ble_ans_category_id_t)p_evt->data.alert.alert_category;
        NRF_LOG_DEBUG("Unread status notification enabled for received categories.");
    }
    else if (p_evt->uuid.uuid == BLE_UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR)
    {
        setting.command  = ANS_ENABLE_NEW_INCOMING_ALERT_NOTIFICATION;
        setting.category = (ble_ans_category_id_t)p_evt->data.alert.alert_category;
        NRF_LOG_DEBUG("New incoming notification enabled for received categories.");
    }
    else
    {
        return;
    }

    err_code = ble_ans_c_control_point_write(&m_ans_c, &setting);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for reading supported alert notifications in central.
 *
 * @details This function will be called when a connection has been established.
 */
static void supported_alert_notification_read(void)
{
    NRF_LOG_DEBUG("Read supported Alert Notification characteristics on the connected peer.");

    ret_code_t err_code;

    err_code = ble_ans_c_new_alert_read(&m_ans_c);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ans_c_unread_alert_read(&m_ans_c);
    APP_ERROR_CHECK(err_code);

}


/**@brief Function for lighting up the LED corresponding to the notification received.
 *
 * @param[in]   p_evt   Event containing the notification.
 */
static void handle_alert_notification(ble_ans_c_evt_t * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->uuid.uuid)
    {
        case BLE_UUID_UNREAD_ALERT_CHAR:
        {
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_1);
            NRF_LOG_INFO("New Unread Alert:");
            NRF_LOG_INFO("  Category:                 %s",
                         (uint32_t)lit_catid[p_evt->data.alert.alert_category]);
            NRF_LOG_INFO("  Number of unread alerts:  %d",
                         p_evt->data.alert.alert_category_count);

            oled_set_unread_noti(lit_catid[p_evt->data.alert.alert_category],
                                 p_evt->data.alert.alert_category_count);
            break;
        }

        case BLE_UUID_NEW_ALERT_CHAR:
        {
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_0);
            NRF_LOG_INFO("New Alert:");
            NRF_LOG_INFO("  Category:                 %s",
                         (uint32_t)lit_catid[p_evt->data.alert.alert_category]);
            NRF_LOG_INFO("  Number of new alerts:     %d",
                         p_evt->data.alert.alert_category_count);
            NRF_LOG_INFO("  Text String Information:  %s",
                         (uint32_t)p_evt->data.alert.p_alert_msg_buf);

            oled_set_new_noti(lit_catid[p_evt->data.alert.alert_category],
                              p_evt->data.alert.p_alert_msg_buf,
                              p_evt->data.alert.alert_category_count);
            break;
        }

        default:
        {
            // Only Unread and New Alerts exists, thus do nothing.
            break;
        }
    }

    m_display_screen_select = NOTI_0;
    m_screen_timeout = 0;
}


/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    ret_code_t err_code;
    uint8_t  battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


static void update_time(ble_date_time_t * p_time)
{
    if (NULL != p_time)
    {
        UBXGNSS_GET_DATE(GNSS_DEV, p_time->year, p_time->month, p_time->day);
        UBXGNSS_GET_TIME(GNSS_DEV, p_time->hours, p_time->minutes, p_time->seconds);
    }
}


static void navigation_update(void)
{
    m_navigation.position_status = (0 != UBXGNSS_GET_FIX(GNSS_DEV))? BLE_LNS_POSITION_OK : BLE_LNS_NO_POSITION;
    m_navigation.waypoint_reached    = !m_navigation.waypoint_reached;
    m_navigation.destination_reached = !m_navigation.destination_reached;
    m_navigation.bearing++;
  
    m_navigation.heading = m_ecompass_heading * 100;

    update_time(&m_navigation.eta);
}


static void position_quality_update(void)
{
    m_position_quality.position_status = (0 != UBXGNSS_GET_FIX(GNSS_DEV))? BLE_LNS_POSITION_OK : BLE_LNS_NO_POSITION;
    m_position_quality.number_of_satellites_in_solution = UBXGNSS_GET_SATS_IN_USE(GNSS_DEV);
    m_position_quality.number_of_satellites_in_view = UBXGNSS_GET_SATS_IN_VIEW(GNSS_DEV);
    m_position_quality.time_to_first_fix = (UBXGNSS_GET_TIME_TO_FIRST_FIX(GNSS_DEV) / 1000) * 10;
    m_position_quality.hdop = (UBXGNSS_GET_H_DILUTION(GNSS_DEV) * 10);
    m_position_quality.vdop = (UBXGNSS_GET_V_DILUTION(GNSS_DEV) * 10);
}


/**@brief Provide simulated location and speed.
 */
static void loc_speed_update(void)
{
    float elevation;

    m_location_speed.position_status = (0 != UBXGNSS_GET_FIX(GNSS_DEV))? BLE_LNS_POSITION_OK : BLE_LNS_NO_POSITION;
    m_location_speed.data_format = ((2 == UBXGNSS_GET_FIX_MODE(GNSS_DEV))? BLE_LNS_SPEED_DISTANCE_FORMAT_2D :
                                    (3 == UBXGNSS_GET_FIX_MODE(GNSS_DEV))? BLE_LNS_SPEED_DISTANCE_FORMAT_3D : BLE_LNS_SPEED_DISTANCE_FORMAT_2D);
    m_location_speed.latitude = (UBXGNSS_GET_LATITUDE(GNSS_DEV) * 10000000);
    m_location_speed.longitude = (UBXGNSS_GET_LONGITUDE(GNSS_DEV) * 10000000);

    if (BLE_LNS_ELEV_SOURCE_POSITIONING_SYSTEM == m_location_speed.elevation_source)
    {
        m_location_speed.elevation = (UBXGNSS_GET_ALTITUDE(GNSS_DEV) * 100);
    }
    else if (BLE_LNS_ELEV_SOURCE_BAROMETRIC == m_location_speed.elevation_source)
    {
        barometer_get_altitude(&elevation);
        m_location_speed.elevation = (uint32_t)(elevation * 100);
    }
//    m_location_speed.instant_speed = (UBXGNSS_GET_INSTANT_SPEED(GNSS_DEV, lwgps_speed_mph) * 10);

    /* Read from sensors */
    ecompass_read_heading(&m_ecompass_heading);

    m_location_speed.heading = m_ecompass_heading * 100;

    if (TRAVEL_STARTED == m_travelling_status)
    {
        m_location_speed.rolling_time++;
    }

    update_time(&m_location_speed.utc_time);
}


static void general_timer_handler(void)
{
    if (0xFFFF != m_screen_timeout)
    {
        m_screen_timeout++;
    }

    if ((m_screen_timeout >= SCREEN_TIMEOUT_PERIOD) && (0xFFFF != m_screen_timeout))
    {
        m_screen_timeout = 0xFFFF;
        m_display_screen_select = NAVIGATION_0;
    }
}


/**@brief Location and navigation time-out handler.
 *
 * @details This function will be called each time the location and navigation measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the time-out handler.
 */
static void lns_timeout_handler(void)
{
    ret_code_t err_code;

    loc_speed_update();
    position_quality_update();
    navigation_update();

    err_code = ble_lns_loc_speed_send(&m_lns);
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }

    err_code = ble_lns_navigation_send(&m_lns);
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
    
    ecompass_read_calibration_status(&m_accel_calib_status, &m_gyro_calib_status, &m_mag_calib_status, m_system_calib_status);
    oled_set_calib_status(m_accel_calib_status, m_gyro_calib_status, m_mag_calib_status, m_system_calib_status);
    oled_navigation_screen(&m_lns, m_display_screen_select);
}


/**@brief Function for handling the Alert Notification Service Client.
 *
 * @details This function will be called for all events in the Alert Notification Client which
 *          are passed to the application.
 *
 * @param[in]   p_evt   Event received from the Alert Notification Service Client.
 */
static void on_ans_c_evt(ble_ans_c_evt_t * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_ANS_C_EVT_NOTIFICATION:
            handle_alert_notification(p_evt);
            NRF_LOG_DEBUG("Alert Notification received from server, UUID: %X.", p_evt->uuid.uuid);
            break; // BLE_ANS_C_EVT_NOTIFICATION

        case BLE_ANS_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG_DEBUG("Alert Notification Service discovered on the server.");
            err_code = ble_ans_c_handles_assign(&m_ans_c,
                                                p_evt->conn_handle,
                                                &p_evt->data.service);
            APP_ERROR_CHECK(err_code);
            supported_alert_notification_read();
            alert_notification_setup();
            break; // BLE_ANS_C_EVT_DISCOVERY_COMPLETE

        case BLE_ANS_C_EVT_READ_RESP:
            NRF_LOG_DEBUG("Alert Setup received from server, UUID: %X.", p_evt->uuid.uuid);
            control_point_setup(p_evt);
            break; // BLE_ANS_C_EVT_READ_RESP

        case BLE_ANS_C_EVT_DISCONN_COMPLETE:
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            APP_ERROR_CHECK(err_code);
            break; // BLE_ANS_C_EVT_DISCONN_COMPLETE

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_DISP);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Alert Notification Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void alert_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Glucose, Battery and Device Information services.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    ble_lns_init_t     lns_init;
    ble_dis_init_t     dis_init;
    ble_bas_init_t     bas_init;
    ble_ans_c_init_t   ans_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Location and Navigation Service.
    memset(&lns_init, 0, sizeof(lns_init));

    lns_init.evt_handler      = on_lns_evt;
    lns_init.lncp_evt_handler = on_ln_ctrlpt_evt;
    lns_init.error_handler    = lns_error_handler;
    lns_init.p_gatt_queue     = &m_ble_gatt_queue;

    lns_init.is_position_quality_present = true;
    lns_init.is_control_point_present    = true;
    lns_init.is_navigation_present       = true;

    lns_init.available_features     = BLE_LNS_FEATURE_INSTANT_SPEED_SUPPORTED                 |
                                      BLE_LNS_FEATURE_TOTAL_DISTANCE_SUPPORTED                |
                                      BLE_LNS_FEATURE_LOCATION_SUPPORTED                      |
                                      BLE_LNS_FEATURE_ELEVATION_SUPPORTED                     |
                                      BLE_LNS_FEATURE_HEADING_SUPPORTED                       |
                                      BLE_LNS_FEATURE_ROLLING_TIME_SUPPORTED                  |
                                      BLE_LNS_FEATURE_UTC_TIME_SUPPORTED                      |
                                      BLE_LNS_FEATURE_REMAINING_DISTANCE_SUPPORTED            |
                                      BLE_LNS_FEATURE_REMAINING_VERT_DISTANCE_SUPPORTED       |
                                      BLE_LNS_FEATURE_EST_TIME_OF_ARRIVAL_SUPPORTED           |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_SOLUTION_SUPPORTED          |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_VIEW_SUPPORTED              |
                                      BLE_LNS_FEATURE_TIME_TO_FIRST_FIX_SUPPORTED             |
                                      BLE_LNS_FEATURE_HORZ_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_VERT_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_LOC_AND_SPEED_CONTENT_MASKING_SUPPORTED |
                                      BLE_LNS_FEATURE_FIX_RATE_SETTING_SUPPORTED              |
                                      BLE_LNS_FEATURE_ELEVATION_SETTING_SUPPORTED             |
                                      BLE_LNS_FEATURE_POSITION_STATUS_SUPPORTED;


    m_location_speed   = initial_lns_location_speed;
    m_position_quality = initial_lns_pos_quality;
    m_navigation       = initial_lns_navigation;

    lns_init.p_location_speed   = &m_location_speed;
    lns_init.p_position_quality = &m_position_quality;
    lns_init.p_navigation       = &m_navigation;

    lns_init.loc_nav_feature_security_req_read_perm  = SEC_OPEN;
    lns_init.loc_speed_security_req_cccd_write_perm  = SEC_OPEN;
    lns_init.position_quality_security_req_read_perm = SEC_OPEN;
    lns_init.navigation_security_req_cccd_write_perm = SEC_OPEN;
    lns_init.ctrl_point_security_req_write_perm      = SEC_OPEN;
    lns_init.ctrl_point_security_req_cccd_write_perm = SEC_OPEN;

    err_code = ble_lns_init(&m_lns, &lns_init);
    APP_ERROR_CHECK(err_code);

    ble_lns_route_t route1 = {.route_name = "Route one"};
    err_code = ble_lns_add_route(&m_lns, &route1);
    APP_ERROR_CHECK(err_code);

    ble_lns_route_t route2 = {.route_name = "Route two"};
    err_code = ble_lns_add_route(&m_lns, &route2);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec        = SEC_OPEN;
    bas_init.bl_cccd_wr_sec   = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);

    ble_srv_ascii_to_utf8(&dis_init.serial_num_str, MODEL_NUMBER);

    ble_dis_sys_id_t system_id;
    system_id.manufacturer_id            = MANUFACTURER_ID;
    system_id.organizationally_unique_id = ORG_UNIQUE_ID;
    dis_init.p_sys_id                    = &system_id;

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    // Initialize the Alert Notification Service Client.
    memset(&ans_init, 0, sizeof(ans_init));
    memset(m_alert_message_buffer, 0, MESSAGE_BUFFER_SIZE);

    ans_init.evt_handler         = on_ans_c_evt;
    ans_init.message_buffer_size = MESSAGE_BUFFER_SIZE;
    ans_init.p_message_buffer    = m_alert_message_buffer;
    ans_init.error_handler       = alert_notification_error_handler;
    ans_init.p_gatt_queue        = &m_ble_gatt_queue;

    err_code =  ble_ans_c_init(&m_ans_c, &ans_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the sensor simulators.
 */
static void sensor_simulator_init(void)
{
    m_battery_sim_cfg.min          = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max          = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr         = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ans_c_on_db_disc_evt(&m_ans_c, p_evt);
}


/** @brief Database discovery module initialization.
 */
static void db_discovery_init(void)
{
    ret_code_t err_code;
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(ble_db_discovery_init_t));

    db_init.evt_handler  = db_disc_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue;

    err_code = ble_db_discovery_init(&db_init);

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameter events.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail configuration parameter, but instead we use the
 *                event handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAM_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;;
    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
//    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED_HIGH_DUTY:
        {
            NRF_LOG_INFO("Directed advertising");
            APP_ERROR_CHECK(bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED));
            break;
        }

        case BLE_ADV_EVT_FAST:
        {
            NRF_LOG_INFO("Fast advertising");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break; // BLE_ADV_EVT_FAST
        }

        case BLE_ADV_EVT_SLOW:
        {
            NRF_LOG_INFO("Slow advertising");
            APP_ERROR_CHECK(bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW));
            break;
        }

        case BLE_ADV_EVT_IDLE:
        {
            sleep_mode_enter();
            break; // BLE_ADV_EVT_IDLE
        }

        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;

    pm_handler_secure_on_connection(p_ble_evt);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
        {
            NRF_LOG_INFO("Disconnected");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            // Check if the last connected peer had not used MITM, if so, delete its bond information.
            if (m_peer_to_be_deleted != PM_PEER_ID_INVALID)
            {
                err_code = pm_peer_delete(m_peer_to_be_deleted);
                APP_ERROR_CHECK(err_code);
                NRF_LOG_DEBUG("Collector's bond deleted");
                m_peer_to_be_deleted = PM_PEER_ID_INVALID;
            }
            break;
        }

        case BLE_GAP_EVT_CONNECTED:
        {
            NRF_LOG_INFO("Connected");
            m_peer_to_be_deleted = PM_PEER_ID_INVALID;
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = ble_db_discovery_start(&m_ble_db_discovery, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            // Start Security Request timer.
            break;
        }

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
            break;
        }

        case BLE_GATTC_EVT_TIMEOUT:
        {
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
        }

        case BLE_GATTS_EVT_TIMEOUT:
        {
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
        }

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        {
            NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            break;
        }

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
        {
            char passkey[PASSKEY_LENGTH + 1];
            memcpy(passkey, p_ble_evt->evt.gap_evt.params.passkey_display.passkey, PASSKEY_LENGTH);
            passkey[PASSKEY_LENGTH] = 0;

            NRF_LOG_INFO("Passkey: %s", nrf_log_push(passkey));
            break;
        }

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
        {
            NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;
        }

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
        {
            NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
            break;
        }

        case BLE_GAP_EVT_AUTH_STATUS:
        {
            NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status = 0x%x bond = 0x%x lv4: %d kdist_own: 0x%x kdist_peer: 0x%x",
                         p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                         p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                         p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                         *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                         *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
            break;
        }

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
        {
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP
        }

        case BSP_EVENT_DISCONNECT:
        {
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT
        }

        case BSP_EVENT_WHITELIST_OFF:
        {
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_WHITELIST_OFF
        }

        case BSP_EVENT_KEY_0:
        {
            break;
        }

        case BSP_EVENT_KEY_1:
        {
            m_display_screen_select = NOTI_0;
            break;
        }

        case BSP_EVENT_KEY_2:
        {
            if (TRAVEL_STARTED == m_travelling_status)
            {
                m_location_speed.rolling_time = 0;
                m_travelling_status = TRAVEL_READY;
            }
            else
            {
                m_travelling_status = TRAVEL_STARTED;
            }
            break;
        }

        case BSP_EVENT_KEY_3:
        {
            m_display_screen_select = (m_display_screen_select + 1) % ALL_SCREEN;
            break;
        }

        default:
            break;
    }
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_lesc_request_handler();
    APP_ERROR_CHECK(err_code);

    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    log_init();
    peripherals_init();
    buttons_leds_init(&erase_bonds);

    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    db_discovery_init();
    services_init();
    sensor_simulator_init();
    conn_params_init();
    peer_manager_init();

    gnss_init();
    barometer_init();
    ecompass_init();
    oled_init();

    peripherals_assign_comm_handle(TIMER_BATTERY, battery_level_update);
    peripherals_assign_comm_handle(TIMER_LNS, lns_timeout_handler);
    peripherals_assign_comm_handle(TIMER_GENERAL, general_timer_handler);

    // Start execution.
    NRF_LOG_INFO("Positioning example started.");
    peripherals_start_timers();
    advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
        comm_handle_polling();
        idle_state_handle();
    }
}


/**
 * @}
 */
