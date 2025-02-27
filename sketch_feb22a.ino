#include <SoftwareSerial.h>

#define TdsSensorPin A0
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30    // sum of sample points
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

// Create a SoftwareSerial object for communication with ESP8266
SoftwareSerial espSerial(2, 3); // RX, TX (connect to TX, RX of ESP8266)

void setup() {
  Serial.begin(9600); // Initialize Serial Monitor (for debugging)
  espSerial.begin(9600); // Initialize SoftwareSerial for ESP8266
  pinMode(TdsSensorPin, INPUT);
}

void loop() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) { // Every 40 milliseconds, read the analog value
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); // Read and store the analog value
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) { // Every 800 milliseconds, process and send data
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // Convert to voltage
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); // Temperature compensation
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5; // Convert to TDS value

    // Send TDS value to ESP8266 in JSON format
    String jsonData = "{\"TDS\":" + String(tdsValue, 0) + "}";
    espSerial.println(jsonData);

    // Print to Serial Monitor for debugging
    Serial.print("TDS Value:");
    Serial.println(tdsValue);
    Serial.println(compensationVoltage);
  }
}

// Median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}