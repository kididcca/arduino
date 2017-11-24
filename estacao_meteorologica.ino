#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define led 8
#define led2 9
#define pino_d  2     //sensor de chuva
#define pino_a  A5    //sensor de chuva
#define DHTPIN 3     // Define o pino de dados para pino digital 2
#define DHTTYPE DHT22   // Define que o sensor é o DHT 22 (AM2302)

byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress broker(191,36,8,4);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

EthernetClient ethClient;
PubSubClient client(ethClient);

int temp_anterior = 0;
int chuva_anterior = 0;
int temperatura_anterior = 0;
int umidade_anterior = 0;
float pressao_anterior = 0;

int temp_atual = 0;
int chuva_atual = 0;
int umidade_atual = 0;
int temperatura_atual = 0;
float pressao_atual = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if((char)payload[0] == '0'){
    digitalWrite(led,LOW);
    Serial.println("Led vermelho desligado...?");
  }else{
    digitalWrite(led,HIGH);
    Serial.println("Led vermelho ligado...?");
  }
  Serial.println((char)payload[0]);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe("estacao0/pi");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pino_a,INPUT); //sensor de chuva

  client.setServer(broker, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);
  dht.begin();

  /*if (!bmp.begin()) {
    Serial.println("Sensor não encontrado");
    while (1) {}
  }*/

  delay(1500);
}

void loop(){
  if (!client.connected()) {
    reconnect();
  } else {

    chuva_atual = analogRead(pino_a);

    umidade_atual = dht.readHumidity();

    temperatura_atual = dht.readTemperature();

    //pressao_atual = bmp.readPressure();

    char chuva_atual_str[10];
    char temperatura_atual_str[10];
    char umidade_atual_str[10];
    char pres_atual_str[10];

    itoa(chuva_atual,chuva_atual_str,10);
    itoa(temperatura_atual, temperatura_atual_str,10);
    itoa(umidade_atual, umidade_atual_str,10);
    itoa(pressao_atual,pres_atual_str,10);

    delay(1000);

    if(digitalRead(pino_d) == 1){
      if(chuva_anterior != chuva_atual){
        Serial.print("Chuva atual: ");
        Serial.println(chuva_atual);
        client.publish((const char*)"estacao0/chuva",chuva_atual_str);
      }
    }

    if(digitalRead(DHTPIN) == 1){
      if(temperatura_anterior != temperatura_atual){
        Serial.print("Temp  atual: ");
        Serial.println(temperatura_atual);
        client.publish((const char*)"estacao0/temperatura",temperatura_atual_str);
      }
    }
    if(digitalRead(DHTPIN) == 1){
      if(umidade_anterior != umidade_atual){
        Serial.print("Umidade atual: ");
        Serial.println(umidade_atual);
        client.publish((const char*)"estacao0/umidade",umidade_atual_str);
      }
    }

    /*if(pressao_anterior != pressao_atual){
      Serial.print("Pressão atual: ");
      Serial.println(pressao_atual);
      client.publish((const char*)"estacao0/pressao",pres_atual_str);
    }*/
  }

  chuva_anterior = chuva_atual;
  temperatura_anterior = temperatura_atual;
  umidade_anterior = umidade_atual;
  pressao_anterior = pressao_atual;

  client.loop();
  Ethernet.maintain();
}
