#ifndef _BLUETOOTH_H__
#define _BLUETOOTH_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_DEV_NAME_LEN 249
#define MAX_SERVICE_NUM 16

/** Bluetooth Address */
typedef struct {
	uint8_t address[6];
} __attribute__((packed))bt_bdaddr_t;

/** Bluetooth Device Name */
typedef struct {
	uint8_t name[MAX_DEV_NAME_LEN];
} __attribute__((packed))bt_bdname_t;

/** Bluetooth Adapter Visibility Modes*/
typedef enum {
    BT_SCAN_MODE_NONE,
    BT_SCAN_MODE_CONNECTABLE,
    BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE
} bt_scan_mode_t;

/** Bluetooth Adapter State */
typedef enum {
    BT_STATE_OFF,
    BT_STATE_ON,
	BT_STATE_OFF_ERR = 0xf
} bt_state_t;

/** Bluetooth Error Status */
/** We need to build on this */
typedef enum {
    BT_STATUS_SUCCESS,
    BT_STATUS_FAIL,
    BT_STATUS_NOT_READY,
    BT_STATUS_NOMEM,
    BT_STATUS_BUSY,
    BT_STATUS_DONE,        /* request already completed */
    BT_STATUS_UNSUPPORTED,
    BT_STATUS_PARM_INVALID,
    BT_STATUS_UNHANDLED,
    BT_STATUS_AUTH_FAILURE,
    BT_STATUS_RMT_DEV_DOWN,
    BT_STATUS_AUTH_REJECTED,
    BT_STATUS_JNI_ENVIRONMENT_ERROR,
    BT_STATUS_JNI_THREAD_ATTACH_ERROR,
    BT_STATUS_WAKELOCK_ERROR
} bt_status_t;

/** Bluetooth PinKey Code */
typedef struct {
	uint8_t pin[16];
} __attribute__((packed))bt_pin_code_t;

/** Bluetooth Adapter Discovery state */
typedef enum {
    BT_DISCOVERY_STOPPED,
    BT_DISCOVERY_STARTED
} bt_discovery_state_t;

/** Bluetooth ACL connection state */
typedef enum {
    BT_ACL_STATE_CONNECTED,
    BT_ACL_STATE_DISCONNECTED
} bt_acl_state_t;

/** Bluetooth 128-bit UUID */
typedef struct {
	uint8_t uu[16];
} bt_uuid_t;

/** Bluetooth SDP service record */
typedef struct {
	bt_uuid_t uuid;
	uint16_t channel;
	char name[256]; // what's the maximum length
} bt_service_record_t;

/** Bluetooth Remote Version info */
typedef struct {
	int version;
	int sub_ver;
	int manufacturer;
} bt_remote_version_t;

/**
 * @brief 远端蓝牙设备结构体 */
typedef struct {
    bt_bdaddr_t address;
    bt_bdname_t name;
    uint32_t class_of_device;
    uint8_t num_service;
    uint16_t services[MAX_SERVICE_NUM];
} bt_remote_device_t;

/**
 * @brief 远端蓝牙设备的制造商ID,*/
typedef enum {
	BT_MANUFACTURER_CODE_UNKNOWN,
	BT_MANUFACTURER_CODE_APPLE,
	BT_MANUFACTURER_CODE_OTHER,
} bt_manufacturer_code_t;

/* Bluetooth Adapter and Remote Device property types */
typedef enum {
    /* Properties common to both adapter and remote device */
    /**
     * Description - Bluetooth Device Name
     * Access mode - Adapter name can be GET/SET. Remote device can be GET
     * Data type   - bt_bdname_t
     */
    BT_PROPERTY_BDNAME = 0x1,
    /**
     * Description - Bluetooth Device Address
     * Access mode - Only GET.
     * Data type   - bt_bdaddr_t
     */
    BT_PROPERTY_BDADDR,
    /**
     * Description - Bluetooth Service 128-bit UUIDs
     * Access mode - Only GET.
     * Data type   - Array of bt_uuid_t (Array size inferred from property length).
     */
    BT_PROPERTY_UUIDS,
    /**
     * Description - Bluetooth Class of Device as found in Assigned Numbers
     * Access mode - Only GET.
     * Data type   - uint32_t.
     */
    BT_PROPERTY_CLASS_OF_DEVICE,
    /**
     * Description - Device Type - BREDR, BLE or DUAL Mode
     * Access mode - Only GET.
     * Data type   - bt_device_type_t
     */
    BT_PROPERTY_TYPE_OF_DEVICE,
    /**
     * Description - Bluetooth Service Record
     * Access mode - Only GET.
     * Data type   - bt_service_record_t
     */
    BT_PROPERTY_SERVICE_RECORD,

    /* Properties unique to adapter */
    /**
     * Description - Bluetooth Adapter scan mode
     * Access mode - GET and SET
     * Data type   - bt_scan_mode_t.
     */
    BT_PROPERTY_ADAPTER_SCAN_MODE,
    /**
     * Description - List of bonded devices
     * Access mode - Only GET.
     * Data type   - Array of bt_bdaddr_t of the bonded remote devices
     *               (Array size inferred from property length).
     */
    BT_PROPERTY_ADAPTER_BONDED_DEVICES,
    /**
     * Description - Bluetooth Adapter Discovery timeout (in seconds)
     * Access mode - GET and SET
     * Data type   - uint32_t
     */
    BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,

    /* Properties unique to remote device */
    /**
     * Description - User defined friendly name of the remote device
     * Access mode - GET and SET
     * Data type   - bt_bdname_t.
     */
    BT_PROPERTY_REMOTE_FRIENDLY_NAME,
    /**
     * Description - RSSI value of the inquired remote device
     * Access mode - Only GET.
     * Data type   - int32_t.
     */
    BT_PROPERTY_REMOTE_RSSI,
    /**
     * Description - Remote version info
     * Access mode - SET/GET.
     * Data type   - bt_remote_version_t.
     */

    BT_PROPERTY_REMOTE_VERSION_INFO,

    /**
     * Description - Local LE features
     * Access mode - GET.
     * Data type   - bt_local_le_features_t.
     */
    BT_PROPERTY_LOCAL_LE_FEATURES,

    /**
     * Description - Default pin code
     * Access mode - SET/GET
     * Data type   - char *
     */
    BT_PROPERTY_DEFAULT_PINCODE,

	/**
	 * Description - Get bond devices
     * Access mode - Only GET
     * Data type   - bt_remote_device_t
	 */
	BT_PROPERTY_ADAPTER_SAVED_DEVICES, /*0x0F*/

    /**
     * Description - The priority of the remote device
     * Access mode - GET
     * Data type   - uint8_t
     */
    BT_PROPERTY_REMOTE_PRIORITY = 0x20,

	/**
	 * Description - The manufacturer code of the remote device.
	 * Access mode - Get
	 * Data type   - bt_manufacturer_code_t
	 */
    BT_PROPERTY_MANUFACTURER_CODE,

	/**
	 * Description - Get the software version
	 * Access mode - Get
	 * Data type   - char *
	 */
    BT_PROPERTY_SW_VERSION,

	/**
	 * Description - The max number of bonded devices which will be saved.
	 * Access mode - Set/Get
	 * Data type   - int
	 */
	BT_PROPERTY_BONDED_DEVICE_NUM,

	/**
	 * Description - The max duration of device discovery procedure.
	 * Access mode - Set/Get
	 * Data type   - int
	 */
	BT_PROPERTY_ADAPTER_DISCOVERY_INTERVAL,
    BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP = 0xFF,
} bt_property_type_t;

/** Bluetooth Adapter Property data structure */
typedef struct {
	bt_property_type_t type;
	int len;
	void *val;
} bt_property_t;

/** Bluetooth Device Type */
typedef enum {
    BT_DEVICE_DEVTYPE_BREDR = 0x1,
    BT_DEVICE_DEVTYPE_BLE,
    BT_DEVICE_DEVTYPE_DUAL
} bt_device_type_t;
/** Bluetooth Bond state */
typedef enum {
    BT_BOND_STATE_NONE,
    BT_BOND_STATE_BONDING,
    BT_BOND_STATE_BONDED
} bt_bond_state_t;

/** Bluetooth SSP Bonding Variant */
typedef enum {
    BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
    BT_SSP_VARIANT_PASSKEY_ENTRY,
    BT_SSP_VARIANT_CONSENT,
    BT_SSP_VARIANT_PASSKEY_NOTIFICATION
} bt_ssp_variant_t;

#define BT_MAX_NUM_UUIDS 32

#ifdef __cplusplus
}
#endif

#endif /* _BLUETOOTH_H__ */

