/*
* Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
* http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#ifndef AWSIOTDEVICEDEFENDERAGENT_METRICS_H
#define AWSIOTDEVICEDEFENDERAGENT_METRICS_H

#include "agent_config.h"

static const int MAX_CHAR = 10000;

//You can decrease this size of addr string length if you know you aren't using ipv6
static const int MAX_REPORT_SIZE = 128000;
#define MAX_IP_ADDR_STRING_LENGTH 46  //15 for v4, 45 for v6
#define MAX_PORT_STRING_LENGTH 6
#define MAX_INTERFACE_NAME_LENGTH 16


/**
 * @brief TCP Connection States
 */
enum state {
    ESTABLISHED = 1, LISTEN, OTHER
};

/**
 * @brief Network Protocols supported
 */
enum protocol {
    TCP = 1, UDP
};

/**
 * @Aggregate Network stats
 */
typedef struct {
    char *interface; /** Network Interface Name */
    unsigned long bytesIn;
    unsigned long bytesOut;
    unsigned long packetsIn;
    unsigned long packetsOut;
} NetworkStats;

typedef struct {
    char localInterface[MAX_INTERFACE_NAME_LENGTH]; /** Network Interface Name */
    char localAddress[MAX_IP_ADDR_STRING_LENGTH];  /** Local IP Address */
    char localPort[MAX_PORT_STRING_LENGTH]; /** Local TCP Port */
    char remoteAddress[MAX_IP_ADDR_STRING_LENGTH]; /** Remote Peer IP Address */
    char remotePort[MAX_PORT_STRING_LENGTH]; /** Remote peer TCP Port */
    enum state connectionState;
} NetworkConnection;


/**
 * @brief Metrics Report header information
 */
struct Header {
    unsigned int reportId; /** should be an increasing, positive integer */
    char *version;
};

/**
 * @brief Metrics Block Portion of Report
 */
struct metrics {
    NetworkConnection *tcpConnections; /** An array of TCP connections */
    int tcpConnectionCount;  /** When using sampled list, may be larger than the number of items in connection list */
    NetworkConnection *listeningUDPPorts; /** Array of listening UDP ports */
    int udpPortCount; /** When using sampled list, may be larger than the number of items in port list */
    NetworkConnection *listeningTCPPorts; /** Array of listening TCP ports */
    int tcpPortCount; /** When using sampled list, may be larger than the number of items in port list */
    NetworkStats networkStats;
};


/**
 * @brief Overall Metrics report structure
 */
struct Report {
    struct Header header;
    struct metrics metrics;
};


/**
 * @brief Report field names
 */
struct Tags {
    const char *REPORT_ID;
    const char *VERSION;
    const char *HEADER;
    const char *METRICS;
    const char *PORT;
    const char *PORTS;
    const char *INTERFACE;
    const char *TOTAL;
    const char *LISTENING_TCP_PORTS;
    const char *LISTENING_UDP_PORTS;
    const char *BYTES_IN;
    const char *BYTES_OUT;
    const char *PACKETS_IN;
    const char *PACKETS_OUT;
    const char *NETWORK_STATS;
    const char *REMOTE_ADDR;
    const char *LOCAL_PORT;
    const char *LOCAL_INTERFACE;
    const char *CONNECTIONS;
    const char *ESTABLISHED_CONNECTIONS;
    const char *TCP_CONNECTIONS;
};

/**
 * Generate a metrics report in JSON Format
 *
 * @param [in] report Internal representation of report data
 * @param [out] json JSON formatted report, suitable for submission to Device Defender
 * @param [out] length Length of the Generated JSON
 * @param [in] tags Field name length to use
 */
void generateJSONReport(const struct Report *report, char *json, int *length, enum tagType tags);

void generateCBORReport(const struct Report *report, char *json, int *length, enum tagType tags);

/**
 * @brief Prints a compact view of the report to the console for debugging purposes only
 * @param report
 */
void printReportToConsole(const struct Report *report);

#endif //AWSIOTDEVICEDEFENDERAGENT_METRICS_H
