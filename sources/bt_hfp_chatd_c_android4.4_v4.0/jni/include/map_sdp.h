#ifndef MAP_SDP_H
#define MAP_SDP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_SDP_XML_MAX_LEN 4096

typedef struct map_mas_sdp_config_s {
    uint8_t mas_instance_id;
    uint8_t rfcomm_channel;
    uint16_t l2cap_psm;
    uint8_t supported_message_types;
    uint32_t supported_features;
    const char *service_name;
    const char *service_description;
    const char *provider_name;
} map_mas_sdp_config_t;

int map_sdp_build_mas_record_xml(const map_mas_sdp_config_t *config,
                                 char *xml_buffer,
                                 size_t xml_buffer_size);

#ifdef __cplusplus
}
#endif

#endif
