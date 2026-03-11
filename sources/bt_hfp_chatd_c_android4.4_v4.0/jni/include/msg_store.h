#ifndef MSG_STORE_H
#define MSG_STORE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_STORE_MAX_TEXT_LEN        512
#define MSG_STORE_MAX_ADDR_LEN         64
#define MSG_STORE_MAX_HANDLE_LEN       32
#define MSG_STORE_MAX_RECORDS         128

typedef enum {
    MSG_DIR_OUTGOING = 0,
    MSG_DIR_INCOMING = 1
} msg_direction_e;

typedef enum {
    MSG_STATUS_PENDING = 0,
    MSG_STATUS_SENT = 1,
    MSG_STATUS_DELIVERED = 2,
    MSG_STATUS_FAILED = 3,
    MSG_STATUS_DELETED = 4
} msg_status_e;

typedef struct msg_record_s {
    uint32_t id;
    char handle[MSG_STORE_MAX_HANDLE_LEN];
    char remote_addr[MSG_STORE_MAX_ADDR_LEN];
    char text[MSG_STORE_MAX_TEXT_LEN];
    uint8_t read;
    uint8_t transparent;
    uint8_t retry_count;
    uint8_t reserved0;
    msg_direction_e direction;
    msg_status_e status;
    time_t timestamp;
    uint32_t change_id;
} msg_record_t;

typedef struct msg_store_s {
    msg_record_t records[MSG_STORE_MAX_RECORDS];
    size_t count;
    uint32_t next_id;
    uint32_t next_change_id;
} msg_store_t;

void msg_store_init(msg_store_t *store);
int msg_store_add_outgoing(msg_store_t *store,
                           const char *remote_addr,
                           const char *text,
                           uint8_t transparent,
                           msg_record_t *out_record);
int msg_store_add_incoming(msg_store_t *store,
                           const char *remote_addr,
                           const char *text,
                           msg_record_t *out_record);
int msg_store_mark_sent(msg_store_t *store, uint32_t id);
int msg_store_mark_delivered(msg_store_t *store, uint32_t id);
int msg_store_mark_failed(msg_store_t *store, uint32_t id);
int msg_store_mark_read(msg_store_t *store, uint32_t id, uint8_t read);
int msg_store_remove(msg_store_t *store, uint32_t id);
void msg_store_clear(msg_store_t *store);
size_t msg_store_list(const msg_store_t *store, msg_record_t *out_records, size_t max_records);
const msg_record_t *msg_store_find_by_id(const msg_store_t *store, uint32_t id);
const msg_record_t *msg_store_find_by_handle(const msg_store_t *store, const char *handle);

#ifdef __cplusplus
}
#endif

#endif
