#include "msg_store.h"

#include <stdio.h>
#include <string.h>

static void msg_store_safe_copy(char *dst, size_t dst_size, const char *src)
{
    if (!dst || !dst_size) {
        return;
    }

    if (!src) {
        dst[0] = '\0';
        return;
    }

    snprintf(dst, dst_size, "%s", src);
}

static void msg_store_make_handle(uint32_t id, char *handle, size_t handle_size)
{
    snprintf(handle, handle_size, "%08X", id);
}

static msg_record_t *msg_store_find_mutable(msg_store_t *store, uint32_t id)
{
    size_t i;

    if (!store) {
        return 0;
    }

    for (i = 0; i < store->count; ++i) {
        if (store->records[i].id == id) {
            return &store->records[i];
        }
    }

    return 0;
}

void msg_store_init(msg_store_t *store)
{
    if (!store) {
        return;
    }

    memset(store, 0, sizeof(*store));
    store->next_id = 1;
    store->next_change_id = 1;
}

static int msg_store_add_common(msg_store_t *store,
                                const char *remote_addr,
                                const char *text,
                                msg_direction_e direction,
                                uint8_t transparent,
                                msg_record_t *out_record)
{
    msg_record_t *record;

    if (!store || !remote_addr || !text) {
        return -1;
    }

    if (store->count >= MSG_STORE_MAX_RECORDS) {
        return -2;
    }

    record = &store->records[store->count];
    memset(record, 0, sizeof(*record));

    record->id = store->next_id++;
    msg_store_make_handle(record->id, record->handle, sizeof(record->handle));
    msg_store_safe_copy(record->remote_addr, sizeof(record->remote_addr), remote_addr);
    msg_store_safe_copy(record->text, sizeof(record->text), text);
    record->direction = direction;
    record->status = (direction == MSG_DIR_OUTGOING) ? MSG_STATUS_PENDING : MSG_STATUS_DELIVERED;
    record->transparent = transparent;
    record->read = (direction == MSG_DIR_OUTGOING) ? 1 : 0;
    record->timestamp = time(0);
    record->change_id = store->next_change_id++;

    ++store->count;

    if (out_record) {
        *out_record = *record;
    }

    return 0;
}

int msg_store_add_outgoing(msg_store_t *store,
                           const char *remote_addr,
                           const char *text,
                           uint8_t transparent,
                           msg_record_t *out_record)
{
    return msg_store_add_common(store, remote_addr, text, MSG_DIR_OUTGOING, transparent, out_record);
}

int msg_store_add_incoming(msg_store_t *store,
                           const char *remote_addr,
                           const char *text,
                           msg_record_t *out_record)
{
    return msg_store_add_common(store, remote_addr, text, MSG_DIR_INCOMING, 0, out_record);
}

static int msg_store_update_status(msg_store_t *store, uint32_t id, msg_status_e status)
{
    msg_record_t *record = msg_store_find_mutable(store, id);

    if (!record) {
        return -1;
    }

    record->status = status;
    record->change_id = store->next_change_id++;
    return 0;
}

int msg_store_mark_sent(msg_store_t *store, uint32_t id)
{
    return msg_store_update_status(store, id, MSG_STATUS_SENT);
}

int msg_store_mark_delivered(msg_store_t *store, uint32_t id)
{
    return msg_store_update_status(store, id, MSG_STATUS_DELIVERED);
}

int msg_store_mark_failed(msg_store_t *store, uint32_t id)
{
    msg_record_t *record = msg_store_find_mutable(store, id);

    if (!record) {
        return -1;
    }

    record->status = MSG_STATUS_FAILED;
    if (record->retry_count < 255) {
        ++record->retry_count;
    }
    record->change_id = store->next_change_id++;
    return 0;
}

int msg_store_mark_read(msg_store_t *store, uint32_t id, uint8_t read)
{
    msg_record_t *record = msg_store_find_mutable(store, id);

    if (!record) {
        return -1;
    }

    record->read = read ? 1 : 0;
    record->change_id = store->next_change_id++;
    return 0;
}

int msg_store_remove(msg_store_t *store, uint32_t id)
{
    size_t i;

    if (!store) {
        return -1;
    }

    for (i = 0; i < store->count; ++i) {
        if (store->records[i].id == id) {
            size_t remaining = store->count - i - 1;
            if (remaining) {
                memmove(&store->records[i], &store->records[i + 1], remaining * sizeof(store->records[0]));
            }
            memset(&store->records[store->count - 1], 0, sizeof(store->records[0]));
            --store->count;
            return 0;
        }
    }

    return -1;
}

void msg_store_clear(msg_store_t *store)
{
    if (!store) {
        return;
    }

    memset(store->records, 0, sizeof(store->records));
    store->count = 0;
    store->next_change_id++;
}

size_t msg_store_list(const msg_store_t *store, msg_record_t *out_records, size_t max_records)
{
    size_t copy_count;

    if (!store || !out_records || !max_records) {
        return 0;
    }

    copy_count = (store->count < max_records) ? store->count : max_records;
    memcpy(out_records, store->records, copy_count * sizeof(store->records[0]));
    return copy_count;
}

const msg_record_t *msg_store_find_by_id(const msg_store_t *store, uint32_t id)
{
    size_t i;

    if (!store) {
        return 0;
    }

    for (i = 0; i < store->count; ++i) {
        if (store->records[i].id == id) {
            return &store->records[i];
        }
    }

    return 0;
}

const msg_record_t *msg_store_find_by_handle(const msg_store_t *store, const char *handle)
{
    size_t i;

    if (!store || !handle) {
        return 0;
    }

    for (i = 0; i < store->count; ++i) {
        if (strcmp(store->records[i].handle, handle) == 0) {
            return &store->records[i];
        }
    }

    return 0;
}
