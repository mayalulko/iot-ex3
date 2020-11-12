#include <string.h>
#include <stdio.h>
#include "serial_io.h"
#include "config.h"

byte TX_BUFFER[BUFFER_SIZE] = {0};
byte RX_BUFFER[BUFFER_SIZE] = {0};


int main(){
/**
 * init
 * wait for pbready (read)
 * AT (write)
 * AT+CCID
 * AT+COPS=?
 * AT+CREG
 * AT+COPS?
 * print the command and response
 * TODO: turn off the echo - this will affect the parsing -don't want that
 */
    int retval =  SerialInit(char* port, unsigned int baudRate);

    printf("1) Initializing MQTT client with broker %s:%d (<host>:<port>)\n", HOST, PORT);
    MQTTCtx mqttCtx;
    initMqttCtx(&mqttCtx);

    int retval = initClient(&mqttCtx);
    if (retval < 0) {
        fprintf(stderr, "\t1a) Error occurred while initializing HTTP client, aborting.\n"
                        "Error code (%d): %s\n", retval, MqttClient_ReturnCodeToString(retval));
        return retval;
    }

    printf("2) Connecting to MQTT Broker\n");
    retval = connectToBroker(&mqttCtx);

    if (retval < 0) {
        fprintf(stderr, "\t2a) Error occurred while connecting to MQTT broker, aborting\n"
                        "Error code (%d): %s\n", retval, MqttClient_ReturnCodeToString(retval));
        return retval;
    }

    printf("3) Publishing message\n");
    retval = publishMessage(&mqttCtx);
    if (retval < 0){
        fprintf(stderr, "\t3a) Error occurred while publishing message, aborting\n"
                        "Error code (%d): %s\n", retval, MqttClient_ReturnCodeToString(retval));
        return retval;
    }


    printf("4) Terminating connection with MQTT Broker\n");
    MqttClientNet_DeInit(&mqttCtx.net);

    printf("\n\nBuh bye...\n");

    return MQTT_CODE_SUCCESS;
}

static void initMqttCtx(MQTTCtx* mqttCtx) {
    XMEMSET(mqttCtx, 0, sizeof(MQTTCtx));
    mqttCtx->host = HOST;
    mqttCtx->port = PORT;
    mqttCtx->qos = MQTT_QOS_1;
    mqttCtx->clean_session = 0;
    mqttCtx->keep_alive_sec = DEFAULT_KEEP_ALIVE_SEC;
    mqttCtx->client_id = "MayOrOfFlavortown";
    mqttCtx->topic_name = TOPIC_NAME;
    mqttCtx->cmd_timeout_ms = TIMEOUT_MS;
    mqttCtx->retain = 0;
    mqttCtx->app_name = "MayOr (mayonnaise-filled Oreo)";
    mqttCtx->message = PAYLOAD;
    mqttCtx->tx_buf = TX_BUFFER;
    mqttCtx->rx_buf = RX_BUFFER;
    MqttNet network;
    XMEMSET(&network, 0, sizeof(MqttNet));
    mqttCtx->net = network;
}

static int initClient(MQTTCtx* mqttCtx) {
    MqttClient client;
    memset(&client, 0, sizeof(MqttClient));
    mqttCtx->client.ctx = &mqttCtx;
    mqttCtx->client = client;
    mqttCtx->client.cmd_timeout_ms = TIMEOUT_MS;
    mqttCtx->client.net = &(mqttCtx->net);
    mqttCtx->client.tx_buf = TX_BUFFER;
    mqttCtx->client.rx_buf = RX_BUFFER;
    mqttCtx->client.tx_buf_len = BUFFER_SIZE;
    mqttCtx->client.rx_buf_len = BUFFER_SIZE;

    return MqttClientNet_Init(mqttCtx->client.net, mqttCtx);
}

static void initConnectPacket(MQTTCtx *mqttCtx)
{
    /* Build connect packet */
    MqttConnect connect;
    XMEMSET(&connect, 0, sizeof(MqttConnect));
    mqttCtx->connect = connect;
    mqttCtx->connect.stat = MQTT_MSG_BEGIN;
    XMEMSET(&mqttCtx->connect, 0, sizeof(MqttConnect));
    mqttCtx->connect.keep_alive_sec = mqttCtx->keep_alive_sec;
    mqttCtx->connect.clean_session = mqttCtx->clean_session;
    mqttCtx->connect.client_id = mqttCtx->client_id;

    XMEMSET(&mqttCtx->lwt_msg, 0, sizeof(mqttCtx->lwt_msg));
    mqttCtx->connect.lwt_msg = &mqttCtx->lwt_msg;
    mqttCtx->connect.enable_lwt = mqttCtx->enable_lwt;
    if (mqttCtx->enable_lwt) {
        /* Send client id in LWT payload */
        mqttCtx->lwt_msg.qos = mqttCtx->qos;
        mqttCtx->lwt_msg.retain = 0;
        mqttCtx->lwt_msg.topic_name = mqttCtx->topic_name;
        mqttCtx->lwt_msg.buffer = (byte*)mqttCtx->client_id;
        mqttCtx->lwt_msg.total_len = (word16)XSTRLEN(mqttCtx->client_id);
    }
}

static int connectToBroker(MQTTCtx* mqttCtx) {
    int retval = MqttClient_NetConnect(&(mqttCtx->client), mqttCtx->host, mqttCtx->port, mqttCtx->cmd_timeout_ms, 0,NULL);
    if (retval < 0) {
        return retval;
    }

    initConnectPacket(mqttCtx);
    retval = MqttClient_Connect(&(mqttCtx->client), &(mqttCtx->connect));
    return retval;
}



static int publishMessage(MQTTCtx* mqttCtx){
    // publish
    MqttMessage publish;
    XMEMSET(&publish, 0, sizeof(MqttPublish));
    publish.stat = MQTT_MSG_BEGIN;
    publish.retain = 0;
    publish.qos = mqttCtx->qos;
    publish.duplicate = 0;
    publish.topic_name = mqttCtx->topic_name;
    publish.packet_id = 2;
    publish.buffer = (byte*)mqttCtx->message;
    unsigned char buffbuff [PAYLOAD_SIZE];
    sprintf((char*)buffbuff, PAYLOAD, time(NULL));
    publish.buffer = (byte*)buffbuff;
    publish.total_len = (word16)XSTRLEN((char*)buffbuff);
    return MqttClient_Publish(&(mqttCtx->client), &publish);
}
