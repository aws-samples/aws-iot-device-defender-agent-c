/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <time.h>
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "arpa/inet.h"

#include "collector.h"

#define MAX_CHAR 1000
#define MAX_CONNECTIONS 500
#define MAX_FILE_LINES 500
#define MAX_LIST_ITEMS 10

// net/dev fields
#define NAME_TOK 0
#define BYTES_IN_TOK 1
#define PKTS_IN_TOK 2
#define BYTES_OUT_TOK 9
#define PKTS_OUT_TOK 10

// /net/tcp fields
#define LOCAL_ADDR_TOK 1
#define LOCAL_PORT_TOK 2
#define REMOTE_ADDR_TOK 3
#define REMOTE_PORT_TOK 4
#define STATUS_TOK 5

void getNetworkStats(const char *path, NetworkStats *stats) {

    char *fileContents[MAX_FILE_LINES];
    int fileLines = 0;

    //Get file contents as a string array
    fileLines = readFile(path, fileContents, MAX_FILE_LINES);
    if (fileLines <= 0) {
        printf("Unable to read lines from /proc/net/dev\n");
        return;
    }

    parseNetDev(fileContents, fileLines, stats);

    for (int i = 0; i < fileLines; i++) {
        free(fileContents[i]);
    }

    return;
}

void getAllTCPConnections(const char *path, NetworkConnection *connections, int *numConnections) {

    char *fileContents[MAX_CONNECTIONS];
    int fileLines = 0;
    int numAllConnections = 0;
    int numUniqueConnections = 0;

    NetworkConnection allConnections[MAX_CONNECTIONS];
    NetworkConnection uniqueConnections[MAX_CONNECTIONS];

    //Get file contents as a string array
    fileLines = readFile(path, fileContents, MAX_CONNECTIONS);
    if (fileLines <= 0) {
        printf("Unable to read lines from %s\n", path);
        return;
    }

    printf("Number of Lines in %s : %i\n", path, fileLines);

    //Get All the TCP Connections
    parseNetProtocol(fileContents, fileLines, allConnections, &numAllConnections);
    filterDuplicateConnections(allConnections, numAllConnections, uniqueConnections, &numUniqueConnections);

    *numConnections = 0;
    for (int n = 0; n < numUniqueConnections; n++) {
        memcpy(&connections[*numConnections], &uniqueConnections[n], sizeof connections[*numConnections]);
        //copyNetworkConnection(&connections[*numConnections],&uniqueConnections[n]);
        (*numConnections)++;
    }

    for (int i = 0; i < fileLines; i++) {
        free(fileContents[i]);
    }

    return;
}

void parseNetProtocol(char **fileContents, int fileLines, NetworkConnection connections[], int *numConnections) {

    *numConnections = 0;
    char *charPtr;
    char tempAddr[MAX_IP_ADDR_STRING_LENGTH];
    char tempLine[MAX_CHAR];
    int tokNum = 0;

    for (int line = 0; line < fileLines; line++) {
        if (line == 0) {
            printf("Discarding Header Line\n");
        } else {
            tokNum = 0;
            strcpy(tempLine, fileContents[line]);
            charPtr = strtok(tempLine, " :");

            connections[*numConnections].localAddress[0] = '\0';
            connections[*numConnections].localPort[0] = '\0';
            connections[*numConnections].localInterface[0] = '\0';
            connections[*numConnections].remoteAddress[0] = '\0';
            connections[*numConnections].remotePort[0] = '\0';

            while (charPtr != NULL) {
                //TODO introduce logging levels, and move the following to a TRACE level
                //printf("Token %i:%s\n",tokNum,charPtr);
                switch (tokNum) {
                    case LOCAL_ADDR_TOK:
                        hexAddrToIpStr(charPtr, tempAddr, MAX_IP_ADDR_STRING_LENGTH);
                        strcpy(connections[*numConnections].localAddress, tempAddr);
                        tempAddr[0] = '\0';
                        break;
                    case LOCAL_PORT_TOK:
                        hexPortToTcpPort(charPtr, tempAddr, MAX_IP_ADDR_STRING_LENGTH);
                        strcpy(connections[*numConnections].localPort, tempAddr);
                        tempAddr[0] = '\0';
                        break;
                    case REMOTE_ADDR_TOK:
                        hexAddrToIpStr(charPtr, tempAddr, MAX_IP_ADDR_STRING_LENGTH);
                        strcpy(connections[*numConnections].remoteAddress, tempAddr);
                        tempAddr[0] = '\0';
                        break;
                    case REMOTE_PORT_TOK:
                        hexPortToTcpPort(charPtr, tempAddr, MAX_IP_ADDR_STRING_LENGTH);
                        strcpy(connections[*numConnections].remotePort, tempAddr);
                        tempAddr[0] = '\0';
                        break;
                    case STATUS_TOK: {
                        if (strcmp("01", charPtr) == 0) {
                            connections[*numConnections].connectionState = ESTABLISHED;
                        } else if (strcmp("0A", charPtr) == 0) {
                            connections[*numConnections].connectionState = LISTEN;
                        } else {
                            connections[*numConnections].connectionState = OTHER;
                        }
                        break;
                    }
                    default :
                        break;
                }
                charPtr = strtok(NULL, " :");
                tokNum++;
            }
            (*numConnections)++;
        }
    }

    return;
}

void parseNetDev(char **fileContents, int fileLines, NetworkStats *stats) {

    //accumulators
    unsigned long bytesIn = 0;
    unsigned long bytesOut = 0;
    unsigned long packetsIn = 0;
    unsigned long packetsOut = 0;

    // Walk the file contents line by line, parsing the values we need
    for (int line = 0; line < fileLines; line++) {
        if (line <= 1) {
            printf("Discarding Header Line\n");
        } else {
            char *charPtr;
            charPtr = strtok(fileContents[line], " ,.-");
            int tokNum = 0;
            bool skip = false;

            while (charPtr != NULL && !skip) {

                unsigned long tokIntVal = strtoul(charPtr, NULL, 0);

                switch (tokNum) {
                    case NAME_TOK :
                        if (strcmp("lo:\0", charPtr) == 0) {
                            printf("Skipping %s\n", charPtr);
                            skip = true;
                        } else {
                            printf("%s", charPtr);
                        }
                        break;
                    case BYTES_IN_TOK :
                        bytesIn += tokIntVal;
                        printf("Interface BytesIn: %lu, Total BytesIn: %lu\n", tokIntVal, bytesIn);
                        break;
                    case BYTES_OUT_TOK :
                        bytesOut += tokIntVal;
                        printf("Interface BytesOut: %lu, Total BytesOut: %lu\n", tokIntVal, bytesOut);
                        break;
                    case PKTS_IN_TOK :
                        packetsIn += tokIntVal;
                        printf("Interface Packets In: %lu, Total Packets In: %lu\n", tokIntVal, packetsIn);
                        break;
                    case PKTS_OUT_TOK :
                        packetsOut += tokIntVal;
                        printf("Interface Packets Out: %lu, Total Packets Out: %lu\n", tokIntVal, packetsOut);
                        break;
                    default :
                        break;
                }

                charPtr = strtok(NULL, " ,.-");
                tokNum++;
            }
        }
    }

    (*stats).bytesIn = bytesIn;
    (*stats).bytesOut = bytesOut;
    (*stats).packetsIn = packetsIn;
    (*stats).packetsOut = packetsOut;

    return;
}


int readFile(const char *path, char *buffer[], const int bufferSize) {
    //TODO read the file and get the number of lines so we can allocate memory more accurately here
    bool done = false;
    int lines = 0;
    char temp[MAX_CHAR] = "\0"; //temp buffer to read into

    FILE *fileHandle = fopen(path, "r");
    if (fileHandle == NULL) {
        printf("Cannot open %s for reading", path);
        return 0;
    }


    while (!done && lines < bufferSize) {
        temp[0] = '\0';
        if (fgets(temp, MAX_CHAR, fileHandle) == NULL) {
            done = true;
        } else {
            buffer[lines] = malloc(MAX_CHAR);
            strcpy(buffer[lines], temp);
            lines++;
        }
        temp[0] = '\0';
    }
    fclose(fileHandle);
    return lines;
}

void hexAddrToIpStr(const char *hexAddr, char ipStr[], const int ipStrLength) {

    uint32_t addrNum = (uint32_t) strtoul(hexAddr, NULL, 16);
    struct in_addr addr;
    addr.s_addr = addrNum; // hexAddr is in network byte order already
    char *s = inet_ntoa(addr);

    snprintf(ipStr, ipStrLength, "%s", s);
}

void hexPortToTcpPort(const char *hexPort, char portStr[], const int portStrLength) {

    int port = (int) strtol(hexPort, NULL, 16);
    snprintf(portStr, portStrLength, "%i", port);
}

void getAllListeningUDPPorts(const char *path, NetworkConnection *connections, int *numConnections) {
    char *fileContents[MAX_CONNECTIONS];
    int fileLines = 0;
    int numAllUDP = 0;
    int numUniqueUDP = 0;
    NetworkConnection allUDP[MAX_CONNECTIONS];
    NetworkConnection uniqueUDP[MAX_CONNECTIONS];

    //Get file contents as a string array
    fileLines = readFile(path, fileContents, MAX_CONNECTIONS);
    if (fileLines <= 0) {
        printf("Unable to read lines from %s\n", path);
        return;
    }

    printf("Number of Lines in %s : %i\n", path, fileLines);

    parseNetProtocol(fileContents, fileLines, allUDP, &numAllUDP);
    filterDuplicateConnections(allUDP, numAllUDP, uniqueUDP, &numUniqueUDP);

    for (int n = 0; n < numUniqueUDP; n++) {
        memcpy(&connections[*numConnections], &uniqueUDP[n], sizeof(NetworkConnection));
        (*numConnections)++;
    }

    for (int i = 0; i < fileLines; i++) {
        free(fileContents[i]);
    }

    return;

}

void
filterTCPConnectionsByState(enum state status, const NetworkConnection allConnections[], const int allConnectionCount,
                            NetworkConnection inState[], int *inStateCount) {

    *inStateCount = 0;
    for (int i = 0; i < allConnectionCount; i++) {
        if (allConnections[i].connectionState == status) {
            memcpy(&inState[*inStateCount], &allConnections[i], sizeof(allConnections[i]));
            (*inStateCount)++;
        }
    }

}


void generateMetricsReport(char *reportBuffer, const int reportBufferSize, int *reportSize, enum tagType tagLen,
                           enum format reportFormat) {

    printf("Using file: %s\n", PROC_NET_DEV);

    NetworkStats stats;
    getNetworkStats(PROC_NET_DEV, &stats);

    NetworkConnection tcpConnections[MAX_CONNECTIONS];
    int tcpConnectionCount = 0;
    //First, get all the tcpConnections, will filter out what we need for report after
    getAllTCPConnections(PROC_NET_TCP, tcpConnections, &tcpConnectionCount);

    //Filter for only ESTABLISHED TCP Connections
    NetworkConnection establishedConnections[MAX_CONNECTIONS];
    int establishedCount = 0;
    filterTCPConnectionsByState(ESTABLISHED, tcpConnections, tcpConnectionCount, establishedConnections,
                                &establishedCount);

    //Filter for Listening Ports
    NetworkConnection listeningConnections[MAX_CONNECTIONS];
    int listeningCount = 0;
    filterTCPConnectionsByState(LISTEN, tcpConnections, tcpConnectionCount, listeningConnections, &listeningCount);

    NetworkConnection udpConnections[MAX_CONNECTIONS];
    int udpConnectionCount = 0;
    getAllListeningUDPPorts(PROC_NET_UDP, udpConnections, &udpConnectionCount);


    struct metrics metrics;
    metrics.listeningTCPPorts = listeningConnections;
    metrics.tcpPortCount = listeningCount;
    metrics.listeningUDPPorts = udpConnections;
    metrics.udpPortCount = udpConnectionCount;
    metrics.tcpConnections = establishedConnections;
    metrics.tcpConnectionCount = establishedCount;
    metrics.networkStats = stats;

    //TODO Generate a real report ID
    struct Header header = {12312312, "1.0"};

    struct Report report;
    report.header = header;
    report.metrics = metrics;

    printReportToConsole(&report);
    int length = 0;

    if (reportFormat == CBOR) {
        generateCBORReport(&report, reportBuffer, reportSize, tagLen);
    } else {
        generateJSONReport(&report, reportBuffer, reportSize, tagLen);
    }
}


int compare_connections(const void *a, const void *b) {

    NetworkConnection connA = *(const NetworkConnection *) a;
    NetworkConnection connB = *(const NetworkConnection *) b;

    //TODO make this size more reasonable?
    char hashA[MAX_CHAR] = "\0";
    char hashB[MAX_CHAR] = "\0";

    sprintf(hashA, "%s%s%s%s%s%i", connA.localPort, connA.localAddress, connA.localInterface, connA.remotePort,
            connA.remoteAddress, connA.connectionState);
    sprintf(hashB, "%s%s%s%s%s%i", connB.localPort, connB.localAddress, connB.localInterface, connB.remotePort,
            connB.remoteAddress, connB.connectionState);

    return (strcmp(hashA, hashB));
}


void filterDuplicateConnections(NetworkConnection connections[], const int itemCount,
                                NetworkConnection filtered[], int *filteredCount) {

    qsort(connections, itemCount, sizeof(NetworkConnection), compare_connections);
    memcpy(&filtered[0], &connections[0], sizeof(NetworkConnection));
    *filteredCount = 1;

    for (int i = 1; i < itemCount; i++) {
        if (compare_connections(&connections[i], &filtered[*filteredCount - 1]) != 0) {
            memcpy(&filtered[*filteredCount], &connections[i], sizeof(NetworkConnection));
            *filteredCount += 1;
        }
    }

    printf("Filtered %i duplicate connections", itemCount - *filteredCount);
}

void sampleConnectionList(const NetworkConnection *connections, const int itemCount,
                          NetworkConnection *sampled, int *sampleCount, const int sampleSize) {

    if (itemCount <= sampleSize) {
        for (int i = 0; i < itemCount; i++) {
            sampled[i] = connections[i];
        }
        *sampleCount = itemCount;
        return;
    }

    bool used[itemCount];
    for (int i = 0; i < itemCount; i++) {
        used[i] = false;
    }

    int randIndex = 0;
    while (*sampleCount < sampleSize) {
        //Get a random index
        srand(time(NULL)); // use current time as seed for random generator
        randIndex = rand() % itemCount;

        if (!used[randIndex]) {
            used[randIndex] = true;
            sampled[*sampleCount] = connections[randIndex];
            *sampleCount += 1;
        }

    }

}


