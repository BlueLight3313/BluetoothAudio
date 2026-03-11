#ifndef MAP_MNS_H
#define MAP_MNS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_MNS_MAX_EVENTS            64
#define MAP_MNS_EVENT_XML_MAX_LEN    512

typedef enum {
    MAP_EVENT_NEW_MESSAGE = 0,
    MAP_EVENT_DELIVERY_SUCCESS = 1,
    MAP_EVENT_SENDING_SUCCESS = 2,
    MAP_EVENT_SENDING_FAILURE = 3,
    MAP_EVENT_MESSAGE_DELETED = 4,
    MAP_EVENT_MESSAGE_SHIFT = 5,
    MAP_EVENT_MEMORY_FULL = 6,
    MAP_EVENT_MEMORY_AVAILABLE = 7,
    MAP_EVENT_READ_STATUS_CHANGED = 8
} map_event_type_e;

typedef struct map_event_s {
    map_event_type_e type;
    char handle[32];
    char folder[64];
    char old_folder[64];
    char msg_type[16];
    uint8_t value;
} map_event_t;

typedef struct map_mns_queue_s {
    map_event_t events[MAP_MNS_MAX_EVENTS];
    size_t head;
    size_t tail;
    size_t count;
} map_mns_queue_t;

typedef struct map_mns_state_s {
    uint8_t registered;
    uint8_t remote_notifications_enabled;
    uint8_t reserved0;
    uint8_t reserved1;
    uint8_t rfcomm_channel;
    uint16_t l2cap_psm;
    map_mns_queue_t queue;
} map_mns_state_t;

void map_mns_init(map_mns_state_t *state);
void map_mns_reset(map_mns_state_t *state);
int map_mns_set_registration(map_mns_state_t *state, uint8_t registered);
int map_mns_set_remote_notification_status(map_mns_state_t *state, uint8_t enabled);
int map_mns_queue_event(map_mns_state_t *state, const map_event_t *event_data);
int map_mns_pop_event(map_mns_state_t *state, map_event_t *event_data);
int map_mns_build_event_report_xml(const map_event_t *event_data,
                                   char *xml_buffer,
                                   size_t xml_buffer_size);

#ifdef __cplusplus
}
#endif

#endif
