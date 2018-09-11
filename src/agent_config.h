//
// Created by Miller, Daniel on 11/21/18.
//
#ifndef AWSIOTDEVICEDEFENDERAGENT_AGENT_CONFIG_H
#define AWSIOTDEVICEDEFENDERAGENT_AGENT_CONFIG_H

#include <stdint.h>

#define HOST_ADDRESS_SIZE 255
#define MAX_TOPIC_LENGTH 256


/**
 * @brief Indicates use of long or short field names ("established_connections" vs "ec")
 */
enum tagType {
    LONG_NAMES = 1, SHORT_NAMES
};

/**
 * @brief Report Format
 */
enum format {
    JSON = 1, CBOR
};

extern int PUBLISH_INTERVAL;

extern enum format REPORT_FORMAT;
extern enum tagType TAG_LENGTH;


#endif //AWSIOTDEVICEDEFENDERAGENT_AGENT_CONFIG_H
