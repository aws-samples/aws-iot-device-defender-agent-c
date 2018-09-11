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


IoT_Error_t setupJobsSubscriptions(AWS_IoT_Client *client);

void cleanUpJobSubscriptions(AWS_IoT_Client *client);

void checkForNewJobs(AWS_IoT_Client *client);

void getPendingCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                               IoT_Publish_Message_Params *params, void *pData);

void nextJobCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                            IoT_Publish_Message_Params *params, void *pData);

void jobUpdateAcceptedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData);

void jobUpdateRejectedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData);


#endif //AWSIOTDEVICEDEFENDERAGENT_JOBSHANDLER_H
