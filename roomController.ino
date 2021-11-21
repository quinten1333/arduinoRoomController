#include <WiFiNINA.h>

#include "./network.h" // Sets SSID and pass variables
#include "./stateServer.h"

const int buttonPin = 2;
const int ledPin = 4;
bool buttonHandled = false;

StateServer stateServer("192.168.1.17", 2000);

int status = WL_IDLE_STATUS;

WiFiServer server(80);

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.print("Attempting to connect to network: ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, pass);

  if (status != WL_CONNECTED) {
    Serial.print("Could not connect to network.");
    while (true)
      ;
  }
  Serial.println("Connected");

  server.begin();
  stateServer.onConnected(subscribeState);
  stateServer.initialize();
}

void printNetworkInfo(Stream &stream) {
  stream.println("Board Information:");
  stream.print("IP Address: ");
  stream.println(WiFi.localIP());
  stream.println("Network Information:");
  stream.print("SSID: ");
  stream.println(WiFi.SSID());
  stream.print("signal strength (RSSI):");
  stream.println(WiFi.RSSI());
}

void handleCommand(String command, Stream &stream) {
  if (command == "status") {
    printNetworkInfo(stream);
  } else {
    stream.println("Unkown command");
  }
}

void handleClient(WiFiClient client) {
  if (!client.connected()) {
    client.stop();
    return;
  }

  if (client.available() > 0) {
    String command = client.readString();
    command.trim();
    if (command == "exit") {
      client.println("Closing connection");
      client.stop();
      return;
    }

    handleCommand(command, client);
  }
}

void subscribeState() {
    stateServer.send("subscribe", "bluOS", "streamer");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    handleCommand(command, Serial);
  }

  if (WiFiClient newClient = server.available()) {
    handleClient(newClient);
  }

  if (stateServer.available()) {
    JsonObject stateServerUpdate = stateServer.getNextMessage();
    digitalWrite(ledPin, (boolean)stateServerUpdate["playing"]);
  }

  int buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && !buttonHandled) {
    buttonHandled = true;

    char *args[] = { "toggle" };
    stateServer.send("action", "bluOS", "streamer", args, 1);
  } else if (buttonState == LOW && buttonHandled) {
    buttonHandled = false;
  }
}
