#ifndef MAP_MSE_H
#define MAP_MSE_H

#include <stddef.h>
#include <stdint.h>

#include "map_mns.h"
#include "map_sdp.h"
#include "msg_store.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_MSE_DEFAULT_MAS_INSTANCE_ID  0
#define MAP_MSE_DEFAULT_RFCOMM_CHANNEL  18
#define MAP_MSE_DEFAULT_L2CAP_PSM     0x1025

#define MAP_MSE_MSG_TYPE_SMS_GSM      0x02
#define MAP_MSE_FEATURE_NOTIFICATION_REGISTRATION  0x00000001u
#define MAP_MSE_FEATURE_NOTIFICATION                 0x00000002u
#define MAP_MSE_FEATURE_BROWSING                     0x00000004u
#define MAP_MSE_FEATURE_UPLOAD                       0x00000008u
#define MAP_MSE_FEATURE_DELETE                       0x00000010u
#define MAP_MSE_FEATURE_INSTANCE_INFORMATION         0x00000020u

typedef struct map_mse_state_s {
    msg_store_t *store;
    map_mns_state_t *mns;
    map_mas_sdp_config_t sdp_config;
    char sdp_xml[MAP_SDP_XML_MAX_LEN];
    uint8_t started;
    uint8_t mas_advertised;
    uint8_t reserved0;
    uint8_t reserved1;
} map_mse_state_t;

void map_mse_init(map_mse_state_t *state, msg_store_t *store, map_mns_state_t *mns);
int map_mse_start(map_mse_state_t *state);
int map_mse_stop(map_mse_state_t *state);
int map_mse_rebuild_sdp(map_mse_state_t *state);
const char *map_mse_get_sdp_xml(const map_mse_state_t *state);
int map_mse_set_notification_registration(map_mse_state_t *state, uint8_t enabled);
int map_mse_msg_send(map_mse_state_t *state,
                     const char *remote_addr,
                     const char *text,
                     uint8_t transparent,
                     msg_record_t *out_record);
size_t map_mse_msg_list(map_mse_state_t *state, msg_record_t *out_records, size_t max_records);
int map_mse_msg_clear(map_mse_state_t *state);
int map_mse_on_delivery_result(map_mse_state_t *state, uint32_t id, uint8_t delivered);
int map_mse_on_incoming_message(map_mse_state_t *state,
                                const char *remote_addr,
                                const char *text,
                                msg_record_t *out_record);

#ifdef __cplusplus
}
#endif

#endif
