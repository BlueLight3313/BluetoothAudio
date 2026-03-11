#include "map_sdp.h"

#include <stdio.h>
#include <string.h>

#ifndef MAP_MAS_DEFAULT_SERVICE_NAME
#define MAP_MAS_DEFAULT_SERVICE_NAME "Bluetooth Message Access"
#endif

#ifndef MAP_MAS_DEFAULT_SERVICE_DESC
#define MAP_MAS_DEFAULT_SERVICE_DESC "MAP MAS for smart headset interoperability"
#endif

#ifndef MAP_MAS_DEFAULT_PROVIDER
#define MAP_MAS_DEFAULT_PROVIDER "bt_hfp_chatd"
#endif

int map_sdp_build_mas_record_xml(const map_mas_sdp_config_t *config,
                                 char *xml_buffer,
                                 size_t xml_buffer_size)
{
    const char *service_name;
    const char *service_description;
    const char *provider_name;
    int written;

    if (!config || !xml_buffer || xml_buffer_size < 64) {
        return -1;
    }

    service_name = config->service_name ? config->service_name : MAP_MAS_DEFAULT_SERVICE_NAME;
    service_description = config->service_description ? config->service_description : MAP_MAS_DEFAULT_SERVICE_DESC;
    provider_name = config->provider_name ? config->provider_name : MAP_MAS_DEFAULT_PROVIDER;

    written = snprintf(
        xml_buffer,
        xml_buffer_size,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<record>\n"
        "  <attribute id=\"0x0001\">\n"
        "    <sequence>\n"
        "      <uuid value=\"0x1132\"/>\n"
        "      <uuid value=\"0x1134\"/>\n"
        "    </sequence>\n"
        "  </attribute>\n"
        "  <attribute id=\"0x0004\">\n"
        "    <sequence>\n"
        "      <sequence>\n"
        "        <uuid value=\"0x0100\"/>\n"
        "      </sequence>\n"
        "      <sequence>\n"
        "        <uuid value=\"0x0003\"/>\n"
        "        <uint8 value=\"0x%02X\"/>\n"
        "      </sequence>\n"
        "    </sequence>\n"
        "  </attribute>\n"
        "  <attribute id=\"0x0005\">\n"
        "    <sequence>\n"
        "      <uuid value=\"0x1002\"/>\n"
        "    </sequence>\n"
        "  </attribute>\n"
        "  <attribute id=\"0x0009\">\n"
        "    <sequence>\n"
        "      <sequence>\n"
        "        <uuid value=\"0x1134\"/>\n"
        "        <uint16 value=\"0x0104\"/>\n"
        "      </sequence>\n"
        "    </sequence>\n"
        "  </attribute>\n"
        "  <attribute id=\"0x0100\"><text value=\"%s\"/></attribute>\n"
        "  <attribute id=\"0x0101\"><text value=\"%s\"/></attribute>\n"
        "  <attribute id=\"0x0102\"><text value=\"%s\"/></attribute>\n"
        "  <attribute id=\"0x0315\"><uint8 value=\"0x%02X\"/></attribute>\n"
        "  <attribute id=\"0x0316\"><uint8 value=\"0x%02X\"/></attribute>\n"
        "  <attribute id=\"0x0317\"><uint32 value=\"0x%08X\"/></attribute>\n"
        "  <attribute id=\"0x0200\"><uint16 value=\"0x%04X\"/></attribute>\n"
        "</record>\n",
        config->rfcomm_channel,
        service_name,
        service_description,
        provider_name,
        config->mas_instance_id,
        config->supported_message_types,
        (unsigned int)config->supported_features,
        config->l2cap_psm);

    if (written < 0 || (size_t)written >= xml_buffer_size) {
        if (xml_buffer_size) {
            xml_buffer[0] = '\0';
        }
        return -2;
    }

    return written;
}
