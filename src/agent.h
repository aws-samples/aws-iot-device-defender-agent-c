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

#ifndef AWSIOTDEVICEDEFENDERAGENT_AGENT_H
#define AWSIOTDEVICEDEFENDERAGENT_AGENT_H

#include <zconf.h>
#include <aws_iot_config.h>
#include <stdint.h>
#include <aws_iot_mqtt_client.h>
#include "metrics.h"

/**
 * @brief Default cert location
 */
char certDirectory[PATH_MAX + 1] = "../certs";
/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;
/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;
/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
uint32_t publishCount = 0;

int MAX_MESSAGE_SIZE_BYTES = 128000;

void subscriptionCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                 IoT_Publish_Message_Params *params, void *pData);

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data);

void parseInputArgs(int argc, char **argv);

#endif //AWSIOTDEVICEDEFENDERAGENT_AGENT_H
