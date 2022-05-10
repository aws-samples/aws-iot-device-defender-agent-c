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
#include <stdbool.h>
#include "stdlib.h"
#include "string.h"

#include <cJSON.h>
#include "unity.h"

#include "collector.h"
#include "cbor.h"

bool cborStringAssert(const char*expected, CborValue *it) {
    bool result = false;
    TEST_ASSERT_TRUE(cbor_value_is_text_string(it));
    cbor_value_text_string_equals(it,expected,&result);
    return result;
}

bool cborArrayAssertSize(const int expected, const CborValue *it) {
    size_t len = -1;
    TEST_ASSERT_TRUE(cbor_value_is_array(it));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_get_array_length(it,(size_t *)&len));

    return (len == expected);
}

bool cborMapAssertSize(const int expected, const CborValue *it) {
    size_t len = -1;
    TEST_ASSERT_TRUE(cbor_value_is_map(it));
    TEST_ASSERT_TRUE(cbor_value_is_length_known(it));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_get_map_length(it,(size_t *)&len));

    return (len == expected);
}

int cborAdvanceToEnd(CborValue *it) {
    int moves = 0;

    while(!cbor_value_at_end(it)) {
        cbor_value_advance(it);
        moves++;
    }

    return moves;
}

void test_JSONMetricsBasicStructure_LongTags() {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, LONG_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *header = cJSON_GetObjectItemCaseSensitive(report,"header");

    TEST_ASSERT_TRUE(cJSON_IsObject(header));
    cJSON *id = cJSON_GetObjectItem(header,"report_id");
    TEST_ASSERT_TRUE(cJSON_IsNumber(id));
    TEST_ASSERT_TRUE(id->valueint >= 0);
    cJSON *version = cJSON_GetObjectItem(header,"version");
    TEST_ASSERT_EQUAL_STRING("1.0",cJSON_GetStringValue(version));

    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"metrics");

    cJSON *tcpPorts = cJSON_GetObjectItem(metrics,"listening_tcp_ports");
    TEST_ASSERT_TRUE(cJSON_IsObject(tcpPorts));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(tcpPorts,"ports")));

    cJSON *udpPorts = cJSON_GetObjectItem(metrics,"listening_udp_ports");
    TEST_ASSERT_TRUE(cJSON_IsObject(udpPorts));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(udpPorts,"ports")));

    cJSON *netstats = cJSON_GetObjectItem(metrics,"network_stats");
    TEST_ASSERT_TRUE(cJSON_IsObject(netstats));

    cJSON *connections = cJSON_GetObjectItem(metrics,"tcp_connections");
    TEST_ASSERT_TRUE(cJSON_IsObject(connections));
    cJSON *established = cJSON_GetObjectItem(connections,"established_connections");
    TEST_ASSERT_TRUE(cJSON_IsObject(established));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(established,"connections")));

    cJSON_Delete(report);
}

void test_JSONMetricsBasicStructure_ShortTags() {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, SHORT_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *header = cJSON_GetObjectItemCaseSensitive(report,"hed");

    TEST_ASSERT_TRUE(cJSON_IsObject(header));
    cJSON *id = cJSON_GetObjectItem(header,"rid");
    TEST_ASSERT_TRUE(cJSON_IsNumber(id));
    TEST_ASSERT_TRUE(id->valueint >= 0);
    cJSON *version = cJSON_GetObjectItem(header,"v");
    TEST_ASSERT_EQUAL_STRING("1.0",cJSON_GetStringValue(version));

    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"met");

    cJSON *tcpPorts = cJSON_GetObjectItem(metrics,"tp");
    TEST_ASSERT_TRUE(cJSON_IsObject(tcpPorts));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(tcpPorts,"pts")));

    cJSON *udpPorts = cJSON_GetObjectItem(metrics,"up");
    TEST_ASSERT_TRUE(cJSON_IsObject(udpPorts));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(udpPorts,"pts")));

    cJSON *netstats = cJSON_GetObjectItem(metrics,"ns");
    TEST_ASSERT_TRUE(cJSON_IsObject(netstats));

    cJSON *connections = cJSON_GetObjectItem(metrics,"tc");
    TEST_ASSERT_TRUE(cJSON_IsObject(connections));
    cJSON *established = cJSON_GetObjectItem(connections,"ec");
    TEST_ASSERT_TRUE(cJSON_IsObject(established));
    TEST_ASSERT_TRUE(cJSON_IsArray(cJSON_GetObjectItem(established,"cs")));

    cJSON_Delete(report);
}

void test_tcpPortsListJSON_LongTags(void) {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, LONG_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"metrics");
    cJSON *tcpPorts = cJSON_GetObjectItem(metrics,"listening_tcp_ports");
    cJSON *portsList = cJSON_GetObjectItem(tcpPorts,"ports");

    cJSON *port = NULL;
    int items =0;
    cJSON_ArrayForEach(port, portsList) {
        TEST_ASSERT_TRUE(cJSON_IsObject(port));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(port,"port")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(port,"port")->valueint > 0);
        items++;
    }
    TEST_ASSERT_EQUAL(19,items);
    cJSON_Delete(report);
}

void test_tcpPortsListJSON_ShortTags(void) {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, SHORT_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"met");
    cJSON *tcpPorts = cJSON_GetObjectItem(metrics,"tp");
    cJSON *portsList = cJSON_GetObjectItem(tcpPorts,"pts");

    cJSON *port = NULL;
    int items =0;
    cJSON_ArrayForEach(port, portsList) {
        TEST_ASSERT_TRUE(cJSON_IsObject(port));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(port,"pt")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(port,"pt")->valueint > 0);
        items++;
    }
    TEST_ASSERT_EQUAL(19,items);
    cJSON_Delete(report);
}

void test_udpPortsListJSON_LongTags(void) {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, LONG_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"metrics");
    cJSON *udpPorts = cJSON_GetObjectItem(metrics,"listening_udp_ports");
    cJSON *portsList = cJSON_GetObjectItem(udpPorts,"ports");

    cJSON *port = NULL;
    int items =0;
    cJSON_ArrayForEach(port, portsList) {
        TEST_ASSERT_TRUE(cJSON_IsObject(port));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(port,"port")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(port,"port")->valueint > 0);
        items++;
    }
    TEST_ASSERT_EQUAL(18,items);
     cJSON_Delete(report);
}

void test_udpPortsListJSON_ShortTags(void) {

    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, SHORT_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"met");
    cJSON *udpPorts = cJSON_GetObjectItem(metrics,"up");
    cJSON *portsList = cJSON_GetObjectItem(udpPorts,"pts");

    cJSON *port = NULL;
    int items =0;
    cJSON_ArrayForEach(port, portsList) {
        TEST_ASSERT_TRUE(cJSON_IsObject(port));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(port,"pt")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(port,"pt")->valueint > 0);
        items++;
    }
    TEST_ASSERT_EQUAL(18,items);
    cJSON_Delete(report);
}

void test_netstatsJSON_LongTags(void) {
    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    stats.bytesInPrev = 1;
    stats.packetsInPrev = 1;
    stats.bytesOutPrev = 1;
    stats.packetsOutPrev = 1;
    generateMetricsReport(reportString, 128000, &length, &stats, LONG_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"metrics");
    cJSON *netstats = cJSON_GetObjectItem(metrics,"network_stats");

    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"bytes_in")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"bytes_out")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"packets_in")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"packets_out")));

    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"bytes_in")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"bytes_out")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"packets_in")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"packets_out")->valueint > 0);
    cJSON_Delete(report);
}

void test_netstatsJSON_ShortTags(void) {
    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    stats.bytesInPrev = 1;
    stats.packetsInPrev = 1;
    stats.bytesOutPrev = 1;
    stats.packetsOutPrev = 1;
    generateMetricsReport(reportString, 128000, &length, &stats, SHORT_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"met");
    cJSON *netstats = cJSON_GetObjectItem(metrics,"ns");

    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"bi")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"bo")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"pi")));
    TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(netstats,"po")));

    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"bi")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"bo")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"po")->valueint > 0);
    TEST_ASSERT_TRUE(cJSON_GetObjectItem(netstats,"pi")->valueint > 0);

    cJSON_Delete(report);
}

void test_tcpConnectionsJSON_LongTags(void) {
    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, LONG_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"metrics");
    cJSON *tcpConnections = cJSON_GetObjectItem(metrics,"tcp_connections");
    cJSON *established = cJSON_GetObjectItem(tcpConnections,"established_connections");
    cJSON *connections = cJSON_GetObjectItem(established,"connections");

    cJSON *conn = NULL;
    cJSON_ArrayForEach(conn, connections) {
        TEST_ASSERT_TRUE(cJSON_IsObject(conn));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(conn,"local_port")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(conn,"local_port")->valueint > 0);
        TEST_ASSERT_TRUE(cJSON_IsString(cJSON_GetObjectItem(conn,"remote_addr")));
    }
    TEST_ASSERT_EQUAL(4,cJSON_GetArraySize(connections));

     cJSON_Delete(report);
}

void test_tcpConnectionsJSON_ShortTags(void) {
    char reportString[128000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportString, 128000, &length, &stats, SHORT_NAMES, JSON);

    //Basic Structure
    cJSON *report = cJSON_Parse(reportString);
    cJSON *metrics = cJSON_GetObjectItemCaseSensitive(report,"met");
    cJSON *tcpConnections = cJSON_GetObjectItem(metrics,"tc");
    cJSON *established = cJSON_GetObjectItem(tcpConnections,"ec");
    cJSON *connections = cJSON_GetObjectItem(established,"cs");

    cJSON *conn = NULL;
    cJSON_ArrayForEach(conn, connections) {
        TEST_ASSERT_TRUE(cJSON_IsObject(conn));
        TEST_ASSERT_TRUE(cJSON_IsNumber(cJSON_GetObjectItem(conn,"lp")));
        TEST_ASSERT_TRUE(cJSON_GetObjectItem(conn,"lp")->valueint > 0);
        TEST_ASSERT_TRUE(cJSON_IsString(cJSON_GetObjectItem(conn,"rad")));
    }
    TEST_ASSERT_EQUAL(4,cJSON_GetArraySize(connections));

    cJSON_Delete(report);
}

void test_reportCBOR_BasicStructure_LongTags(void) {
    uint8_t reportBuffer[512000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportBuffer, 512000, &length, &stats, LONG_NAMES, CBOR);

    TEST_ASSERT_GREATER_OR_EQUAL(1,strlen(reportBuffer));
    TEST_ASSERT_GREATER_OR_EQUAL(1,length);

    CborParser parser;
    CborValue report, body, headerCtnt;

    char *buf;
    size_t n;
    bool result = false;

    TEST_ASSERT_EQUAL(CborNoError,cbor_parser_init((const uint8_t* )&reportBuffer, 512000, 0, &parser, &report));
    TEST_ASSERT_TRUE(cbor_value_is_container(&report));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&report,&body));

    cbor_value_text_string_equals(&body,"header",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&body);
    TEST_ASSERT_TRUE(cbor_value_is_map(&body));
    cbor_value_advance(&body);
    TEST_ASSERT_TRUE(cbor_value_is_text_string(&body));
    result = false;
    cbor_value_text_string_equals(&body,"metrics",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&body);
    TEST_ASSERT_TRUE(cbor_value_is_map(&body));
    cbor_value_advance(&body);
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_leave_container(&report,&body));

}


void test_reportCBOR_header_LongTags(void) {
    uint8_t reportBuffer[512000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportBuffer, 512000, &length, &stats, LONG_NAMES, CBOR);

    TEST_ASSERT_GREATER_OR_EQUAL(1,strlen(reportBuffer));
    TEST_ASSERT_GREATER_OR_EQUAL(1,length);

    CborParser parser;
    CborValue report, body, header;

    char *buf;
    size_t n;
    bool result = false;

    TEST_ASSERT_EQUAL(CborNoError,cbor_parser_init((const uint8_t* )&reportBuffer, 512000, 0, &parser, &report));
    TEST_ASSERT_TRUE(cbor_value_is_container(&report));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&report,&body));

    cbor_value_text_string_equals(&body,"header",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&body);
    TEST_ASSERT_TRUE(cbor_value_is_map(&body));
    cbor_value_enter_container(&body,&header);
    result = false;
    cbor_value_text_string_equals(&header,"report_id",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&header);
    TEST_ASSERT_TRUE(cbor_value_is_integer(&header));
    cbor_value_advance(&header);
    result = false;
    cbor_value_text_string_equals(&header,"version",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&header);
    result = false;
    cbor_value_text_string_equals(&header,"1.0",&result);
    TEST_ASSERT_TRUE(result);
    cbor_value_advance(&header);
    TEST_ASSERT_TRUE(cbor_value_at_end(&header));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_leave_container(&body,&header));

    while(!cbor_value_at_end(&body)) {
        cbor_value_advance(&body);
    }
    cbor_value_leave_container(&report,&body);

}

void test_reportCBOR_metrics_LongTags(void) {
    uint8_t reportBuffer[512000];
    int length = -1;
    NetworkStats stats;
    generateMetricsReport(reportBuffer, 512000, &length, &stats, LONG_NAMES, CBOR);

    TEST_ASSERT_GREATER_OR_EQUAL(1,strlen(reportBuffer));
    TEST_ASSERT_GREATER_OR_EQUAL(1,length);

    CborParser parser;
    CborValue report, body, metrics;

    char *buf;
    size_t n;
    int result = -1;

    TEST_ASSERT_EQUAL(CborNoError,cbor_parser_init((const uint8_t* )&reportBuffer, 512000, 0, &parser, &report));
    TEST_ASSERT_TRUE(cbor_value_is_container(&report));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&report,&body));

    cbor_value_advance(&body);  // header tag
    cbor_value_advance(&body);  // header map

    TEST_ASSERT_TRUE(cborStringAssert("metrics",&body));
    cbor_value_advance(&body); //metrics map
    TEST_ASSERT_TRUE(cbor_value_is_map(&body));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&body,&metrics));
    TEST_ASSERT_TRUE(cborStringAssert("listening_tcp_ports",&metrics));
    cbor_value_advance(&metrics);
    TEST_ASSERT_TRUE(cbor_value_is_map(&metrics));

    CborValue listeningTcp, tcpPorts;
    cbor_value_enter_container(&metrics,&listeningTcp);
    TEST_ASSERT_TRUE(cborStringAssert("ports",&listeningTcp));
    cbor_value_advance(&listeningTcp);
    TEST_ASSERT_TRUE(cbor_value_is_array(&listeningTcp));
    //TEST_ASSERT_TRUE(cborArrayAssertSize(19, &listeningTcp));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&listeningTcp,&tcpPorts));
    TEST_ASSERT_TRUE(cbor_value_is_map(&tcpPorts));
    //TEST_ASSERT_TRUE(cborMapAssertSize(1,&tcpPorts));

    TEST_ASSERT_EQUAL_INT(19,cborAdvanceToEnd(&tcpPorts));
    cbor_value_leave_container(&listeningTcp,&tcpPorts);
    TEST_ASSERT_TRUE(cborStringAssert("total",&listeningTcp));
    cbor_value_advance(&listeningTcp);
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_get_int(&listeningTcp,&result));
    TEST_ASSERT_EQUAL_INT(19,result);
    cborAdvanceToEnd(&listeningTcp);
    cbor_value_leave_container(&metrics,&listeningTcp);

    //UDP Ports
    CborValue listeningudp, udpPorts;
    TEST_ASSERT_TRUE(cborStringAssert("listening_udp_ports",&metrics));
    cbor_value_advance(&metrics);
    cbor_value_enter_container(&metrics,&listeningudp);
    TEST_ASSERT_TRUE(cborStringAssert("ports",&listeningudp));
    cbor_value_advance(&listeningudp);
    TEST_ASSERT_TRUE(cbor_value_is_array(&listeningudp));
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_enter_container(&listeningudp,&udpPorts));
    TEST_ASSERT_TRUE(cbor_value_is_map(&udpPorts));

    TEST_ASSERT_EQUAL_INT(18,cborAdvanceToEnd(&udpPorts));
    cbor_value_leave_container(&listeningudp,&udpPorts);
    TEST_ASSERT_TRUE(cborStringAssert("total",&listeningudp));
    cbor_value_advance(&listeningudp);
    result = -1;
    TEST_ASSERT_EQUAL(CborNoError,cbor_value_get_int(&listeningudp,&result));
    TEST_ASSERT_EQUAL_INT(18,result);
    cborAdvanceToEnd(&listeningudp);
    cbor_value_leave_container(&metrics,&listeningudp);
    
    

    //advance to the end of the metrics block
    while(!cbor_value_at_end(&metrics)) {
        cbor_value_advance(&metrics);
    }
    cbor_value_leave_container(&body,&metrics);

    //advance to the end of the body
    while(!cbor_value_at_end(&body)) {
        cbor_value_advance(&body);
    }
    cbor_value_leave_container(&report,&body);

}



int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_JSONMetricsBasicStructure_LongTags);
    RUN_TEST(test_JSONMetricsBasicStructure_ShortTags);
    RUN_TEST(test_tcpPortsListJSON_LongTags);
    RUN_TEST(test_tcpPortsListJSON_ShortTags);
    RUN_TEST(test_udpPortsListJSON_LongTags);
    RUN_TEST(test_udpPortsListJSON_ShortTags);
    RUN_TEST(test_netstatsJSON_LongTags);
    RUN_TEST(test_netstatsJSON_ShortTags);
    RUN_TEST(test_tcpConnectionsJSON_LongTags);
    RUN_TEST(test_tcpConnectionsJSON_ShortTags);
    RUN_TEST(test_reportCBOR_BasicStructure_LongTags);
    RUN_TEST(test_reportCBOR_header_LongTags);
    RUN_TEST(test_reportCBOR_metrics_LongTags);
    return UNITY_END();


}