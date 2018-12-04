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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>

#include "collector.h"
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "agent.h"
#include "agent_config.h"
#include "jobsHandler.h"

int PUBLISH_INTERVAL = 301;
enum format REPORT_FORMAT = JSON;
enum tagType TAG_LENGTH = LONG_NAMES;
bool DISABLE_JOBS = false;

void subscriptionCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                 IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("Subscribe callback");
    IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    IOT_WARN("MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if (NULL == pClient) {
        return;
    }

    IOT_UNUSED(data);

    if (aws_iot_is_autoreconnect_enabled(pClient)) {
        IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if (NETWORK_RECONNECTED == rc) {
            IOT_WARN("Manual Reconnect Successful");
        } else {
            IOT_WARN("Manual Reconnect Failed - %d", rc);
        }
    }
}

void parseInputArgs(int argc, char **argv) {
    int opt;

    while (-1 != (opt = getopt(argc, argv, "h:p:c:x:f:sj"))) {
        switch (opt) {
            case 'h':
                strncpy(HostAddress, optarg, HOST_ADDRESS_SIZE);
                IOT_DEBUG("Host %s", optarg);
                break;
            case 'p':
                port = atoi(optarg);
                IOT_DEBUG("arg %s", optarg);
                break;
            case 'c':
                strncpy(certDirectory, optarg, PATH_MAX + 1);
                IOT_DEBUG("cert root directory %s", optarg);
                break;
            case 'x':
                publishCount = atoi(optarg);
                IOT_DEBUG("publish %s times\n", optarg);
                break;
            case 'f':
                if (strcmp("cbor", optarg) == 0) {
                    REPORT_FORMAT = CBOR;
                }
                break;
            case 's':
                TAG_LENGTH = SHORT_NAMES;
                break;
            case 'j' :
                IOT_DEBUG("Disable IoT Jobs Functions")
                DISABLE_JOBS = true;
                break;
            case '?':
                if (optopt == 'c') {
                    IOT_ERROR("Option -%c requires an argument.", optopt);
                } else if (isprint(optopt)) {
                    IOT_WARN("Unknown option `-%c'.", optopt);
                } else {
                    IOT_WARN("Unknown option character `\\x%x'.", optopt);
                }
                break;
            default: IOT_ERROR("Error in command line argument parsing");
                break;
        }
    }

}


int main(int argc, char *argv[]) {
    bool infinitePublishFlag = true;

    char rootCA[PATH_MAX + 1];
    char clientCRT[PATH_MAX + 1];
    char clientKey[PATH_MAX + 1];
    char CurrentWD[PATH_MAX + 1];
    char cPayload[MAX_MESSAGE_SIZE_BYTES];

    char publishTopic[MAX_TOPIC_LENGTH];
    char subscribeAcceptedTopic[MAX_TOPIC_LENGTH];
    char subscribeRejectedTopic[MAX_TOPIC_LENGTH];


    int32_t i = 0;

    IoT_Error_t rc = FAILURE;

    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IoT_Publish_Message_Params paramsQOS0;

    parseInputArgs(argc, argv);

    if (REPORT_FORMAT == JSON) {
        snprintf(publishTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/json", AWS_IOT_MY_THING_NAME);
        snprintf(subscribeAcceptedTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/json/accepted",
                 AWS_IOT_MY_THING_NAME);
        snprintf(subscribeRejectedTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/json/rejected",
                 AWS_IOT_MY_THING_NAME);
    } else if (REPORT_FORMAT == CBOR) {
        snprintf(publishTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/cbor", AWS_IOT_MY_THING_NAME);
        snprintf(subscribeAcceptedTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/cbor/accepted",
                 AWS_IOT_MY_THING_NAME);
        snprintf(subscribeRejectedTopic, MAX_TOPIC_LENGTH, "$aws/things/%s/defender/metrics/cbor/rejected",
                 AWS_IOT_MY_THING_NAME);
    }

    IOT_INFO("Topics:\n Publish: %s\n Accepted: %s\n Rejected:%s", publishTopic, subscribeAcceptedTopic,
             subscribeRejectedTopic);
    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    getcwd(CurrentWD, sizeof(CurrentWD));
    snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
    snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
    snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

    IOT_DEBUG("rootCA %s", rootCA);
    IOT_DEBUG("clientCRT %s", clientCRT);
    IOT_DEBUG("clientKey %s", clientKey);
    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = HostAddress;
    mqttInitParams.port = port;
    mqttInitParams.pRootCALocation = rootCA;
    mqttInitParams.pDeviceCertLocation = clientCRT;
    mqttInitParams.pDevicePrivateKeyLocation = clientKey;
    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if (SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
    }

    connectParams.keepAliveIntervalInSec = 600;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = AWS_IOT_MY_THING_NAME;  //clientid should be same as thingname to get most benefit from device defender
    connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    IOT_INFO("Connecting...");
    rc = aws_iot_mqtt_connect(&client, &connectParams);
    if (SUCCESS != rc) {
        IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return rc;
    }

    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if (SUCCESS != rc) {
        IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
        return rc;
    }

    IOT_INFO("Subscribing...");
    rc = aws_iot_mqtt_subscribe(&client, subscribeAcceptedTopic, strlen(subscribeAcceptedTopic), QOS0,
                                subscriptionCallbackHandler, NULL);
    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing : %d ", rc);
        return rc;
    }

    rc = aws_iot_mqtt_subscribe(&client, subscribeRejectedTopic, strlen(subscribeRejectedTopic), QOS0,
                                subscriptionCallbackHandler, NULL);
    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing : %d ", rc);
        return rc;
    }

    if(!DISABLE_JOBS) {
        setupJobsSubscriptions(&client);
    }
    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;


    if (publishCount != 0) {
        infinitePublishFlag = false;
    }

    char report[MAX_MESSAGE_SIZE_BYTES];
    while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)
           && (publishCount > 0 || infinitePublishFlag)) {

        if(!DISABLE_JOBS) {
            checkForNewJobs(&client);
        }
        //Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 1000);
        if (NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            printf("Network reconnecting, skipping loop");
            continue;
        }

        report[0] = '\0';
        int reportLength = -1;
        if (REPORT_FORMAT == CBOR) {
            generateMetricsReport(report, MAX_MESSAGE_SIZE_BYTES, &reportLength, TAG_LENGTH, CBOR);
        } else {
            generateMetricsReport(report, MAX_MESSAGE_SIZE_BYTES, &reportLength, TAG_LENGTH, JSON);
        }

        memcpy(cPayload, report, reportLength);
        paramsQOS0.payloadLen = reportLength;
        rc = aws_iot_mqtt_publish(&client, publishTopic, strlen(publishTopic), &paramsQOS0);
        IOT_INFO("sleep for %i seconds", PUBLISH_INTERVAL);
        sleep(PUBLISH_INTERVAL);

    }

    if (SUCCESS != rc) {
        IOT_ERROR("An error occurred in the loop.\n");
    } else {
        IOT_INFO("Publish done\n");
    }

    if(!DISABLE_JOBS) {
        cleanUpJobSubscriptions(&client);
    }

    return 0;
}
