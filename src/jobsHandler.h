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
#include "aws_iot_config.h"
#include "aws_iot_error.h"
#include "aws_iot_mqtt_client_interface.h"

#ifndef AWSIOTDEVICEDEFENDERAGENT_JOBSHANDLER_H
#define AWSIOTDEVICEDEFENDERAGENT_JOBSHANDLER_H


char *topicToSubscribeGetPending;
char *topicToSubscribeNotifyNext;
char *topicToSubscribeNotify;
char *topicToSubscribeGetNext;
char *topicToSubscribeUpdateAccepted;
char *topicToSubscribeUpdateRejected;
char *topicToSubscribeStartNext;


/**
 * @brief Helper function to setup MQTT Subscriptions needed for AWS IoT Jobs integration
 *
 * @param [in] client a properly initialized MQTT client instance
 * @return
 */
IoT_Error_t setupJobsSubscriptions(AWS_IoT_Client *client);


/**
 * @brief Remove all AWS IoT Jobs subscriptions, and frees all topic name buffers.
 * @param client
 */
void cleanUpJobSubscriptions(AWS_IoT_Client *client);

/**
 * Checks for new jobs, by publishing to a Jobs-specific MQTT Topic
 *
 * @param [in] client a properly initialized MQTT client instance
 */
void checkForNewJobs(AWS_IoT_Client *client);

/**
 * @brief Callback invoked by IoT Jobs when there are pending jobs
 *
 * @param pClient [in] AWS IoT Client
 * @param topicName [in] Topic message was published to
 * @param topicNameLen [in] Length of the topic name buffer
 * @param params [in] MQTT Message metadata
 * @param pData [in] User-defined data
 */
void getPendingCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                               IoT_Publish_Message_Params *params, void *pData);

/**
 * Callback invoked by IoT Jobs when there is a job to process. This method does the majority of the work necessary to
 * handle an IoT Job. For this agent, this included JSON parsing and setting the agent runtime parameters
 * to the values specified in the jobs document.
 *
 * @param pClient [in] AWS IoT Client
 * @param topicName [in] Topic message was published to
 * @param topicNameLen [in] Length of the topic name buffer
 * @param params [in] MQTT Message metadata
 * @param pData [in] User-defined data
 */
void nextJobCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                            IoT_Publish_Message_Params *params, void *pData);

/**
 * @brief Callback used for more advanced patterns of IoT Jobs handling.  For this implementation, simple prints
 * the published topic and returns
 *
 * @param pClient [in] AWS IoT Client
 * @param topicName [in] Topic message was published to
 * @param topicNameLen [in] Length of the topic name buffer
 * @param params [in] MQTT Message metadata
 * @param pData [in] User-defined data
 */
void jobUpdateAcceptedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData);

/**
 * @brief Callback used for more advanced patterns of IoT Jobs handling.  For this implementation, simple prints
 * the published topic and returns
 *
 * @param pClient [in] AWS IoT Client
 * @param topicName [in] Topic message was published to
 * @param topicNameLen [in] Length of the topic name buffer
 * @param params [in] MQTT Message metadata
 * @param pData [in] User-defined data
 */
void jobUpdateRejectedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData);

#endif //AWSIOTDEVICEDEFENDERAGENT_JOBSHANDLER_H
