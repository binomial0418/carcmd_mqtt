#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <base64.h>

#define RELAY_A_PIN 5  // GPIO 5 (D1 on NodeMCU) - Relay A
#define RELAY_B_PIN 4  // GPIO 4 (D2 on NodeMCU) - Relay B

// WiFi 信息
const char* ssid = "wifi_ssid";
const char* password = "wifi_pass";

// MQTT 服务器信息
const char* mqttServer = "mqtt.url.com";  // mqtt broker address
const int mqttPort = 1883;  // MQTT port
const char* mqttUser = "user";  // mqtt user name
const char* mqttPassword = "pass";  // mqtt pass
const char* topic = "home/carcmd";  // mqtt chennal

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_A_PIN, OUTPUT);
  pinMode(RELAY_B_PIN, OUTPUT);
  digitalWrite(RELAY_A_PIN, HIGH);  // 初始化时关闭继电器 A
  digitalWrite(RELAY_B_PIN, HIGH);  // 初始化时关闭继电器 B

  // 连接 WiFi
  connectWiFi();

  // set mqtt
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // connect mqtt broker
  reconnectMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnectMQTT() {
  // connect mqtt broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  //當接受到mqtt資料為boot_from_ha時，觸發繼電器並發送line
  if (message == "boot_from_ha") {
    Serial.println("Engine start");
    send_line("BVB-7980 準備啟動...");

    // Trigger Relay A for 1 second
    digitalWrite(RELAY_A_PIN, LOW);
    delay(1000);  // Wait for 1 second
    digitalWrite(RELAY_A_PIN, HIGH);

    // Trigger Relay B for 5 seconds
    digitalWrite(RELAY_B_PIN, LOW);
    delay(5000);  // Wait for 5 seconds
    digitalWrite(RELAY_B_PIN, HIGH);
    send_line("BVB-7980 啟動中...");
  } else if (message == "OFF") {
    digitalWrite(RELAY_A_PIN, HIGH);  // Ensure Relay A is off
    digitalWrite(RELAY_B_PIN, HIGH);  // Ensure Relay B is off
  }
}
//這是一個發送line訊息的http服務，請自行處理或移除
void send_line(String msg){
    WiFiClient client;
    HTTPClient http;

    String encodedString = base64::encode(msg);
    // 指定目标URL
    String url = "http://www.send-line-server.com:8088/sendtoline_car.php?msg=" + encodedString;
    http.begin(client, url);

    int httpCode = http.GET(); // 发送HTTP GET请求

    // 检查HTTP响应代码
    if (httpCode > 0) {
      String payload = http.getString(); // 获取响应内容
      Serial.println(httpCode); // 打印HTTP响应代码
      Serial.println(payload);   // 打印响应内容
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end(); // 关闭连接
}
