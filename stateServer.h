#include <ArduinoJson.h>
#include <WiFiNINA.h>

class StateServer {
 private:
  char* ip;
  int port;
  WiFiClient socket;

  void (*connectedCb)();

  int connect() {
    this->socket.flush();
    this->socket.connect(this->ip, this->port);
    this->connectedCb();
  }

  void sendPayload(DynamicJsonDocument &payload) {
    if (!this->socket.status()) {
      this->connect();
    }

    serializeJson(payload, socket);
    socket.write('\n');
  }

 public:
  StateServer(char* ip, const int port) {
    this->ip = ip;
    this->port = port;
  };

  void onConnected(void (*cb)()) {
    this->connectedCb = cb;
  }

  boolean initialize() {
    this->connect();
  }

  boolean available() {
    return socket.available() > 0;
  }

  JsonObject getNextMessage() {
    DynamicJsonDocument updateDoc(1024);
    deserializeJson(updateDoc, socket);

    serializeJsonPretty(updateDoc, Serial);
    return updateDoc["data"];
  }

  void send(String command, String plugin, String instance) {
    send(command, plugin, instance, NULL, 0);
  }

  void send(String command, String plugin, String instance, char *args[], size_t argsLen) {
    DynamicJsonDocument payload(1024);
    payload["command"] = command;
    payload["plugin"] = plugin;
    payload["instance"] = instance;

    if (args) {
      for (size_t i = 0; i < argsLen; i++) {
        payload["args"][i] = args[i];
      }
    }

    this->sendPayload(payload);
  }

  ~StateServer(){
    socket.stop();
  };
};
