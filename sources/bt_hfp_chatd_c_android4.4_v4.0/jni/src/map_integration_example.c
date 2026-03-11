#include "map_mse.h"

#include <stdio.h>
#include <string.h>

/*
 * This file is intentionally small and explicit.
 * It shows how v4.0 MAP additions can be wired into the existing daemon loop.
 * Replace the printf paths with your existing logging, OBEX, RFCOMM, and control handlers.
 */

static msg_store_t g_msg_store;
static map_mns_state_t g_map_mns;
static map_mse_state_t g_map_mse;

int bt_hfp_chatd_map_init_example(void)
{
    msg_store_init(&g_msg_store);
    map_mns_init(&g_map_mns);
    map_mse_init(&g_map_mse, &g_msg_store, &g_map_mns);
    return map_mse_start(&g_map_mse);
}

int bt_hfp_chatd_on_msg_send_example(const char *remote_addr, const char *text)
{
    msg_record_t record;
    int rc;

    rc = map_mse_msg_send(&g_map_mse, remote_addr, text, 0, &record);
    if (rc == 0) {
        printf("MSG_SEND queued handle=%s id=%u\\n", record.handle, record.id);
    }
    return rc;
}

int bt_hfp_chatd_on_notification_registration_example(uint8_t enabled)
{
    return map_mse_set_notification_registration(&g_map_mse, enabled);
}

int bt_hfp_chatd_on_delivery_report_example(uint32_t id, uint8_t delivered)
{
    return map_mse_on_delivery_result(&g_map_mse, id, delivered);
}

int bt_hfp_chatd_flush_map_events_example(void)
{
    map_event_t event_data;
    char xml_buffer[MAP_MNS_EVENT_XML_MAX_LEN];
    int written;

    while (map_mns_pop_event(&g_map_mns, &event_data) == 0) {
        written = map_mns_build_event_report_xml(&event_data, xml_buffer, sizeof(xml_buffer));
        if (written > 0) {
            printf("MAP notify XML:\n%s\n", xml_buffer);
        }
    }

    return 0;
}
