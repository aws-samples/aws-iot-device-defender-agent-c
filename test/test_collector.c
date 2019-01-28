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
#include "unity.h"

#include "../src/collector.h"
#include "../src/metrics.h"

#include "stdlib.h"
#include "string.h"
#include "cJSON.h"

void test_parseNetDevOneInterface(void) {
    int DUMMY_FILE_LINES = 4;

    char *fileContents[DUMMY_FILE_LINES];
    for(int r = 0; r < DUMMY_FILE_LINES; r++){
        fileContents[r] = malloc(1000);
    }

    fileContents[0] = strcpy(fileContents[0],"Inter-|   Receive                                                |  Transmit\n\0");
    fileContents[1] = strcpy(fileContents[1]," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n\0");
    fileContents[2] = strcpy(fileContents[2],"    lo: 128579792  492217    0    0    0     0          0         0 128579792  492217    0    0    0     0       0          0\n\0");
    fileContents[3] = strcpy(fileContents[3],"  eno1: 1 2    0    0    0     0          0    1 3 4    0    0    0     0       0          0\n\0");

    NetworkStats stats;
    parseNetDev(fileContents, DUMMY_FILE_LINES, &stats);


    TEST_ASSERT_EQUAL(1,stats.bytesIn);
    TEST_ASSERT_EQUAL(2,stats.packetsIn);
    TEST_ASSERT_EQUAL(3,stats.bytesOut);
    TEST_ASSERT_EQUAL(4,stats.packetsOut);

    for(int r = 0; r < DUMMY_FILE_LINES; r++){
        free(fileContents[r]);
    }

    return;
}

void test_parseNetDevTwoInterfaces(void) {

    int DUMMY_FILE_LINES = 5;

    char *fileContents[DUMMY_FILE_LINES];
    for(int r = 0; r < DUMMY_FILE_LINES; r++){
        fileContents[r] = malloc(1000);
    }

    fileContents[0] = strcpy(fileContents[0],"Inter-|   Receive                                                |  Transmit\n\0");
    fileContents[1] = strcpy(fileContents[1]," face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n\0");
    fileContents[2] = strcpy(fileContents[2],"    lo: 128579792  492217    0    0    0     0          0         0 128579792  492217    0    0    0     0       0          0\n\0");
    fileContents[3] = strcpy(fileContents[3],"  eno1: 1 2    0    0    0     0          0    1 3 4    0    0    0     0       0          0\n\0");
    fileContents[4] = strcpy(fileContents[4],"  eno2: 1 2    0    0    0     0          0    1 3 4    0    0    0     0       0          0\n\0");

    NetworkStats stats;
    parseNetDev(fileContents, DUMMY_FILE_LINES, &stats);

    TEST_ASSERT_EQUAL(2,stats.bytesIn);
    TEST_ASSERT_EQUAL(4,stats.packetsIn);
    TEST_ASSERT_EQUAL(6,stats.bytesOut);
    TEST_ASSERT_EQUAL(8,stats.packetsOut);

     for(int r = 0; r < DUMMY_FILE_LINES; r++){
         free(fileContents[r]);
    }

    return;
}

void test_getNetworkStatsBasic(void) {
    NetworkStats stats;

    stats.bytesOut = 0;
    stats.bytesIn = 0;
    stats.packetsOut = 0;
    stats.packetsIn = 0;

    getNetworkStats("../test/data/proc_dev",&stats);
    TEST_ASSERT_EQUAL(35977584,stats.bytesIn);
    TEST_ASSERT_EQUAL(178326,stats.packetsIn);
    TEST_ASSERT_EQUAL(35977584,stats.bytesOut);
    TEST_ASSERT_EQUAL(178326,stats.packetsOut);


}

void test_parseTCPConnectionsBasic(void) {
    int DUMMY_FILE_LINES = 2;

    char *fileContents[DUMMY_FILE_LINES];
    for(int r = 0; r < DUMMY_FILE_LINES; r++){
        fileContents[r] = malloc(500);
    }

    fileContents[0] = strcpy(fileContents[0],"sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode                                                     \n\0");
    fileContents[1] = strcpy(fileContents[1],"    1: 00000000:170C     11111111:0000 0A 00000000:00000000 00:00000000 00000000 1420238916        0 46115 1 0000000000000000 100 0 0 10 0                 \n\0");

    NetworkConnection connList[DUMMY_FILE_LINES];
    int connCount = 0;
    parseNetProtocol(fileContents, DUMMY_FILE_LINES, connList, &connCount);

    TEST_ASSERT_NOT_NULL(connList);
    TEST_ASSERT_EQUAL_INT(1,connCount);

    TEST_ASSERT_EQUAL_STRING("0.0.0.0",connList[0].localAddress);
    TEST_ASSERT_EQUAL_STRING("5900",connList[0].localPort);
    TEST_ASSERT_EQUAL_STRING("17.17.17.17",connList[0].remoteAddress);
    TEST_ASSERT_EQUAL(connList[0].connectionState,LISTEN);


     for(int r = 0; r < DUMMY_FILE_LINES; r++){
        free(fileContents[r]);
    }

}

void test_connectionsDedup(void) {
    int DUMMY_FILE_LINES = 3;

    char *fileContents[DUMMY_FILE_LINES];
    for(int r = 0; r < DUMMY_FILE_LINES; r++){
        fileContents[r] = malloc(500);
    }

    fileContents[0] = strcpy(fileContents[0],"sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode                                                     \n\0");
    fileContents[1] = strcpy(fileContents[1],"    1: 00000000:170C     0100007F:0000 0A 00000000:00000000 00:00000000 00000000 1420238916        0 46115 1 0000000000000000 100 0 0 10 0                 \n\0");
    fileContents[2] = strcpy(fileContents[2],"    2: 00000000:170C     0100007F:0000 0A 00000000:00000000 00:00000000 00000000 1420238916        0 46115 1 0000000000000000 100 0 0 10 0                 \n\0");

    NetworkConnection connList[DUMMY_FILE_LINES];
    int connCount = 0;
    parseNetProtocol(fileContents, DUMMY_FILE_LINES, connList, &connCount);

    NetworkConnection deduped[DUMMY_FILE_LINES];
    int uniqueConns = 0;
    filterDuplicateConnections(connList,connCount,deduped,&uniqueConns);

    TEST_ASSERT_NOT_NULL(connList);
    TEST_ASSERT_NOT_NULL(deduped)
    TEST_ASSERT_EQUAL_INT(2,connCount);
    TEST_ASSERT_EQUAL_INT(1,uniqueConns);

    TEST_ASSERT_EQUAL_STRING("0.0.0.0",connList[0].localAddress);
    TEST_ASSERT_EQUAL_STRING("5900",connList[0].localPort);
    TEST_ASSERT_EQUAL_STRING("127.0.0.1",connList[0].remoteAddress);
    TEST_ASSERT_EQUAL(connList[0].connectionState,LISTEN);

     for(int i=0; i< DUMMY_FILE_LINES; i++) {
        free(fileContents[i]);
    }
}


void test_uniqueConnections() {

    NetworkConnection allConns[] = {
            { "A","111", "111","999","999",ESTABLISHED},
            { "A","111", "111","999","999",ESTABLISHED}
    };

    NetworkConnection unique[2];
    int uniqueCount = 0;
    filterDuplicateConnections(allConns,2,unique,&uniqueCount);
    TEST_ASSERT_EQUAL_INT(1,uniqueCount);

    NetworkConnection allConnsNulls[] = {
            { "A","111", "111","999","999",ESTABLISHED},
            { "A","111", "111","999"}
    };
    uniqueCount = 0;
    filterDuplicateConnections(allConnsNulls,2,unique,&uniqueCount);
    TEST_ASSERT_EQUAL_INT(2,uniqueCount);


    NetworkConnection emptyConns[] = {
            { "A","111", "111","999","999",ESTABLISHED},
            { "A","", "111","999","999",ESTABLISHED},
            { "","111", "111","999","999",ESTABLISHED}
    };
    uniqueCount = 0;
    filterDuplicateConnections(emptyConns,3,unique,&uniqueCount);
    TEST_ASSERT_EQUAL_INT(3,uniqueCount);

}


void test_hexStringToIpString(void) {
    char ipStr[MAX_IP_ADDR_STRING_LENGTH];
    hexAddrToIpStr("6BA44E0A",ipStr,MAX_IP_ADDR_STRING_LENGTH);
    TEST_ASSERT_EQUAL_STRING("10.78.164.107",ipStr);
 }

void test_hexPortToTcpPort(void) {

    char portStr[MAX_PORT_STRING_LENGTH];
    hexPortToTcpPort("0000",portStr,MAX_PORT_STRING_LENGTH);
    TEST_ASSERT_EQUAL_STRING("0",portStr);

    hexPortToTcpPort("076C",portStr,MAX_PORT_STRING_LENGTH);
    TEST_ASSERT_EQUAL_STRING("1900",portStr);

    hexPortToTcpPort("E828",portStr,MAX_PORT_STRING_LENGTH);
    TEST_ASSERT_EQUAL_STRING("59432",portStr);

    hexPortToTcpPort("ZZXXYY",portStr,MAX_PORT_STRING_LENGTH);
    TEST_ASSERT_EQUAL_STRING("0",portStr);
 }

void test_getTCPConnections(void) {

    NetworkConnection connections[50];
    int numConnections = 0;
    getAllTCPConnections("../test/data/proc_tcp", connections, &numConnections);

    TEST_ASSERT_EQUAL(28,numConnections);
}

void test_readFile(void) {
    char *fileContents[50];
    int fileLines = 0;

    //Get file contents as a string array
    fileLines = readFile("../test/data/proc_tcp", fileContents, 50);
    TEST_ASSERT_EQUAL(29, fileLines);

    for(int i=0; i< fileLines; i++) {
        free(fileContents[i]);
    }
}


void test_getUDPConnectionsBasic(void) {

    NetworkConnection connections[50];
    int numConnections = 0;
    getAllListeningUDPPorts("../test/data/proc_udp", connections, &numConnections);

    TEST_ASSERT_EQUAL(18,numConnections);

    //UDP is connectionless, so remote port and address are always 0
    bool found = false;
    for(int i = 0; i < numConnections; i++) {
       TEST_ASSERT_EQUAL_STRING("0.0.0.0",connections[i].remoteAddress);
       TEST_ASSERT_EQUAL_STRING("0",connections[i].remotePort);

       if(strcmp("10.78.166.53",connections[i].localAddress) == 0) {
           if(strcmp("45775",connections[i].localPort) == 0) {
               found = true;
           }
       }
    }
    TEST_ASSERT_TRUE(found);
}

void test_filterConnections(void) {

    NetworkConnection connections[50];
    int numConnections = 0;
    getAllTCPConnections("../test/data/proc_tcp", connections, &numConnections);

     //Filter for only ESTABLISHED TCP Connections
    NetworkConnection establishedConnections[50];
    int establishedCount = 0;
    filterTCPConnectionsByState(ESTABLISHED,connections,numConnections,establishedConnections,&establishedCount);

    TEST_ASSERT_EQUAL(4,establishedCount);

    NetworkConnection listeningConnections[50];
    int listeningCount = 0;
    filterTCPConnectionsByState(LISTEN,connections,numConnections,listeningConnections,&listeningCount);

    TEST_ASSERT_EQUAL(19,listeningCount);
}



void test_sampleList(void) {

    NetworkConnection allConns[] = {
            { "A","111", "111","999","999",ESTABLISHED},
            { "B","111", "111","999","999",ESTABLISHED},
            { "C","111", "111","999","999",ESTABLISHED},
            { "D","111", "111","999","999",ESTABLISHED},
            { "E","111", "111","999","999",ESTABLISHED}
    };

    NetworkConnection sampled[2];
    int sampleCount = 0;
    sampleConnectionList(allConns,5,sampled,&sampleCount,2);
    TEST_ASSERT_EQUAL_INT(2,sampleCount);

    sampleCount = 0;
    sampleConnectionList(allConns,5,sampled,&sampleCount,6);
    TEST_ASSERT_EQUAL_INT(5,sampleCount);

}
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parseNetDevOneInterface);
    RUN_TEST(test_parseNetDevTwoInterfaces);
    RUN_TEST(test_parseTCPConnectionsBasic);
    RUN_TEST(test_connectionsDedup);
    RUN_TEST(test_hexStringToIpString);
    RUN_TEST(test_hexPortToTcpPort);
    RUN_TEST(test_readFile);
    RUN_TEST(test_getTCPConnections);
    RUN_TEST(test_getUDPConnectionsBasic);
    RUN_TEST(test_filterConnections);
    RUN_TEST(test_getNetworkStatsBasic);
    RUN_TEST(test_uniqueConnections);
    RUN_TEST(test_sampleList);

    return UNITY_END();


}