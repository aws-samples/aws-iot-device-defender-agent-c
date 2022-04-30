## AWS IoT Device Defender Agent C {#mainpage}

Example C implementation of a AWS IoT Device Defender metrics collection agent, and other AWS IoT Device Defender C samples.

## License

This library is licensed under the Apache 2.0 License.

## Overview

Example implementation of a Device Defender metrics collection agent, and other Device Defender Python samples. This code is targetted at a
linux environment to illustrate the basic requirements of a Device Defender agent.

## Getting Started

### Prerequisites

* Understand the AWS IoT platform and create the necessary certificates and policies.
For more information on the AWS IoT platform please visit the [AWS IoT developer guide](https://docs.aws.amazon.com/iot/latest/developerguide/iot-security-identity.html_).
* You have [created a thing](https://docs.aws.amazon.com/iot/latest/developerguide/register-device.html) and associated certificates through your AWS IoT Console
* Ensure the certificate has an [attached policy](https://docs.aws.amazon.com/iot/latest/developerguide/attach-policy-to-certificate.html) which allows the proper permissions for AWS IoT
* Install [CMake](https://cmake.org/) on your development computer

### Quick Start

#### Get dependencies using bootstrap script

1. Clone the Device Defender C Agent Repository
```
git clone https://github.com/aws-samples/aws-iot-device-defender-agent-c.git
```
2. Run the bootstrap script to download dependencies to their proper locations
   ```
   cd scripts
   ./bootstrap.sh
   ```
3. This program also uses zlib, a data compression library. To install,
   ```
   wget http://www.zlib.net/zlib-1.2.12.tar.gz
   tar -xvzf zlib-1.2.12.tar.gz
   cd zlib-1.2.12
   ./configure --prefix=/usr/local/zlib
   make install
   ```
   or use a package manager.

#### Configure the SDK with your device parameters

1. [Download your device certificates](https://docs.aws.amazon.com/iot/latest/developerguide/create-device-certificate.html) and place them in the _certs_ directory. The directory should look something like this
   ```
   certs/
     ├── cert.pem
     ├── privkey.pem
     ├── README.txt
     └── rootCA.crt
    ```
2. Edit the [aws_iot_config.h](include/aws_iot_config.h) file so that the values for the following match your certificates, this is required for the AWS IoT Device SDK:
   * __AWS_IOT_MQTT_HOST__ - Your custom endpoint
   * __AWS_IOT_MQTT_CLIENT_ID__ - This should match your thing name
   * __AWS_IOT_MY_THING_NAME__ - Also should be your thing name
   * __AWS_IOT_ROOT_CA_FILENAME__ - File name of your root certificate authority file [see this](https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html) for more information
   * __AWS_IOT_CERTIFICATE_FILENAME__ - File name of your device certificate
   * __AWS_IOT_PRIVATE_KEY_FILENAME__ - File name of your certificate private key

Given the certificate files named as illustrated in the previous step, and a thing named __MY_THING__ your *aws_iot_config.h* would contain a block that looks something like this:
```
// Get from console
// =================================================
#define AWS_IOT_MQTT_HOST              "YOUR_ENDPOINT_HERE" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define AWS_IOT_MQTT_PORT              8883 ///< default port for MQTT/S
#define AWS_IOT_MY_THING_NAME          "MY_THING" ///< Thing Name of the Shadow this device is associated with
#define AWS_IOT_MQTT_CLIENT_ID         AWS_IOT_MY_THING_NAME  //For device defender, client id should be thing name
#define AWS_IOT_ROOT_CA_FILENAME       "rootCA.crt" ///< Root CA file name
#define AWS_IOT_CERTIFICATE_FILENAME   "cert.pem" ///< device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "privkey.pem" ///< Device private key filename
// =================================================

```

#### Build the Code

This agent builds using [CMake](https://cmake.org/), which provides a simple platform-independent config file.
1. Make a build directory and change to it
   ```
   cd ..
   mkdir build
   cd build
   ```
2. Run CMake to generate the necessary make files. This will setup an out of source build.
   ```
   cmake ..
   ```
3. Compile
   ```
   make agent
   ```

#### Run the agent

1. Start the agent

   ```
   ./agent
   ```

## Generate API Documentation

1. Install [Doxygen](http://www.doxygen.nl/manual/install.html)
2. From the repository's root directory, run Doxygen to generate the api documentation
   ```
   doxygen
   ```
3. The API documentation will be generated in html format and stored in *docs/generated/html*
4. To view the documentation, open *docs/generated/html/index.html* with a browser

## IoT Jobs Integration

You can use [AWS IoT Jobs](https://docs.aws.amazon.com/iot/latest/developerguide/iot-jobs.html) to
set the metrics reporting interval of your agent. Below you will find a
sample jobs document that sets the reporting interval to 600 seconds.

```json
{
  "agent_parameters" : {
    "report_interval_seconds" : 600
  }
}
```

To use the above jobs, file you can follow the steps as outlined in [Creating and Managing Jobs](https://docs.aws.amazon.com/iot/latest/developerguide/manage-job-console.html). **When following these steps, you can skip the steps related to code-signing.


### Disabling Jobs at runtime

If you wish to disable IoT Jobs functionality at runtime you can pass the "-j" argument to the agent.
By passing this argument, the agent will not setup the necessary jobs MQTT subscriptions or check for new
jobs. By default, jobs integration is enabled.

```
agent -j
```
