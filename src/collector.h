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


#ifndef COLLECTOR_H
#define COLLECTOR_H

#ifdef COLLECTOR_TEST
#define PROC_NET_DEV "../test/data/proc_dev"
#define PROC_NET_TCP "../test/data/proc_tcp"
#define PROC_NET_UDP "../test/data/proc_udp"
#else
#define PROC_NET_DEV "/proc/net/dev"
#define PROC_NET_TCP "/proc/net/tcp"
#define PROC_NET_UDP "/proc/net/udp"
#endif

#include "metrics.h"

/**
 * Gather aggregate network stats at the interface level, these include total Bytes/Packets In/Out.\n
 * On a Linux system this information is contained in <i>/proc/net/dev</i>
 *
 * @param [in] path File to read that contains the network information
 * @param [out] stats Network stats object to populate with
 */
void getNetworkStats(const char *path, NetworkStats *stats);

/**
 * Retrieve a list of all TCP connections currently tracked by the system. \n
 * On Linux this list is maintained at <i>/proc/net/tcp</i> \n
 * <b>Note:</b> This function does not allocate memory, caller must supply a fully-allocated array of structs
 *
 * @param [in] path File to read that contains the tcp connection list
 * @param [out] connections Array of pre-allocated NetworkConnection structs to fill with connection information
 * @param [out] numConnections Number of connections parsed
 */
void getAllTCPConnections(const char *path, NetworkConnection *connections, int *numConnections);


/**
 * Retrieve a list of all listening UDP ports currently tracked by the system.  \n
 * On Linux this list is maintained at <i>/proc/net/udp</i>  Connections object is used here, however it is a bit of a
 * misnomer, as UDP is a connectionless protocol\n
 * <b>Note:</b> This function does not allocate memory, caller must supply a fully-allocated array of structs
 *
 * @param [in] path File to read that contains the UDP listeners list
 * @param [out] connections Connections array of pre-allocated NetworkConnection structs
 * @param [out] numConnections Number of listening ports
 */
void getAllListeningUDPPorts(const char *path, NetworkConnection *connections, int *numConnections);


/**
 * Utility function to read a file into an array of strings, with each line of the file reprsented as a string. \n
 *
 *  <b>Note:</b> this function allocates memory on the heap, the caller is responsible for
 *  deallocating the contents of the buffer
 *
 * @param [in] path File to read
 * @param [out] buffer String array to hold contents of the file
 * @param [in] bufferSize Number of lines read from the file
 * @return
 */
int readFile(const char *path, char *buffer[], const int bufferSize);

/**
 * Parses <i>/proc/net/dev</i> contents and extracts aggregate network stats.\n
 *
 * <b>Note:</b> This function does not allocate memory, caller must supply a fully-allocated NetworkStats struct
 *
 * @param [in] fileContents Array of strings holding file contents
 * @param [in] fileLines Size of the fileContents buffer
 * @param [out] stats NetworkStats structure to hold parsed values
 */
void parseNetDev(char **fileContents, int fileLines, NetworkStats *stats);


/**
 * Parse protocol-specific information from <i>/proc/net/[tcp|udp]</i> \n
 *
 * <b>Note:</b> This function does not allocate memory, caller must supply a fully-allocated array of structs
 *
 * @param [in] fileContents Array of strings holding file contents
 * @param [in] fileLines Size of the fileContents buffer
 * @param [out] connections
 * @param [out] numConnections
 */
void parseNetProtocol(char **fileContents, int fileLines, NetworkConnection *connections, int *numConnections);

/**
 * Convert hexadecimal representation of an IP address to numbers-and-dots notation string.
 *
 * @param [in] hexAddr Address as hexadecimal, as parsed from <i>/proc</i>
 * @param [out] ipStr String to hold numbers-and-dots notation address
 * @param [in] ipStrLength Length of the IP String
 */
void hexAddrToIpStr(const char *hexAddr, char ipStr[], const int ipStrLength);

/**
 * Convert hex port number to integer port number
 *
 * @param [in] hexPort Hex port string
 * @param [out] portStr Integer port number string
 * @param [in] bufLength Length of supplied buffer
 */
void hexPortToTcpPort(const char *hexPort, char portStr[], const int bufLength);

/**
 * Takes list of connections and returns a list of copies of the connection with the supplied state. \n
 *
 * * <b>Note:</b> This function does not allocate memory, caller must supply a fully-allocated array of NetworkConnections
 *
 * @param [in] status Status to filter on
 * @param [in] allConnections Array of NetworkConnections to filter, this list is not modified
 * @param [in] allConnectionCount
 * @param [out] inState Array containing copies of connections of desired status
 * @param [out] inStateCount Number of NetworkConnections returned
 */
void
filterTCPConnectionsByState(enum state status, const NetworkConnection allConnections[], const int allConnectionCount,
                            NetworkConnection inState[], int *inStateCount);

/**
 * Generate a AWS IoT Device Defender Metrics report, using short or long field names. \name
 * 
 * <b>Note:</b> This function does not allocate memory, caller must supply an allocated string to hold report
 *
 * @param [in] reportFormat
 * @param [out] reportBuffer String to hold full report
 * @param [in] reportBufferSize Maximum length of the final report
 * @param [out] stats Network stats struct
 * @param [in] tagLen Use Long or Short names
 */
void generateMetricsReport(char *reportBuffer, const int reportBufferSize, int *reportSize, NetworkStats *stats, enum tagType tagLen,
                           enum format reportFormat);

/**
 * @brief Compare two NetworkConnection structs, for use in qsort function
 */
int compare_connections(const void *a, const void *b);

/**
 * Generates a list of unique connections from the supplied list of connections.
 *
 * <b>Note:</b> This function does not allocate memory, caller must supply an allocated array of NetworkConnections
 *
 * @param [in,out] connections Source list to filter. <b> Order of elements the array may be modfied</b>
 * @param [in] itemCount Number of connections in full connection list
 * @param [out] filtered Copies of unique Connections
 * @param [out] filteredCount Number of unique connections
 */
void filterDuplicateConnections(NetworkConnection connections[], const int itemCount,
                                NetworkConnection filtered[], int *filteredCount);

/**
 * Generates a smaller array of NetworkConnections from a larger one.
 *
 * @param [in] connections All connections
 * @param [in] itemCount Size of the connections array
 * @param [out] sampled  Randomly sampled items from Connections array
 * @param [out] sampledCount Size of the Sampled list, may not be the same as Sample Size
 * @param [in] sampleSize Desired number of elements to sample
 */
void sampleConnectionList(const NetworkConnection connections[], const int itemCount,
                          NetworkConnection sampled[], int *sampledCount, const int sampleSize);


#endif /* COLLECTOR_H */


