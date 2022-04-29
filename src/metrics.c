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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "metrics.h"
#include "cJSON.h"
#include "cbor.h"

const struct Tags longNames = {
        "report_id",
        "version",
        "header",
        "metrics",
        "port",
        "ports",
        "interface",
        "total",
        "listening_tcp_ports",
        "listening_udp_ports",
        "bytes_in",
        "bytes_out",
        "packets_in",
        "packets_out",
        "network_stats",
        "remote_addr",
        "local_port",
        "local_interface",
        "connections",
        "established_connections",
        "tcp_connections"};


const struct Tags shortNames = {
        .REPORT_ID = "rid",
        .VERSION = "v",
        .HEADER = "hed",
        .METRICS = "met",
        .PORT = "pt",
        .PORTS =  "pts",
        .INTERFACE = "if",
        .TOTAL = "t",
        .LISTENING_TCP_PORTS = "tp",
        .LISTENING_UDP_PORTS = "up",
        .BYTES_IN = "bi",
        .BYTES_OUT = "bo",
        .PACKETS_IN = "pi",
        .PACKETS_OUT = "po",
        .NETWORK_STATS = "ns",
        .REMOTE_ADDR = "rad",
        .LOCAL_PORT = "lp",
        .LOCAL_INTERFACE = "li",
        .CONNECTIONS = "cs",
        .ESTABLISHED_CONNECTIONS = "ec",
        .TCP_CONNECTIONS = "tc"};


void printReportToConsole(const struct Report *report) {

    struct Header h = report->header;
    printf("Header:\n\tVersion:%s \n\tId:%i\n", h.version, h.reportId);

    struct metrics m = report->metrics;
    printf("Metrics:\n");
    printf("\tListening TCP PORTS\n\t\t");
    for (int i = 0; i < m.tcpPortCount; i++) {
        printf("%s, ", m.listeningTCPPorts[i].localPort);
    }
    printf("\tListening UDP PORTS\n\t\t");
    for (int i = 0; i < m.udpPortCount; i++) {
        printf("%s, ", m.listeningUDPPorts[i].localPort);
    }
    printf("\n\tTCP Connections\n");
    for (int i = 0; i < m.tcpConnectionCount; i++) {
        printf("\t\tlocal: %s:%s\n", m.tcpConnections[i].localAddress, m.tcpConnections[i].localPort);
        printf("\t\tremote: %s:%s\n", m.tcpConnections[i].remoteAddress, m.tcpConnections[i].remotePort);
    }

    NetworkStats s = m.networkStats;
    printf("\tNetwork Stats\n");
    printf("\t\tBytes in/out: %lu/%lu\n", s.bytesInDelta, s.bytesOutDelta);
    printf("\t\tPackets in/out: %lu/%lu\n", s.packetsInDelta, s.packetsOutDelta);

}

void generateJSONReport(const struct Report *rpt, char *json, int *length, enum tagType tagLen) {

    const struct Tags *t = NULL;

    if (tagLen == SHORT_NAMES) {
        t = &shortNames;
    } else {
        t = &longNames;
    }

    cJSON *report = cJSON_CreateObject();;
    cJSON *header = cJSON_CreateObject();

    cJSON_AddNumberToObject(header, t->REPORT_ID, rpt->header.reportId);
    cJSON_AddStringToObject(header, t->VERSION, rpt->header.version);

    cJSON_AddItemToObject(report, t->HEADER, header);

    cJSON *metrics = cJSON_CreateObject();
    //TODO check for NULL returns from cJSON

    //Listening TCP Ports
    cJSON *tcpPorts = cJSON_CreateObject();
    cJSON *ports = cJSON_CreateArray();

    for (int i = 0; i < rpt->metrics.tcpPortCount; i++) {
        cJSON *portDetail = cJSON_CreateObject();
        cJSON_AddNumberToObject(portDetail, t->PORT, atoi(rpt->metrics.listeningTCPPorts[i].localPort));
        if (strlen(rpt->metrics.listeningTCPPorts[i].localInterface) > 0) {
            cJSON_AddStringToObject(portDetail, t->INTERFACE, rpt->metrics.listeningTCPPorts[i].localInterface);
        }
        cJSON_AddItemToArray(ports, portDetail);
    }
    cJSON_AddItemToObject(tcpPorts, t->PORTS, ports);
    cJSON_AddNumberToObject(tcpPorts, t->TOTAL, rpt->metrics.tcpPortCount);
    cJSON_AddItemToObject(metrics, t->LISTENING_TCP_PORTS, tcpPorts);


    //Listening UDP Ports
    cJSON *udpPorts = cJSON_CreateObject();
    cJSON *portsArray = cJSON_CreateArray();

    //TODO PORT Count can be > than the number of ports in the list
    for (int i = 0; i < rpt->metrics.udpPortCount; i++) {
        cJSON *port = cJSON_CreateObject();
        cJSON_AddNumberToObject(port, t->PORT, atoi(rpt->metrics.listeningUDPPorts[i].localPort));

        if (strlen(rpt->metrics.listeningUDPPorts[i].localInterface) > 0) {
            cJSON_AddStringToObject(port, t->INTERFACE, rpt->metrics.listeningUDPPorts[i].localInterface);
        }
        cJSON_AddItemToArray(portsArray, port);
    }
    cJSON_AddItemToObject(udpPorts, t->PORTS, portsArray);
    cJSON_AddNumberToObject(udpPorts, t->TOTAL, rpt->metrics.udpPortCount);
    cJSON_AddItemToObject(metrics, t->LISTENING_UDP_PORTS, udpPorts);


    //Network Stats
    cJSON *stats = cJSON_CreateObject();
    cJSON_AddNumberToObject(stats, t->BYTES_IN, rpt->metrics.networkStats.bytesInDelta);
    cJSON_AddNumberToObject(stats, t->BYTES_OUT, rpt->metrics.networkStats.bytesOutDelta);
    cJSON_AddNumberToObject(stats, t->PACKETS_IN, rpt->metrics.networkStats.packetsInDelta);
    cJSON_AddNumberToObject(stats, t->PACKETS_OUT, rpt->metrics.networkStats.packetsOutDelta);
    cJSON_AddItemToObject(metrics, t->NETWORK_STATS, stats);

    //TCP Connections
    cJSON *tcpConnections = cJSON_CreateObject();
    cJSON *establishedConnections = cJSON_CreateObject();
    cJSON *connections = cJSON_CreateArray();

    for (int i = 0; i < rpt->metrics.tcpConnectionCount; i++) {
        cJSON *connection = cJSON_CreateObject();
        //TODO concatenate the port to the address with a ":"
        char remote[100];
        snprintf(remote, 100, "%s:%s", rpt->metrics.tcpConnections[i].remoteAddress,
                 rpt->metrics.tcpConnections[i].remotePort);
        cJSON_AddStringToObject(connection, t->REMOTE_ADDR, remote);
        if (strlen(rpt->metrics.tcpConnections[i].localInterface) > 0) {
            cJSON_AddStringToObject(connection, t->LOCAL_INTERFACE, rpt->metrics.tcpConnections[i].localInterface);
        }
        if (rpt->metrics.tcpConnections[i].localPort > 0) {
            cJSON_AddNumberToObject(connection, t->LOCAL_PORT, atoi(rpt->metrics.tcpConnections[i].localPort));
        }
        cJSON_AddItemToArray(connections, connection);
    }

    cJSON_AddItemToObject(establishedConnections, t->CONNECTIONS, connections);
    cJSON_AddNumberToObject(establishedConnections, t->TOTAL, rpt->metrics.tcpConnectionCount);
    cJSON_AddItemToObject(tcpConnections, t->ESTABLISHED_CONNECTIONS, establishedConnections);
    cJSON_AddItemToObject(metrics, t->TCP_CONNECTIONS, tcpConnections);


    cJSON_AddItemToObject(report, t->METRICS, metrics);

    char *jsonStringFormatted = cJSON_Print(report);
    printf("JSON Report: \n %s\n", jsonStringFormatted);
    free(jsonStringFormatted);

    char *jsonStringUnformatted = cJSON_PrintUnformatted(report);
    strncpy(json, jsonStringUnformatted, MAX_REPORT_SIZE);
    *length = strlen(json);
    printf("Report Length: %i\n", *length);
    cJSON_Delete(report);
    free(jsonStringUnformatted);

}


void generateCBORReport(const struct Report *rpt, char *cbor, int *length, enum tagType tagLen) {

    const struct Tags *t = NULL;

    if (tagLen == SHORT_NAMES) {
        t = &shortNames;
    } else {
        t = &longNames;
    }

    CborEncoder encoder, report, header, metrics;
    uint8_t buffer[MAX_REPORT_SIZE];
    cbor_encoder_init(&encoder, buffer, MAX_REPORT_SIZE, 0);
    cbor_encoder_create_map(&encoder, &report, 2);

    //Header
    cbor_encode_text_stringz(&report, t->HEADER);
    cbor_encoder_create_map(&report, &header, 2);
    cbor_encode_text_stringz(&header, t->REPORT_ID);
    cbor_encode_int(&header, rpt->header.reportId);
    cbor_encode_text_stringz(&header, t->VERSION);
    cbor_encode_text_stringz(&header, rpt->header.version);
    cbor_encoder_close_container(&report, &header);

    //Metrics
    cbor_encode_text_stringz(&report, t->METRICS);
    cbor_encoder_create_map(&report, &metrics, CborIndefiniteLength);

    //Listening TCP Ports
    if (rpt->metrics.listeningTCPPorts != NULL) {
        CborEncoder listeningTCP, tcpPorts;
        cbor_encode_text_stringz(&metrics, t->LISTENING_TCP_PORTS);
        cbor_encoder_create_map(&metrics, &listeningTCP, 2);
        cbor_encode_text_stringz(&listeningTCP, t->PORTS);
        cbor_encoder_create_array(&listeningTCP, &tcpPorts, rpt->metrics.tcpPortCount);
        for (int i = 0; i < rpt->metrics.tcpPortCount; i++) {
            NetworkConnection portDetail = rpt->metrics.listeningTCPPorts[i];

            CborEncoder portEncoder;
            cbor_encoder_create_map(&tcpPorts, &portEncoder, CborIndefiniteLength);
            if (strlen(portDetail.localInterface) > 0) {
                cbor_encode_text_stringz(&portEncoder, t->INTERFACE);
                cbor_encode_text_stringz(&portEncoder, portDetail.localInterface);
            }
            if (portDetail.localPort > 0) {
                cbor_encode_text_stringz(&portEncoder, t->PORT);
                cbor_encode_int(&portEncoder, atoi(portDetail.localPort));
            }
            cbor_encoder_close_container(&tcpPorts, &portEncoder);
        }

        cbor_encoder_close_container(&listeningTCP, &tcpPorts);
        if (rpt->metrics.tcpPortCount >= 0) {
            cbor_encode_text_stringz(&listeningTCP, t->TOTAL);
            cbor_encode_int(&listeningTCP, rpt->metrics.tcpPortCount);
        }
        cbor_encoder_close_container(&metrics, &listeningTCP);
    }

    //Listening TCP Ports
    if (rpt->metrics.listeningUDPPorts != NULL) {
        CborEncoder listeningUDP, UDPPorts;
        cbor_encode_text_stringz(&metrics, t->LISTENING_UDP_PORTS);
        cbor_encoder_create_map(&metrics, &listeningUDP, 2);
        cbor_encode_text_stringz(&listeningUDP, t->PORTS);
        cbor_encoder_create_array(&listeningUDP, &UDPPorts, rpt->metrics.udpPortCount);
        for (int i = 0; i < rpt->metrics.udpPortCount; i++) {
            NetworkConnection portDetail = rpt->metrics.listeningUDPPorts[i];

            CborEncoder portEncoder;
            cbor_encoder_create_map(&UDPPorts, &portEncoder, CborIndefiniteLength);
            if (strlen(portDetail.localInterface) > 0) {
                cbor_encode_text_stringz(&portEncoder, t->INTERFACE);
                cbor_encode_text_stringz(&portEncoder, portDetail.localInterface);
            }
            if (portDetail.localPort > 0) {
                cbor_encode_text_stringz(&portEncoder, t->PORT);
                cbor_encode_int(&portEncoder, atoi(portDetail.localPort));
            }
            cbor_encoder_close_container(&UDPPorts, &portEncoder);
        }

        cbor_encoder_close_container(&listeningUDP, &UDPPorts);
        if (rpt->metrics.udpPortCount >= 0) {
            cbor_encode_text_stringz(&listeningUDP, t->TOTAL);
            cbor_encode_int(&listeningUDP, rpt->metrics.udpPortCount);
        }
        cbor_encoder_close_container(&metrics, &listeningUDP);
    }

    //Network Stats
    if (rpt->metrics.networkStats.packetsOutDelta > 0 || rpt->metrics.networkStats.bytesOutDelta > 0
        || rpt->metrics.networkStats.packetsInDelta > 0 || rpt->metrics.networkStats.bytesInDelta > 0) {

        CborEncoder netStats;
        cbor_encode_text_stringz(&metrics, t->NETWORK_STATS);
        cbor_encoder_create_map(&metrics, &netStats, CborIndefiniteLength);

        if (rpt->metrics.networkStats.bytesInDelta > 0) {
            cbor_encode_text_stringz(&netStats, t->BYTES_IN);
            cbor_encode_int(&netStats, rpt->metrics.networkStats.bytesInDelta);
        }

        if (rpt->metrics.networkStats.bytesOutDelta > 0) {
            cbor_encode_text_stringz(&netStats, t->BYTES_OUT);
            cbor_encode_int(&netStats, rpt->metrics.networkStats.bytesOutDelta);
        }

        if (rpt->metrics.networkStats.packetsInDelta > 0) {
            cbor_encode_text_stringz(&netStats, t->PACKETS_IN);
            cbor_encode_int(&netStats, rpt->metrics.networkStats.packetsInDelta);
        }

        if (rpt->metrics.networkStats.packetsOutDelta > 0) {
            cbor_encode_text_stringz(&netStats, t->PACKETS_OUT);
            cbor_encode_int(&netStats, rpt->metrics.networkStats.packetsOutDelta);
        }

        cbor_encoder_close_container(&metrics, &netStats);
    }

    //TCP Connections
    if (rpt->metrics.tcpConnections != NULL) {
        CborEncoder tcpConnections, establishedConnections, connections;
        cbor_encode_text_stringz(&metrics, t->TCP_CONNECTIONS);
        cbor_encoder_create_map(&metrics, &tcpConnections, CborIndefiniteLength);
        cbor_encode_text_stringz(&tcpConnections, t->ESTABLISHED_CONNECTIONS);
        cbor_encoder_create_map(&tcpConnections, &establishedConnections, CborIndefiniteLength);
        cbor_encode_text_stringz(&establishedConnections, t->CONNECTIONS);
        cbor_encoder_create_array(&establishedConnections, &connections, CborIndefiniteLength);

        for (int i = 0; i < rpt->metrics.tcpConnectionCount; i++) {
            NetworkConnection connectionDetail = rpt->metrics.tcpConnections[i];
            CborEncoder connectionEncoder;
            cbor_encoder_create_map(&connections, &connectionEncoder, CborIndefiniteLength);

            if (strlen(connectionDetail.localInterface) > 0) {
                cbor_encode_text_stringz(&connectionEncoder, t->LOCAL_INTERFACE);
                cbor_encode_text_stringz(&connectionEncoder, connectionDetail.localInterface);
            }

            if (connectionDetail.localPort > 0) {
                cbor_encode_text_stringz(&connectionEncoder, t->LOCAL_PORT);
                cbor_encode_text_stringz(&connectionEncoder, connectionDetail.localPort);
            }

            if (strlen(connectionDetail.remoteAddress) > 0) {
                cbor_encode_text_stringz(&connectionEncoder, t->REMOTE_ADDR);
                if (connectionDetail.localPort > 0) {
                    char remoteAddr[MAX_IP_ADDR_STRING_LENGTH + MAX_PORT_STRING_LENGTH];
                    sprintf(remoteAddr, "%s:%s", connectionDetail.remoteAddress, connectionDetail.remotePort);
                    cbor_encode_text_stringz(&connectionEncoder, remoteAddr);
                } else {
                    cbor_encode_text_stringz(&connectionEncoder, connectionDetail.remoteAddress);
                }
            }
            cbor_encoder_close_container(&connections, &connectionEncoder);
        }
        cbor_encoder_close_container(&establishedConnections, &connections);

        if (rpt->metrics.tcpConnectionCount >= 0) {
            cbor_encode_text_stringz(&establishedConnections, t->TOTAL);
            cbor_encode_int(&establishedConnections, rpt->metrics.tcpConnectionCount);
        }
        cbor_encoder_close_container(&tcpConnections, &establishedConnections);
        cbor_encoder_close_container(&metrics, &tcpConnections);
    }
    cbor_encoder_close_container(&report, &metrics);
    cbor_encoder_close_container(&encoder, &report);

    size_t len = -1;
    len = cbor_encoder_get_buffer_size(&encoder, buffer);
    printf("Buffer Length: %zu\n", len);

    *length = len;
    memcpy(cbor, buffer, len);

    //DEBUG ONLY
    CborParser parser;
    CborValue value;
    cbor_parser_init(buffer, MAX_REPORT_SIZE, 0, &parser, &value);
    cbor_value_to_pretty(stdout, &value);
    printf("\n");

}

