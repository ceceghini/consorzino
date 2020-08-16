#include "arduino_secrets.h"

const char* prova = "eyJjcm9uIjoiMCAqLzMgKiAqICoiLCJ0aW1lIjoxLCJkYXRlIjp7InkiOjIwMjAsIm0iOjgsImQiOjEzLCJoIjoxOCwiaSI6Mzh9fQ==";

#include <WiFiNINA.h>
const char* WIFI_SSID = _WIFI_SSID;
const char* WIFI_PASS = _WIFI_PASS;
int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiClient _client;

#include "KMPProDinoMKRZero.h"
#include "KMPCommon.h"
#include "MqttTopicHelper2.h"
#include <PubSubClient.h>

enum DataType {
  DeviceIsReady
};

// MQTT server settings. 
const char* MQTT_SERVER = _MQTT_SERVER;
const int MQTT_PORT = 1883;
const char* MQTT_USER = _MQTT_USER;
const char* MQTT_PASS = _MQTT_PASS;
const char* MQTT_CLIENT_ID = "prova";

const char* BASE_TOPIC = "cnz";
const char* DEVICE_TOPIC = "prova";
const char* CRON_TOPIC = "setcron";

// A buffer to send output information.
char _topicBuff[128];

// Ethernet client.
// EthernetClient _client;
// Declare a MQTT client.
PubSubClient _mqttClient(MQTT_SERVER, MQTT_PORT, _client);

/**
* @brief Setup method. It Arduino executed first and initialize board.
*
* @return void
*/
void setup()
{
  delay(1000);
  Serial.begin(9600);

  // Init Dino board. Set pins, start W5500.
  //KMPProDinoMKRZero.init(ProDino_MKR_Zero_Ethernet);

  // Initialize MQTT helper
  MqttTopicHelper.init(BASE_TOPIC, DEVICE_TOPIC, &Serial);

  // Connect to the network
  connectWifi();
  
  // Set MQTT callback method
  _mqttClient.setCallback(callback);

  Serial.println("The example MqttBase is started.");

//  setCron(prova);
  
}

/**
* @brief Loop method. Arduino executed second.
*
* @return void
*/
void loop()
{
  // Checking is device connected to MQTT server.
  if (!ConnectMqtt())
  {
    return;
  }

  _mqttClient.loop();
 
}

/**
* @brief This method publishes all data per device.
* @dataType Type of data which will be publish.
* @num device number, if need for publish this topic.
* @isPrintPublish is print Publish. 
*
* @return void
*/
void publishTopic(DataType dataType, int num = 0, bool isPrintPublish = true)
{
  if (isPrintPublish)
  {
    Serial.println("Publish");
  }

  const char * topic = NULL;
  const char * payload = NULL;
  char numBuff[8];
  char payloadBuff[16];

  switch (dataType)
  {
  case DeviceIsReady:
    topic = MqttTopicHelper.getIsReadyTopic();
    payload = DEVICE_TOPIC;
    break;
  default:
    break;
  }

  if (topic != NULL)
  {
    MqttTopicHelper.printTopicAndPayload(topic, payload);
    _mqttClient.publish(topic, payload);
  }
}

/**
* @brief Print in debug console Subscribed topic and payload.
*
* @return void
*/
void printSubscribeTopic(char* topic, byte* payload, unsigned int length)
{
  Serial.println("Subscribe");
  MqttTopicHelper.printTopicAndPayload(topic, payload, length);
}

/**
* @brief Callback method. It executes when has information from subscribed topics: kmp and kmp/prodinomkrzero/#
*
* @return void
*/
void callback(char* topics, byte* payload, unsigned int payloadLen)
{

  bool payloadEmpty = payloadLen == 0;

  // If the topic doesn't start with kmp/prodinomkrzero it doesn't need to do.
  if (!MqttTopicHelper.startsWithMainTopic(topics))
    return;

  // Publishing all information: kmp/prodinomkrzero:[]
  if (MqttTopicHelper.isMainTopic(topics) && payloadEmpty)
  {
    printSubscribeTopic(topics, payload, payloadLen);
    return;
  }

  char nextTopic[32];
  char* otherTopics = nullptr;
  // Get topic after  kmp/prodinomkrzero/...
  if (!MqttTopicHelper.getNextTopic(topics, nextTopic, &otherTopics, true))
    return;

  if (isEqual(nextTopic, CRON_TOPIC)) {

    Serial.println(topics);
    
  }

}

/**
* @brief Checking if device connected to MQTT server. When it connects add subscribe topics per device.
*
* @return void
*/
bool ConnectMqtt()
{
  if (_mqttClient.connected())
  {
    return true;
  }

  Serial.println("Attempting to connect MQTT...");

  if (_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
  {
    // It is broadcast topic: kmp
    //_mqttClient.subscribe(BASE_TOPIC);
    // Building topic with wildcard symbol: kmp/prodinomkrzero/#
    // With this topic we are going to subscribe for all topics per device. All topics started with: kmp/prodinomkrzero
    MqttTopicHelper.buildTopicWithMT(_topicBuff, 1, "#");
    Serial.println(_topicBuff);
    _mqttClient.subscribe(_topicBuff);

    Serial.println("Connected.");
    publishTopic(DeviceIsReady);

    return true;
  }

  Serial.print("failed, rc=");
  Serial.print(_mqttClient.state());
  Serial.println(" try again after 5 seconds...");
  // Wait 5 seconds before retrying
  delay(5000);

  return false;
}

void connectWifi() {

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);

    // wait 2 seconds for connection:
    delay(2000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  
}

/*void setCron(inputString) {

  int inputStringLength = sizeof(inputString);
  int decodedLength = Base64.decodedLength(inputString, inputStringLength);
  char decodedString[decodedLength];
  Base64.decode(decodedString, inputString, inputStringLength);
  Serial.print("Decoded string is:\t");
  Serial.println(decodedString);
  
}*/
