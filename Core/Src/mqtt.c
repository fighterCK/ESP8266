/*
================================================================================
mqtt_client.c - MQTT客户端实现文件
================================================================================
*/
#include "mqtt_client.h"
#include "app_task.h"

/* Private variables ---------------------------------------------------------*/
volatile uint8_t mqtt_connected = 0;
uint16_t mqtt_message_id = 1;

/**
  * @brief  Connect to MQTT broker
  * @retval MQTT_StatusTypeDef
  */
MQTT_StatusTypeDef MQTT_Connect(void)
{
    if(!wifi_connected) return MQTT_ERROR;

    // Connect to MQTT server
    if(ESP8266_ConnectTCP(MQTT_BROKER, MQTT_PORT) != ESP8266_OK)
        return MQTT_ERROR;

    osDelay(2000);

    // Build MQTT CONNECT packet
    uint8_t connect_packet[128];
    uint16_t packet_len = 0;

    // Fixed header
    connect_packet[packet_len++] = 0x10; // CONNECT message type

    // Variable header
    uint8_t remaining_length_pos = packet_len++;
    connect_packet[packet_len++] = 0x00; // Protocol name length MSB
    connect_packet[packet_len++] = 0x04; // Protocol name length LSB
    connect_packet[packet_len++] = 'M';
    connect_packet[packet_len++] = 'Q';
    connect_packet[packet_len++] = 'T';
    connect_packet[packet_len++] = 'T';
    connect_packet[packet_len++] = 0x04; // Protocol level
    connect_packet[packet_len++] = 0xC2; // Connect flags
    connect_packet[packet_len++] = 0x00; // Keep alive MSB
    connect_packet[packet_len++] = MQTT_KEEP_ALIVE; // Keep alive LSB

    // Client ID
    uint16_t client_id_len = strlen(MQTT_CLIENT_ID);
    connect_packet[packet_len++] = (client_id_len >> 8) & 0xFF;
    connect_packet[packet_len++] = client_id_len & 0xFF;
    memcpy(&connect_packet[packet_len], MQTT_CLIENT_ID, client_id_len);
    packet_len += client_id_len;

    // Username
    uint16_t username_len = strlen(MQTT_USERNAME);
    connect_packet[packet_len++] = (username_len >> 8) & 0xFF;
    connect_packet[packet_len++] = username_len & 0xFF;
    memcpy(&connect_packet[packet_len], MQTT_USERNAME, username_len);
    packet_len += username_len;

    // Password
    uint16_t password_len = strlen(MQTT_PASSWORD);
    connect_packet[packet_len++] = (password_len >> 8) & 0xFF;
    connect_packet[packet_len++] = password_len & 0xFF;
    memcpy(&connect_packet[packet_len], MQTT_PASSWORD, password_len);
    packet_len += password_len;

    // Update remaining length
    connect_packet[remaining_length_pos] = packet_len - 2;

    if(ESP8266_SendData(connect_packet, packet_len) == ESP8266_OK)
    {
        mqtt_connected = 1;
        return MQTT_OK;
    }

    return MQTT_ERROR;
}

/**
  * @brief  Disconnect from MQTT broker
  * @retval MQTT_StatusTypeDef
  */
MQTT_StatusTypeDef MQTT_Disconnect(void)
{
    uint8_t disconnect_packet[2] = {0xE0, 0x00}; // DISCONNECT packet

    if(ESP8266_SendData(disconnect_packet, 2) == ESP8266_OK)
    {
        mqtt_connected = 0;
        return MQTT_OK;
    }

    return MQTT_ERROR;
}

/**
  * @brief  Publish message to MQTT topic
  * @param  topic: Topic name
  * @param  payload: Message payload
  * @retval MQTT_StatusTypeDef
  */
MQTT_StatusTypeDef MQTT_Publish(const char* topic, const char* payload)
{
    if(!mqtt_connected) return MQTT_NOT_CONNECTED;

    uint8_t publish_packet[256];
    uint16_t packet_len = 0;
    uint16_t topic_len = strlen(topic);
    uint16_t payload_len = strlen(payload);

    // Fixed header
    publish_packet[packet_len++] = 0x30; // PUBLISH message type
    publish_packet[packet_len++] = 2 + topic_len + payload_len; // Remaining length

    // Variable header - Topic name
    publish_packet[packet_len++] = (topic_len >> 8) & 0xFF;
    publish_packet[packet_len++] = topic_len & 0xFF;
    memcpy(&publish_packet[packet_len], topic, topic_len);
    packet_len += topic_len;

    // Payload
    memcpy(&publish_packet[packet_len], payload, payload_len);
    packet_len += payload_len;

    if(ESP8266_SendData(publish_packet, packet_len) == ESP8266_OK)
        return MQTT_OK;

    return MQTT_ERROR;
}

/**
  * @brief  Subscribe to MQTT topic
  * @param  topic: Topic name
  * @retval MQTT_StatusTypeDef
  */
MQTT_StatusTypeDef MQTT_Subscribe(const char* topic)
{
    if(!mqtt_connected) return MQTT_NOT_CONNECTED;

    uint8_t subscribe_packet[128];
    uint16_t packet_len = 0;
    uint16_t topic_len = strlen(topic);

    // Fixed header
    subscribe_packet[packet_len++] = 0x82; // SUBSCRIBE message type
    subscribe_packet[packet_len++] = 2 + 2 + topic_len + 1; // Remaining length

    // Variable header - Message ID
    subscribe_packet[packet_len++] = (mqtt_message_id >> 8) & 0xFF;
    subscribe_packet[packet_len++] = mqtt_message_id & 0xFF;
    mqtt_message_id++;

    // Payload - Topic filter
    subscribe_packet[packet_len++] = (topic_len >> 8) & 0xFF;
    subscribe_packet[packet_len++] = topic_len & 0xFF;
    memcpy(&subscribe_packet[packet_len], topic, topic_len);
    packet_len += topic_len;

    // QoS level
    subscribe_packet[packet_len++] = 0x00;

    if(ESP8266_SendData(subscribe_packet, packet_len) == ESP8266_OK)
        return MQTT_OK;

    return MQTT_ERROR;
}

/**
  * @brief  Unsubscribe from MQTT topic
  * @param  topic: Topic name
  * @retval MQTT_StatusTypeDef
  */
MQTT_StatusTypeDef MQTT_Unsubscribe(const char* topic)
{
    if(!mqtt_connected) return MQTT_NOT_CONNECTED;

    uint8_t unsubscribe_packet[128];
    uint16_t packet_len = 0;
    uint16_t topic_len = strlen(topic);

    // Fixed header
    unsubscribe_packet[packet_len++] = 0xA2; // UNSUBSCRIBE message type
    unsubscribe_packet[packet_len++] = 2 + 2 + topic_len; // Remaining length

    // Variable header - Message ID
    unsubscribe_packet[packet_len++] = (mqtt_message_id >> 8) & 0xFF;
    unsubscribe_packet[packet_len++] = mqtt_message_id & 0xFF;
    mqtt_message_id++;

    // Payload - Topic filter
    unsubscribe_packet[packet_len++] = (topic_len >> 8) & 0xFF;
    unsubscribe_packet[packet_len++] = topic_len & 0xFF;
    memcpy(&unsubscribe_packet[packet_len], topic, topic_len);
    packet_len += topic_len;

    if(ESP8266_SendData(unsubscribe_packet, packet_len) == ESP8266_OK)
        return MQTT_OK;

    return MQTT_ERROR;
}

/**
  * @brief  Process received MQTT message
  * @param  data: Received data
  * @param  length: Data length
  * @retval None
  */
void MQTT_ProcessMessage(const char* data, uint16_t length)
{
    // Simple MQTT message parsing
    if(strstr(data, "+IPD") != NULL)
    {
        // Parse received MQTT message
        MQTT_Message_t mqtt_msg;
        strcpy(mqtt_msg.topic, MQTT_TOPIC_SUB);
        strcpy(mqtt_msg.payload, "received_command");
        osMessageQueuePut(mqttQueueHandle, &mqtt_msg, 0, 0);
    }
}

/**
  * @brief  Send MQTT keep alive packet
  * @retval None
  */
void MQTT_KeepAlive(void)
{
    if(mqtt_connected)
    {
        uint8_t pingreq_packet[2] = {0xC0, 0x00}; // PINGREQ packet
        ESP8266_SendData(pingreq_packet, 2);
    }
}
