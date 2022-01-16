// Good reference: https://github.com/njanssen/arduino-nano-33-ble
#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>

#define NANO_33_BLE_SERVICE_UUID(val) ("e905de3e-" val "-44de-92c4-bb6e04fb0212")

String name;
BLEService service(NANO_33_BLE_SERVICE_UUID("0000"));
BLECharacteristic lightCharacteristic(NANO_33_BLE_SERVICE_UUID("2001"), BLENotify, 4 * sizeof(unsigned short));  // Array of 16-bit, RGB ambient

void setup() {
  Serial.begin(9600);

  Serial.print("Attempting to connect to ble: ");

  if (!BLE.begin() || !APDS.begin()) {
    Serial.println("Failled to initialized!");
    while (1)
      ;
  }

  String address = BLE.address();
  address.toUpperCase();
  name = "Nano33BLESense-";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());

  BLE.setEventHandler(BLEConnected, bleConnected);
  BLE.setEventHandler(BLEDisconnected, bleDisconnected);

  service.addCharacteristic(lightCharacteristic);

  BLE.addService(service);
  BLE.setAdvertisedService(service);
  BLE.advertise();
}

void handleCommand(String command, Stream &stream) {
  if (command == "status") {
    Serial.print("Name = ");
    Serial.println(name);

    Serial.print("Address: ");
    Serial.println(BLE.address());
  } else {
    stream.println("Unkown command");
  }
}

void bleConnected(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void bleDisconnected(BLEDevice central) {
  Serial.print("Disconnect event, central: ");
  Serial.println(central.address());
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    handleCommand(command, Serial);
  }

  if (!BLE.connected()) {
    return;
  }

  if (lightCharacteristic.subscribed() && APDS.colorAvailable()) {
    int r, g, b, ambientLight;

    APDS.readColor(r, g, b, ambientLight);

    unsigned short colors[4] = {r, g, b, ambientLight};
    lightCharacteristic.writeValue(colors, sizeof(colors));
  }
}
