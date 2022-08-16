#include <IBMIOTF32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_I2CDevice.h>
#include <SPI.h>

#define LIGHT_PIN 36
#define DHTPIN 22
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

String user_html = "";


char*               ssid_pfix = (char*)"iothome";
unsigned long       lastPublishMillis = 0;

const char *displaytext = "Ready";

int lightValue;
float h, t;


void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");


    data["LIGHT"] = lightValue;
    data["TEMP"] = t;
    data["HUMIDITY"] = h;

    serializeJson(root, msgBuffer);
    client.publish(publishTopic, msgBuffer);

}

void handleUserCommand(JsonDocument* root) {
    JsonObject d = (*root)["d"];

    // YOUR CODE for command handling

}

void message(char* topic, byte* payload, unsigned int payloadLength) {
    byte2buff(msgBuffer, payload, payloadLength);
    StaticJsonDocument<512> root;
    DeserializationError error = deserializeJson(root, String(msgBuffer));

    if (error) {
        Serial.println("handleCommand: payload parse FAILED");
        return;
    }

    handleIOTCommand(topic, &root);
    if (strstr(topic, "/device/update")) {
        JsonObject meta = cfg["meta"];

    // YOUR CODE for meta data synchronization

    } else if (strstr(topic, "/cmd/")) {            // strcmp return 0 if both string matches
        handleUserCommand(&root);
    }
}

void lightLoop() {

    lightValue = analogRead(LIGHT_PIN);
    Serial.print("brightness = ");
    Serial.println(lightValue);

}

void dhtLoop() {

    h = dht.readHumidity();
    t = dht.readTemperature();

    Serial.print("humidity = ");
    Serial.println(h);
    Serial.print("temperature = ");
    Serial.println(t);

}

void dhtSetup() {
    dht.begin();
}


void setup() {
    Serial.begin(115200);
    initDevice();

    dhtSetup();

    yield();
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? atoi((const char*)meta["pubInterval"]) : 0;
    lastPublishMillis = - pubInterval;
    startIOTWatchDog((void*)&lastPublishMillis, (int)(pubInterval * 5));
    // YOUR CODE for initialization of device/environment
    yield();
    WiFi.mode(WIFI_STA);

    yield();
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    int i = 0;
    yield();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(i++ > 25) reboot();
    }
    Serial.println("\nIP address : "); Serial.println(WiFi.localIP());

    client.setCallback(message);
    set_iot_server();
}

void loop() {
    
    if (!client.connected()) {
        yield();
        iot_connect();
    }
    client.loop();
    // YOUR CODE for routine operation in loop

    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
        yield();
    }

    lightLoop();

    dhtLoop();

    delay(2000);

}