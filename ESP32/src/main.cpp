// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiAP.h>
// #include <WiFiClient.h>

// WiFiServer server(8080);

// void setup() {
//     Serial.begin(9600);
//     WiFi.softAP("ESP32_wifitest", "wifitest");
//     Serial.println(WiFi.softAPIP());
//     server.begin();
// }

// void loop() {
//     WiFiClient client = server.available();

//     if (client) {
//         Serial.println("New Client."); 
//         while (client.connected()) {
//             if (client.available()) {
//                 String c = client.readStringUntil('\n');
//                 Serial.print(c);
//             }
//         }
//         client.stop();
//     }
// }







// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiUdp.h>

// const char *ssid = "ESP32_wifitest";
// const char *password = "wifitest"; 

// WiFiUDP udp;

// void setup() {
//   Serial.begin(9600);
//   WiFi.softAP("ESP32_wifitest", "wifitest");

//   Serial.println("Connected");

//   udp.begin(8080);
// }

// void loop() {
//   if (udp.parsePacket()) {
//     char packet[255];
//     udp.read(packet, 255);
//     char response[] = "response";
//     udp.beginPacket(udp.remoteIP(), udp.remotePort());
//     int i = 0;
//     while (response[i] != 0) udp.write((uint8_t)response[i++]);
//     udp.endPacket();
//     Serial.println(packet);
//   }
// }




// #include <Arduino.h>
// #include <Preferences.h>
// #include <nvs_flash.h>

// Preferences prefs;


// void setup() {
//     Serial.begin(9600);
//     //nvs_flash_erase();
//     //nvs_flash_init();
//     prefs.begin("test_namespace", false);
//     prefs.putString("str", "te");
//     //prefs.end();
// }

// void loop() {
//     delay(1000);
//     String i;
//     i = prefs.getString("str");
//     Serial.println(i);
// }



#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include <stdio.h>
#include <Timer.h>
#include <nvs_flash.h>

#define MAX_PKT_SIZE 200
#define MAX_WIFI_CRED_SIZE 30
#define LED 2
#define IP_LENGTH 20
#define MAC_LENGTH 18

uint8_t ledR = 2;
uint8_t ledG = 4;
uint8_t ledB = 5; 

WiFiUDP udp;
Preferences prefs;

char ssid[MAX_WIFI_CRED_SIZE];
char password[MAX_WIFI_CRED_SIZE];
int LEDBrightness = 0;

enum COMMANDS {
    SetWifi = 1,
    Acknowledge,
    GetIP,
    SetLED,
    ConnectionCheck
};

/*
void clearCharArray(char* array, int len) {
    memset(array, 0, len);
#if NEVER_DO_THIS_AGAIN
    for (int i = 0; i < len; i++) {
        array[i] = '\0';
    }
#endif
}
*/

int commandProcessor(char _packet[MAX_PKT_SIZE]) {

    //Serial.println(_packet);

    // identify the UDP packet is really for the ESP
    // char id[6];
    // for (int i = 0; i < 5; i++) {
    //     id[i] = _packet[i];
    // }
    // Serial.println(id);
    // if (strcmp(id, "ESP32") != 0) {
    //     return -1;
    // }

    //Serial.println("check passed");
    //Serial.print("command #");
    int cmd = (int)_packet[0];
    //Serial.println(cmd);
    String ip = WiFi.localIP().toString();

    switch (_packet[0]) {
        case COMMANDS::SetWifi:
            prefs.begin("wifi", false);
            for (int i = 0; i < MAX_WIFI_CRED_SIZE; i++) {
                ssid[i] = _packet[i+1];
                password[i] = _packet[MAX_WIFI_CRED_SIZE+i+1];
            }
            prefs.putString("ssid", ssid);
            prefs.putString("password", password);
            prefs.end();

            Serial.println("creds received: ");
            Serial.println(ssid);
            Serial.println(password);
            Serial.println("sending check.");

            
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            for (int i = 0; i < 30; i++) {
                udp.write(ssid[i]);
            }
            for (int i = 0; i < 30; i++) {
                udp.write(password[i]);
            }
            for (int i = 0; i < 17; i++) {
                udp.write(WiFi.macAddress()[i]);
            }
            udp.endPacket();
            return COMMANDS::SetWifi;
            break;

        case COMMANDS::Acknowledge:
            return COMMANDS::Acknowledge;
            break;

        case COMMANDS::GetIP: {
            Serial.println("getip command, sending ip");
            char charMac[MAC_LENGTH];
            WiFi.macAddress().toCharArray(charMac, sizeof(charMac));
            bool match = true;

            for (int i = 0; i < MAC_LENGTH; i++) {
                if (charMac[i] != _packet[i+1]) {
                    match = false;
                }
            }

            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            if (match) {
                udp.write(1);
            } else {
                udp.write(0);
            }
            udp.endPacket();

            return COMMANDS::GetIP;
            break;
        }

        case COMMANDS::SetLED:
            LEDBrightness = _packet[1];
            return COMMANDS::SetLED;
            break;

        case COMMANDS::ConnectionCheck:
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write(1);
            udp.endPacket();
            return COMMANDS::ConnectionCheck;
            break;

        default:
            break;
    }
}

void LEDBlink(int brightness){//Timer *timer, bool *state) {
    /*int timeout;
    if (*state) {
        timeout = LEDBrightness;
    } else {
        timeout = 100 - LEDBrightness;
    }

    if (timer->read() >= timeout) {
        *state = !*state;
        timer->start();
        if (*state) {
            ledcWrite(1, 255);
            ledcWrite(2, 255);
            ledcWrite(3, 255);
            //digitalWrite(LED, HIGH);
        } else {
            ledcWrite(1, 0);
            ledcWrite(2, 0);
            ledcWrite(3, 0);
            //digitalWrite(LED, LOW);
        }
    }*/
    ledcWrite(1, brightness);
    ledcWrite(2, brightness);
    ledcWrite(3, brightness);
}

#if 1
void setup() {
    ledcAttachPin(ledR, 1); // assign RGB led pins to channels
    ledcAttachPin(ledG, 2);
    ledcAttachPin(ledB, 3);
    ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcSetup(2, 12000, 8);
    ledcSetup(3, 12000, 8);
    //pinMode(LED, OUTPUT);
    // debugging
    Serial.begin(9600);

    // get stored wifi credentials
    prefs.begin("wifi", false);
    String strSsid = prefs.getString("ssid", "");
    String strPassword = prefs.getString("password", "");
    strSsid.toCharArray(ssid, sizeof(ssid));
    strPassword.toCharArray(password, sizeof(password));
    prefs.end();
}

void loop() {
    // no wifi credentials, setup AP & await credentials
    if (ssid[0] == '\0') {
        Serial.println("no wifi credentials, start AP.");
        WiFi.disconnect();
        WiFi.softAP("ESP32_"+WiFi.macAddress(), WiFi.macAddress().substring(9));
        // TODO indicate AP status with LED 3-5hz

        udp.begin(8080);
        bool credsReceived = false;
        char packet[MAX_PKT_SIZE];
        while (!credsReceived) {
            if (udp.parsePacket()) {
                Serial.println("received command in AP state.");
                //clearCharArray(packet, MAX_PKT_SIZE);
                udp.read(packet, MAX_PKT_SIZE);

                int cmd = commandProcessor(packet);
                if (cmd == COMMANDS::Acknowledge) {
                    Serial.println("correct credentials received, switching to wifi.");
                    credsReceived = true;
                }
            }
        }
        udp.stop();
   } else {
        Serial.println("wifi credentials available, attempting wifi connection.");
        Timer timer;
        int counter = 0;
        int timeout = 15000;
        bool timed_out = false;
        bool connecting = true;
        bool recaptureCreds = false;

        while (connecting) {
            timer.start();
            counter++;
            WiFi.softAPdisconnect();
            WiFi.begin(ssid, password);
            while (WiFi.status() != WL_CONNECTED && !timed_out) {
                if (timer.read() >= timeout) {
                    Serial.println("timed out");
                    timed_out = true;
                }
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("connection successful, switching to normal operating state.");
                connecting = false;
            } else if (counter >= 3) {
                Serial.println("too many time outs, restarting AP for new credentials.");
                connecting = false;
                recaptureCreds = true;
            }
        }

        if (recaptureCreds) {
            ssid[0] = '\0';
        } else {
            Serial.println("normal operating state started.");
            udp.begin(8080);
            char packet[MAX_PKT_SIZE];
            Timer LEDTimer;
            bool LEDState = false;
            while (WiFi.status() == WL_CONNECTED) {
                if (udp.parsePacket()) {
                    //Serial.println("received command in normal wifi state.");
                    //clearCharArray(packet, 255);
                    udp.read(packet, MAX_PKT_SIZE);

                    int cmd = commandProcessor(packet);
                }
                LEDBlink(LEDBrightness);//&LEDTimer, &LEDState);
            }
            udp.stop();
        }
    }
}
#else
void setup() {
    prefs.begin("wifi", false);
    prefs.putString("ssid", "");
    prefs.putString("password", "");
    prefs.end();
}
void loop() {}
#endif