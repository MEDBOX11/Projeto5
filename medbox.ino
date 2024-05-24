#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <myPushButton.h>

// Pin definitions
#define pinLed1 6
#define pinLed2 9
#define pinBuz 11
#define pinBot1 7
#define pinBot2 8
#define pinMag1 12
#define pinMag2 13

// Time intervals for medication reminders (days, hours, minutes, seconds)
#define Intervalo1 0,0,0,15 // ex. every 15 seconds for testing
#define Intervalo2 0,0,0,60  // ex. every 60 seconds for testing

// LCD configuration (16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RTC and button objects
RTC_DS1307 rtc;
myPushButton gaveta1(pinBot1);
myPushButton gaveta2(pinBot2);

// Variables for time and reminders
DateTime horarioAtual;
DateTime horaRemedio1;
DateTime horaRemedio2;
unsigned long ultimaAtualizacao;

bool taNaHora1 = false;
bool taNaHora2 = false;

// Function prototypes
void sirene(int pin = pinBuz, int temp = 10, int esp = 200);

void setup() {
  Serial.begin(9600);
  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  pinMode(pinBuz, OUTPUT);
  pinMode(pinMag1, INPUT);
  pinMode(pinMag2, INPUT);

  // Initialize I2C and LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();

  // Initialize RTC
  if (!rtc.begin()) {
    lcd.print("RTC nao encontrado");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("Ajustando o horario");
    rtc.adjust(DateTime(F(_DATE), F(TIME_)));
  }

  Serial.println("Fim do Setup");
  Serial.println("Para ajustar o horário do RTC, use o formato: AAAA MM DD HH MM SS");
}

void loop() {
  gaveta1.lerBotao(); 
  gaveta2.lerBotao();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove espaços em branco
    if (input.length() == 19) { // Formato AAAA MM DD HH MM SS
      int ano = input.substring(0, 4).toInt();
      int mes = input.substring(5, 7).toInt();
      int dia = input.substring(8, 10).toInt();
      int hora = input.substring(11, 13).toInt();
      int minuto = input.substring(14, 16).toInt();
      int segundo = input.substring(17, 19).toInt();

      rtc.adjust(DateTime(ano, mes, dia, hora, minuto, segundo));
      Serial.println("Horario ajustado.");
    } else {
      Serial.println("Formato invalido. Use: AAAA MM DD HH MM SS");
    }
  }

  if (millis() - ultimaAtualizacao > 1000) {
    horarioAtual = rtc.now();
    taNaHora1 = horarioAtual >= horaRemedio1;
    taNaHora2 = horarioAtual >= horaRemedio2;
    ultimaAtualizacao = millis();

    // Update the LCD with the current time
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(horarioAtual.day(), DEC);
    lcd.print('/');
    lcd.print(horarioAtual.month(), DEC);
    lcd.print('/');
    lcd.print(horarioAtual.year(), DEC);
    lcd.setCursor(0, 1);
    lcd.print(horarioAtual.hour(), DEC);
    lcd.print(':');
    lcd.print(horarioAtual.minute(), DEC);
    lcd.print(':');
    lcd.print(horarioAtual.second(), DEC);

    // Send sensor states and medication reminders to ESP32
    int sensorState1 = digitalRead(pinMag1);
    int sensorState2 = digitalRead(pinMag2);
    Serial.print("S1:");
    Serial.print(sensorState1);
    Serial.print(" S2:");
    Serial.print(sensorState2);
    if (taNaHora1) {
      Serial.print(" R1");
    }
    if (taNaHora2) {
      Serial.print(" R2");
    }
    Serial.println();
  }

  if (gaveta1.released()) {
    taNaHora1 = rtc.now() >= horaRemedio1;

    if (taNaHora1) {
      horaRemedio1 = rtc.now() + TimeSpan(Intervalo1);
      taNaHora1 = false;
    } else {
      sirene(pinBuz);
    }
  }

  if (gaveta2.released()) {
    taNaHora2 = rtc.now() >= horaRemedio2;

    if (taNaHora2) {
      horaRemedio2 = rtc.now() + TimeSpan(Intervalo2);
      taNaHora2 = false;
    } else {
      sirene(pinBuz);
    }
  }

  if (taNaHora1 || taNaHora2){
    if(millis() % 950 == 0)
      tone(pinBuz, 524, 50);
  }

  digitalWrite(pinLed1, taNaHora1);
  digitalWrite(pinLed2, taNaHora2);
}

// Function to sound the siren
void sirene(int pin, int temp, int esp) {
  for (int i = 1600; i >= 600; i--) {
    tone(pin, i, temp);
    delayMicroseconds(esp);
  }
  delay(50);
  for (int i = 1300; i >= 300; i--) {
    tone(pin, i, temp);
    delayMicroseconds(esp);
  }
  delay(50);
  for (int i = 1000; i >= 50; i--) {
    tone(pin, i, temp);
    delayMicroseconds(esp);
  }
}