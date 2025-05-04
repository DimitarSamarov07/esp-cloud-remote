#include <cassert>
#include <cstdio>
#include <cstring>
#include <host/util/util.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

// Globals
static const char *tag = "ESP_Cloud_Remote_BLE";
uint16_t conn_handle;
static uint8_t own_addr_type;
char characteristic_value[50] = "I am characteristic value"; // When a client reads a characteristic, he will get this value.
char characteristic_received_value[500];                     // When client writes to a characteristic , he will set the value of this.

uint16_t min_length = 1;   // Maximum characteristic length
uint16_t max_length = 700; // Minimum characteristic length


// Misc funcs
void print_addr(const uint8_t *addr)
{
    const uint8_t* u8p = addr;
    MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}


// Scan service
// --------

// 31175f53-389f-4d19-8f69-6da32a28d50e
static constexpr ble_uuid128_t ble_scan_service_uuid = BLE_UUID128_INIT(0x0e, 0xd5, 0x28, 0x2a, 0xa3, 0x6d, 0x69, 0x8f,
                                                                    0x19, 0x4d, 0x9f, 0x38, 0x53, 0x5f, 0x17, 0x31);

// 22946000-e86b-4831-adc8-27ab2cdb85fb
static constexpr ble_uuid128_t ble_wifi_ssid_list_chr_uuid = BLE_UUID128_INIT(
    0xfb, 0x85, 0xdb, 0x2c, 0xab, 0x27, 0xc8, 0xad, 0x31, 0x48, 0x6b, 0xe8, 0x00, 0x60, 0x94, 0x22
);

// 3a6b1432-c481-45a7-a798-45ecd7d1ef41
// Empty is considered False, while any data is considered True
static constexpr ble_uuid128_t ble_should_scan_chr_uuid = BLE_UUID128_INIT(
    0x41, 0xef, 0xd1, 0xd7, 0xec, 0x45, 0x98, 0xa7, 0xa7, 0x45, 0x81, 0xc4, 0x32, 0x14, 0x6b, 0x3a
);
// ----------
// Set service
//--------------

//4fafc201-1fb5-459e-8fcc-c5c9c331914b
static constexpr ble_uuid128_t ble_set_service_uuid = BLE_UUID128_INIT(0x4b, 0x91, 0x31, 0xc3, 0xc9, 0xc5, 0xcc, 0x8f, 0x9e,
                                                                   0x45, 0xb5, 0x1f, 0x01, 0xc2, 0xaf, 0x4f);
// beb5483e-36e1-4688-b7f5-ea07361b26a8
static constexpr ble_uuid128_t ble_wifi_ssid_set_chr_uuid = BLE_UUID128_INIT(
    0xa8, 0x26, 0x1b, 0x36, 0x07, 0xea, 0xf5, 0xb7, 0x88, 0x46, 0xe1, 0x36, 0x3e, 0x48, 0xb5, 0xbe
);

// ed9e4092-2c12-4a94-9d86-696058d16573
static constexpr ble_uuid128_t ble_wifi_pass_set_chr_uuid = BLE_UUID128_INIT(
    0x73, 0x65, 0xd1, 0x58, 0x60, 0x69, 0x86, 0x9d, 0x94, 0x4a, 0x12, 0x2c, 0x92, 0x40, 0x9e, 0xed
);

// 4d5ea626-4ac9-4a71-9392-d0764ac7cefe
static constexpr ble_uuid128_t ble_wifi_connection_status_chr_uuid = BLE_UUID128_INIT(
    0xfe, 0xce, 0xc7, 0x4a, 0x76, 0xd0, 0x92, 0x93, 0x71, 0x4a, 0xc9, 0x4a, 0x26, 0xa6, 0x5e, 0x4d
);

//---------------------------------------------------
// Declare some functions and corresponding variables
//---------------------------------------------------


// SCAN SERVICE

static int ssid_list_acc(uint16_t conn_handle, uint16_t attr_handle,
                         const struct ble_gatt_access_ctxt* ctxt,
                         void* arg);
char ssid_list_val_chr_value[1000] = "";
uint16_t ssid_list_noti_hnd;
uint8_t ssid_list_noti_state;


static int should_scan_bool_acc(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt* ctxt,
                                void* arg);
bool should_scan_chr_value = false;


// SET SERVICE

static int set_ssid_acc(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt* ctxt,
                        void* arg);
char set_ssid_chr_value[32] = "";


static int set_pass_acc(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt* ctxt,
                        void* arg);
char set_pass_chr_value[32] = "";


static int wifi_connection_status_acc(uint16_t conn_handle, uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt* ctxt,
                                      void* arg);
uint16_t wifi_connection_status_noti_hnd;
uint8_t wifi_connection_status_noti_state;

int wifi_connection_status_chr_value = 0;
// 0 - waiting for ssid and password to be set
// 1 - Wi-Fi connected
// 2 - Connection failed, recheck data
// 3 - Unknown error


// ----------
// Declare services and their characteristics
//--------------------

static const struct ble_gatt_svc_def gatt_server_services[] = {
    {

        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &ble_scan_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &ble_wifi_ssid_list_chr_uuid.u,
                .access_cb = ssid_list_acc,
                // Callback -> Whenever this characteristic is accessed by user, this function will execute
                .val_handle = &ssid_list_noti_hnd,
                // Notification handle id is stored here to be used later
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY, // User permissions
            },
            {
                .uuid = &ble_should_scan_chr_uuid.u,
                .access_cb = should_scan_bool_acc,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                nullptr
            }
        },
    },
    {

        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &ble_set_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &ble_wifi_ssid_set_chr_uuid.u,
                .access_cb = set_ssid_acc,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                .uuid = &ble_wifi_pass_set_chr_uuid.u,
                .access_cb = set_pass_acc,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
            },
            {
                .uuid = &ble_wifi_connection_status_chr_uuid.u,
                .access_cb = wifi_connection_status_acc,
                .val_handle = &wifi_connection_status_noti_hnd,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            {
                nullptr
            }
        },
    },
    {
        0
    }
};



void ble_init()
{
}


void startNVS() // Non-volatile storage
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_flash_init();
    }
}

// BLE things:
static int gatt_svr_chr_write(struct os_mbuf* om, uint16_t min_len, uint16_t max_len, void* dst, uint16_t* len)
{
    uint16_t om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
    {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    int rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt* ctxt, void* arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
    }
}

int gatt_svr_init()
{
    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(gatt_server_services);
    if (rc != 0)
    {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_server_services);
    if (rc != 0)
    {
        return rc;
    }

    return 0;
}

static void
bleprph_print_conn_desc(struct ble_gap_conn_desc* desc)
{
    MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

static void
bleprph_advertise()
{
    ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields{};
    const char* name;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
        BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t*)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]){
        BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, nullptr, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, nullptr);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 */
static int bleprph_gap_event(struct ble_gap_event* event, void* arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
        }
        MODLOG_DFLT(INFO, "\n");

        if (event->connect.status != 0)
        {
            /* Connection failed; resume advertising. */
            bleprph_advertise();
        }
        conn_handle = event->connect.conn_handle;
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        bleprph_print_conn_desc(&event->disconnect.conn);
        MODLOG_DFLT(INFO, "\n");

    /* Connection terminated; resume advertising. */
        bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        MODLOG_DFLT(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        MODLOG_DFLT(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                    event->adv_complete.reason);
        bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        uint16_t curr_noti_handle;
        uint8_t notify_state;
        if (event->subscribe.attr_handle == wifi_connection_status_noti_hnd)
        {
            curr_noti_handle = wifi_connection_status_noti_hnd;
            notify_state = wifi_connection_status_noti_state;
        }
        else if (event->subscribe.attr_handle == ssid_list_noti_hnd)
        {
            notify_state = ssid_list_noti_state;
            curr_noti_handle = ssid_list_noti_hnd;
        }
        MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; "
                    "val_handle=%d\n"
                    "conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.cur_notify, curr_noti_handle, //!! Client Subscribed to handle
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);


        printf("\nSubscribed with notification_handle =%d\n", event->subscribe.attr_handle);
        notify_state = event->subscribe.cur_notify;
    //!! As the client is now subscribed to notifications, the value is set to 1
        printf("notify_state=%d\n", notify_state);

        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        return 0;
    }

    return 0;
}

static void
bleprph_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}


static void
bleprph_on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");
    /* Begin advertising. */
    bleprph_advertise();
}

void bleprph_host_task(void* param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}


//--------------------------------------
// Declare previously defined functions
//--------------------------------------

static int ssid_list_acc(uint16_t conn_handle, uint16_t attr_handle,
                         const struct ble_gatt_access_ctxt* ctxt,
                         void* arg)
{
    int rc;

    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        //!! In case user accessed this characterstic to read its value, bellow lines will execute
            rc = os_mbuf_append(ctxt->om, &ssid_list_val_chr_value,
                                sizeof ssid_list_val_chr_value);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        //!! In case user accessed this characterstic to write, bellow lines will executed.
            rc = gatt_svr_chr_write(ctxt->om, min_length, max_length, &characteristic_received_value, nullptr);
        //!! Function "gatt_svr_chr_write" will fire.
        printf("Received=%s\n", characteristic_received_value); // Print the received value
        return rc;
    default:
        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }
}