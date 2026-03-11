#include "map_mns.h"

#include <stdio.h>
#include <string.h>

static const char *map_mns_event_name(map_event_type_e type)
{
    switch (type) {
    case MAP_EVENT_NEW_MESSAGE:
        return "NewMessage";
    case MAP_EVENT_DELIVERY_SUCCESS:
        return "DeliverySuccess";
    case MAP_EVENT_SENDING_SUCCESS:
        return "SendingSuccess";
    case MAP_EVENT_SENDING_FAILURE:
        return "SendingFailure";
    case MAP_EVENT_MESSAGE_DELETED:
        return "MessageDeleted";
    case MAP_EVENT_MESSAGE_SHIFT:
        return "MessageShift";
    case MAP_EVENT_MEMORY_FULL:
        return "MemoryFull";
    case MAP_EVENT_MEMORY_AVAILABLE:
        return "MemoryAvailable";
    case MAP_EVENT_READ_STATUS_CHANGED:
        return "ReadStatusChanged";
    default:
        return "Unknown";
    }
}

void map_mns_init(map_mns_state_t *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
}

void map_mns_reset(map_mns_state_t *state)
{
    if (!state) {
        return;
    }

    state->queue.head = 0;
    state->queue.tail = 0;
    state->queue.count = 0;
}

int map_mns_set_registration(map_mns_state_t *state, uint8_t registered)
{
    if (!state) {
        return -1;
    }

    state->registered = registered ? 1 : 0;
    if (!state->registered) {
        state->remote_notifications_enabled = 0;
        map_mns_reset(state);
    }
    return 0;
}

int map_mns_set_remote_notification_status(map_mns_state_t *state, uint8_t enabled)
{
    if (!state || !state->registered) {
        return -1;
    }

    state->remote_notifications_enabled = enabled ? 1 : 0;
    return 0;
}

int map_mns_queue_event(map_mns_state_t *state, const map_event_t *event_data)
{
    map_mns_queue_t *queue;

    if (!state || !event_data) {
        return -1;
    }

    if (!state->registered || !state->remote_notifications_enabled) {
        return -2;
    }

    queue = &state->queue;
    if (queue->count >= MAP_MNS_MAX_EVENTS) {
        return -3;
    }

    queue->events[queue->tail] = *event_data;
    queue->tail = (queue->tail + 1u) % MAP_MNS_MAX_EVENTS;
    ++queue->count;
    return 0;
}

int map_mns_pop_event(map_mns_state_t *state, map_event_t *event_data)
{
    map_mns_queue_t *queue;

    if (!state || !event_data) {
        return -1;
    }

    queue = &state->queue;
    if (!queue->count) {
        return -2;
    }

    *event_data = queue->events[queue->head];
    queue->head = (queue->head + 1u) % MAP_MNS_MAX_EVENTS;
    --queue->count;
    return 0;
}

int map_mns_build_event_report_xml(const map_event_t *event_data,
                                   char *xml_buffer,
                                   size_t xml_buffer_size)
{
    const char *folder;
    const char *old_folder;
    const char *msg_type;
    const char *event_name;
    int written;

    if (!event_data || !xml_buffer || xml_buffer_size < 64) {
        return -1;
    }

    folder = event_data->folder[0] ? event_data->folder : "telecom/msg/outbox";
    old_folder = event_data->old_folder[0] ? event_data->old_folder : "";
    msg_type = event_data->msg_type[0] ? event_data->msg_type : "SMS_GSM";
    event_name = map_mns_event_name(event_data->type);

    written = snprintf(
        xml_buffer,
        xml_buffer_size,
        "<?xml version=\"1.0\"?>\n"
        "<MAP-event-report version=\"1.0\">\n"
        "  <event type=\"%s\" handle=\"%s\" folder=\"%s\" old_folder=\"%s\" msg_type=\"%s\" value=\"%u\"/>\n"
        "</MAP-event-report>\n",
        event_name,
        event_data->handle,
        folder,
        old_folder,
        msg_type,
        (unsigned int)event_data->value);

    if (written < 0 || (size_t)written >= xml_buffer_size) {
        if (xml_buffer_size) {
            xml_buffer[0] = '\0';
        }
        return -2;
    }

    return written;
}
