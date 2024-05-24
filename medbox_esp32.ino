/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL2xU_0sx_a"
#define BLYNK_TEMPLATE_NAME "Medbox"
#define BLYNK_AUTH_TOKEN "igbv3yXUwipjSphiW2RpuGNPzLCpPVr_"
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "iPhone (2)";
char pass[] = "yyyyyyyy";

void setup() {
  Serial.begin(115200);

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("ESP32 setup complete");
}

void loop() {
  Blynk.run();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("S1:")) {
      int sensorState1 = input.substring(3, 4).toInt();
      int sensorState2 = input.substring(8, 9).toInt();

      Blynk.virtualWrite(V1, sensorState1);
      Blynk.virtualWrite(V2, sensorState2);

      if (input.indexOf("R1") != -1) {
        Blynk.notify("Hora de tomar o remédio 1!");
      }
      if (input.indexOf("R2") != -1) {
        Blynk.notify("Hora de tomar o remédio 2!");
      }
    }
  }
}