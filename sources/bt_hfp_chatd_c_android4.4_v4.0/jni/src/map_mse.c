#include "map_mse.h"

#include <string.h>

static void map_mse_queue_status_event(map_mse_state_t *state,
                                       map_event_type_e event_type,
                                       const msg_record_t *record,
                                       uint8_t value)
{
    map_event_t event_data;

    if (!state || !state->mns || !record) {
        return;
    }

    memset(&event_data, 0, sizeof(event_data));
    event_data.type = event_type;
    event_data.value = value;
    memcpy(event_data.handle, record->handle, sizeof(event_data.handle) - 1);
    memcpy(event_data.folder, "telecom/msg/outbox", sizeof("telecom/msg/outbox"));
    memcpy(event_data.msg_type, "SMS_GSM", sizeof("SMS_GSM"));

    map_mns_queue_event(state->mns, &event_data);
}

void map_mse_init(map_mse_state_t *state, msg_store_t *store, map_mns_state_t *mns)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->store = store;
    state->mns = mns;
    state->sdp_config.mas_instance_id = MAP_MSE_DEFAULT_MAS_INSTANCE_ID;
    state->sdp_config.rfcomm_channel = MAP_MSE_DEFAULT_RFCOMM_CHANNEL;
    state->sdp_config.l2cap_psm = MAP_MSE_DEFAULT_L2CAP_PSM;
    state->sdp_config.supported_message_types = MAP_MSE_MSG_TYPE_SMS_GSM;
    state->sdp_config.supported_features = MAP_MSE_FEATURE_NOTIFICATION_REGISTRATION |
                                           MAP_MSE_FEATURE_NOTIFICATION |
                                           MAP_MSE_FEATURE_BROWSING |
                                           MAP_MSE_FEATURE_UPLOAD |
                                           MAP_MSE_FEATURE_DELETE |
                                           MAP_MSE_FEATURE_INSTANCE_INFORMATION;
    state->sdp_config.service_name = "bt_hfp_chatd MAP MAS";
    state->sdp_config.service_description = "MAP MAS foundation for smart headset text path";
    state->sdp_config.provider_name = "bt_hfp_chatd";
}

int map_mse_rebuild_sdp(map_mse_state_t *state)
{
    if (!state) {
        return -1;
    }

    return map_sdp_build_mas_record_xml(&state->sdp_config,
                                        state->sdp_xml,
                                        sizeof(state->sdp_xml));
}

int map_mse_start(map_mse_state_t *state)
{
    int rc;

    if (!state || !state->store || !state->mns) {
        return -1;
    }

    rc = map_mse_rebuild_sdp(state);
    if (rc < 0) {
        return rc;
    }

    state->started = 1;
    state->mas_advertised = 1;
    map_mns_set_registration(state->mns, 1);
    return 0;
}

int map_mse_stop(map_mse_state_t *state)
{
    if (!state) {
        return -1;
    }

    state->started = 0;
    state->mas_advertised = 0;
    if (state->mns) {
        map_mns_set_registration(state->mns, 0);
    }
    return 0;
}

const char *map_mse_get_sdp_xml(const map_mse_state_t *state)
{
    if (!state) {
        return 0;
    }

    return state->sdp_xml;
}

int map_mse_set_notification_registration(map_mse_state_t *state, uint8_t enabled)
{
    if (!state || !state->mns) {
        return -1;
    }

    return map_mns_set_remote_notification_status(state->mns, enabled);
}

int map_mse_msg_send(map_mse_state_t *state,
                     const char *remote_addr,
                     const char *text,
                     uint8_t transparent,
                     msg_record_t *out_record)
{
    int rc;
    msg_record_t record;

    if (!state || !state->store || !state->started) {
        return -1;
    }

    rc = msg_store_add_outgoing(state->store, remote_addr, text, transparent, &record);
    if (rc < 0) {
        return rc;
    }

    msg_store_mark_sent(state->store, record.id);
    map_mse_queue_status_event(state, MAP_EVENT_SENDING_SUCCESS, &record, 1);

    if (out_record) {
        *out_record = record;
        out_record->status = MSG_STATUS_SENT;
    }

    return 0;
}

size_t map_mse_msg_list(map_mse_state_t *state, msg_record_t *out_records, size_t max_records)
{
    if (!state || !state->store) {
        return 0;
    }

    return msg_store_list(state->store, out_records, max_records);
}

int map_mse_msg_clear(map_mse_state_t *state)
{
    if (!state || !state->store) {
        return -1;
    }

    msg_store_clear(state->store);
    return 0;
}

int map_mse_on_delivery_result(map_mse_state_t *state, uint32_t id, uint8_t delivered)
{
    const msg_record_t *record;

    if (!state || !state->store) {
        return -1;
    }

    if (delivered) {
        msg_store_mark_delivered(state->store, id);
        record = msg_store_find_by_id(state->store, id);
        if (record) {
            map_mse_queue_status_event(state, MAP_EVENT_DELIVERY_SUCCESS, record, 1);
        }
    } else {
        msg_store_mark_failed(state->store, id);
        record = msg_store_find_by_id(state->store, id);
        if (record) {
            map_mse_queue_status_event(state, MAP_EVENT_SENDING_FAILURE, record, 0);
        }
    }

    return 0;
}

int map_mse_on_incoming_message(map_mse_state_t *state,
                                const char *remote_addr,
                                const char *text,
                                msg_record_t *out_record)
{
    int rc;
    msg_record_t record;
    map_event_t event_data;

    if (!state || !state->store || !state->started) {
        return -1;
    }

    rc = msg_store_add_incoming(state->store, remote_addr, text, &record);
    if (rc < 0) {
        return rc;
    }

    memset(&event_data, 0, sizeof(event_data));
    event_data.type = MAP_EVENT_NEW_MESSAGE;
    event_data.value = 1;
    memcpy(event_data.handle, record.handle, sizeof(event_data.handle) - 1);
    memcpy(event_data.folder, "telecom/msg/inbox", sizeof("telecom/msg/inbox"));
    memcpy(event_data.msg_type, "SMS_GSM", sizeof("SMS_GSM"));
    map_mns_queue_event(state->mns, &event_data);

    if (out_record) {
        *out_record = record;
    }

    return 0;
}
