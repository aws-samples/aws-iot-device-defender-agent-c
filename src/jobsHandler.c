/*
* Copyright 2015-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "jobsHandler.h"
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#ifndef DISABLE_IOT_JOBS
#include "aws_iot_jobs_interface.h"
#include "agent_config.h"
#include "cJSON.h"
#include <string.h>

IoT_Error_t setupJobsSubscriptions(AWS_IoT_Client *client) {

    topicToSubscribeGetPending = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeNotifyNext = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeNotify = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeGetNext = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeUpdateAccepted = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeUpdateRejected = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);
    topicToSubscribeStartNext = malloc(MAX_JOB_TOPIC_LENGTH_BYTES);

    int rc;
    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, NULL, JOB_GET_PENDING_TOPIC, JOB_WILDCARD_REPLY_TYPE,
            getPendingCallbackHandler, NULL, topicToSubscribeGetPending, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_GET_PENDING_TOPIC: %d ", rc);
        return rc;
    }

    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, NULL, JOB_START_NEXT_TOPIC, JOB_REQUEST_TYPE,
            nextJobCallbackHandler, NULL, topicToSubscribeStartNext, MAX_JOB_TOPIC_LENGTH_BYTES
    );
    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_START_NEXT_TOPIC: %d ", rc);
        return rc;
    }

    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, NULL, JOB_NOTIFY_NEXT_TOPIC, JOB_REQUEST_TYPE,
            nextJobCallbackHandler, NULL, topicToSubscribeNotifyNext, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_NOTIFY_NEXT_TOPIC: %d ", rc);
        return rc;
    }

    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, NULL, JOB_NOTIFY_TOPIC, JOB_REQUEST_TYPE,
            nextJobCallbackHandler, NULL, topicToSubscribeNotify, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_NOTIFY_TOPIC: %d ", rc);
        return rc;
    }


    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, JOB_ID_NEXT, JOB_DESCRIBE_TOPIC, JOB_WILDCARD_REPLY_TYPE,
            nextJobCallbackHandler, NULL, topicToSubscribeGetNext, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_DESCRIBE_TOPIC ($next): %d ", rc);
        return rc;
    }

    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, JOB_ID_WILDCARD, JOB_UPDATE_TOPIC, JOB_ACCEPTED_REPLY_TYPE,
            jobUpdateAcceptedCallbackHandler, NULL, topicToSubscribeUpdateAccepted, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_UPDATE_TOPIC/accepted: %d ", rc);
        return rc;
    }

    rc = aws_iot_jobs_subscribe_to_job_messages(
            client, QOS0, AWS_IOT_MY_THING_NAME, JOB_ID_WILDCARD, JOB_UPDATE_TOPIC, JOB_REJECTED_REPLY_TYPE,
            jobUpdateRejectedCallbackHandler, NULL, topicToSubscribeUpdateRejected, MAX_JOB_TOPIC_LENGTH_BYTES);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing JOB_UPDATE_TOPIC/rejected: %d ", rc);
        return rc;
    }
    checkForNewJobs(client);
    return rc;

}

void checkForNewJobs(AWS_IoT_Client *client) {

    char topicToPublishGetNext[MAX_JOB_TOPIC_LENGTH_BYTES];
    int rc = 0;

    AwsIotDescribeJobExecutionRequest describeRequest;
    describeRequest.executionNumber = 0;
    describeRequest.includeJobDocument = true;
    describeRequest.clientToken = NULL;

    rc = aws_iot_jobs_describe(client, QOS0, AWS_IOT_MY_THING_NAME, JOB_ID_NEXT, &describeRequest,
                               topicToPublishGetNext, sizeof(topicToPublishGetNext), NULL, 0);
}

void nextJobCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                            IoT_Publish_Message_Params *params, void *pData) {
    char topicToPublishUpdate[MAX_JOB_TOPIC_LENGTH_BYTES];
    char messageBuffer[200];
    const cJSON *execution;
    const cJSON *jobidElement;
    const cJSON *jobDocElement;
    const cJSON *agentParamsElement;

    char jobid[MAX_SIZE_OF_JOB_ID];

    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("\nJOB_NOTIFY_NEXT_TOPIC / JOB_DESCRIBE_TOPIC($next) callback");
    IOT_INFO("topic: %.*s", topicNameLen, topicName);
    IOT_INFO("payload: %.*s", (int) params->payloadLen, (char *) params->payload);

    AwsIotJobExecutionUpdateRequest updateRequest;
    cJSON *payload = cJSON_Parse(params->payload);

    if (payload != NULL && cJSON_IsObject(payload)) {
        execution = cJSON_GetObjectItem(payload, "execution");

        if (execution == NULL) {
            cJSON_Delete(payload);
            return;
        }
        jobidElement = cJSON_GetObjectItem(execution, "jobId");

        if (cJSON_IsString(jobidElement)) {
            strncpy(jobid, jobidElement->valuestring, MAX_SIZE_OF_JOB_ID);
            // get the job document
            jobDocElement = cJSON_GetObjectItem(execution, "jobDocument");

            if (cJSON_IsObject(jobDocElement)) {
                agentParamsElement = cJSON_GetObjectItem(jobDocElement, "agent_parameters");

                if (cJSON_IsObject(agentParamsElement)) {
                    int interval = cJSON_GetObjectItem(agentParamsElement, "report_interval_seconds")->valueint;

                    PUBLISH_INTERVAL = interval;
                    //TODO Set the interval
                    updateRequest.status = JOB_EXECUTION_SUCCEEDED;
                    updateRequest.statusDetails = "{\"result\": \"success\"}";
                }
            } else {
                updateRequest.status = JOB_EXECUTION_FAILED;
                updateRequest.statusDetails = "{\"failureDetail\":\"Unable to process job document, could not find agent_parameters element\"}";
            }
        } else {
            updateRequest.status = JOB_EXECUTION_FAILED;
            updateRequest.statusDetails = "{\"failureDetail\":\"Unable to find job document\"}";
        }

    } else {
        printf("Error Parsing Jobs Document Payload");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error before: %s\n", error_ptr);
        }
        updateRequest.status = JOB_EXECUTION_FAILED;
        updateRequest.statusDetails = "{\"failureDetail\":\"Unable to process job document\"}";
    }

    updateRequest.expectedVersion = 0;
    updateRequest.executionNumber = 0;
    updateRequest.includeJobExecutionState = false;
    updateRequest.includeJobDocument = false;
    updateRequest.clientToken = NULL;

    //Send the update
    aws_iot_jobs_send_update(pClient, QOS0, AWS_IOT_MY_THING_NAME, jobid, &updateRequest,
                             topicToPublishUpdate, sizeof(topicToPublishUpdate), messageBuffer, sizeof(messageBuffer));
    cJSON_Delete(payload);
}

void jobUpdateAcceptedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("\nJOB_UPDATE_TOPIC / accepted callback");
    IOT_INFO("topic: %.*s", topicNameLen, topicName);
    IOT_INFO("payload: %.*s", (int) params->payloadLen, (char *) params->payload);
}

void jobUpdateRejectedCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                      IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("\nJOB_UPDATE_TOPIC / rejected callback");
    IOT_INFO("topic: %.*s", topicNameLen, topicName);
    IOT_INFO("payload: %.*s", (int) params->payloadLen, (char *) params->payload);

    /* Do error handling here for when the update was rejected */
}

void getPendingCallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                               IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("\nJOB_UPDATE_TOPIC / rejected callback");
    IOT_INFO("topic: %.*s", topicNameLen, topicName);
    IOT_INFO("payload: %.*s", (int) params->payloadLen, (char *) params->payload);

}

void cleanUpJobSubscriptions(AWS_IoT_Client *client) {
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeGetPending);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeNotifyNext);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeNotify);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeGetNext);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeUpdateAccepted);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeUpdateRejected);
    aws_iot_jobs_unsubscribe_from_job_messages(client, topicToSubscribeStartNext);

    free(topicToSubscribeGetPending);
    free(topicToSubscribeNotifyNext);
    free(topicToSubscribeNotify);
    free(topicToSubscribeGetNext);
    free(topicToSubscribeUpdateAccepted);
    free(topicToSubscribeUpdateRejected);
    free(topicToSubscribeStartNext);
}
#endif