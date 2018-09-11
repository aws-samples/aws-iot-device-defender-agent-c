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

/**
 * @file aws_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_DD_IOT_DD_CONFIG_H_
#define SRC_DD_IOT_DD_CONFIG_H_

// Get from console
// =================================================
#define AWS_IOT_MQTT_HOST              "YOUR_ENDPOINT_HERE" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define AWS_IOT_MQTT_PORT              8883 ///< default port for MQTT/S
#define AWS_IOT_MY_THING_NAME 		   "YOUR_THING_NAME_HERE" ///< Thing Name of the Shadow this device is associated with
#define AWS_IOT_MQTT_CLIENT_ID         AWS_IOT_MY_THING_NAME  //For device defender, client id should be thing name
#define AWS_IOT_ROOT_CA_FILENAME       "rootCA.crt" ///< Root CA file name
#define AWS_IOT_CERTIFICATE_FILENAME   "cert.pem" ///< device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "privkey.pem" ///< Device private key filename
// =================================================

// MQTT PubSub
#define AWS_IOT_MQTT_TX_BUF_LEN 128000 ///< Any time a message is sent out through the MQTT layer. The message is copied into this buffer anytime a publish is done. This will also be used in the case of Thing Shadow
#define AWS_IOT_MQTT_RX_BUF_LEN 128000 ///< Any message that comes into the device should be less than this buffer size. If a received message is bigger than this buffer size the message will be dropped.
#define AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 10 ///< Maximum number of topic filters the MQTT client can handle at any given time. This should be increased appropriately when using Thing Shadow

// Auto Reconnect specific config
#define AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL 1000 ///< Minimum time before the First reconnect attempt is made as part of the exponential back-off algorithm
#define AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL 128000 ///< Maximum time interval after which exponential back-off will stop attempting to reconnect.

#define DISABLE_METRICS false ///< Disable the collection of metrics by setting this to true

// Job specific configs
#define MAX_SIZE_OF_THING_NAME 30 ///< The Thing Name should not be bigger than this value. Modify this if the Thing Name needs to be biggera
#ifndef DISABLE_IOT_JOBS
#define MAX_SIZE_OF_JOB_ID 64
#define MAX_JOB_JSON_TOKEN_EXPECTED 120
#define MAX_SIZE_OF_JOB_REQUEST AWS_IOT_MQTT_TX_BUF_LEN

#define MAX_JOB_TOPIC_LENGTH_WITHOUT_JOB_ID_OR_THING_NAME 40
#define MAX_JOB_TOPIC_LENGTH_BYTES MAX_JOB_TOPIC_LENGTH_WITHOUT_JOB_ID_OR_THING_NAME + MAX_SIZE_OF_THING_NAME + MAX_SIZE_OF_JOB_ID + 2
#endif

#endif /* SRC_DD_IOT_DD_CONFIG_H_ */
