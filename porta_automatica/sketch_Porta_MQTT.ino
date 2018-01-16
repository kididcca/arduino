#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define fim_curso_aberto 52
#define fim_curso_fechado 22
#define TRIGGER_PIN1  30
#define ECHO_PIN1     28
#define MAX_DISTANCE 200

//Definicoes pinos Arduino ligados a entrada da Ponte H (para testes o motor está em A, mesmo as variáveis estando para o motor B)
int IN3 = 36;
int IN4 = 38;
int vel = 34;

byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEF};

IPAddress broker(191,36,8,4);

EthernetClient ethClient;
PubSubClient client(ethClient);

NewPing sonar1(TRIGGER_PIN1, ECHO_PIN1, MAX_DISTANCE);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if((char)payload[0] == '0'){
    //digitalWrite(led,LOW);
    Serial.println("Led vermelho desligado...?");
  }else{
    //digitalWrite(led,HIGH);
    Serial.println("Led vermelho ligado...?");
  }
  Serial.println((char)payload[0]);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("arduinoClient_porta")) {
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

void setup() {
  Serial.begin(9600);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(vel, OUTPUT);
  pinMode(fim_curso_aberto, INPUT);
  pinMode(fim_curso_fechado, INPUT);
  
  client.setServer(broker, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);

  Serial.println("Setup concluido");
}

void loop() {
  if (!client.connected()) {
    Serial.println("Reconecting..");
    reconnect();
  } else {
    Serial.println("Start System");
    int aberto = digitalRead(fim_curso_aberto);
    int fechado = digitalRead(fim_curso_fechado);
    float distancia = sonar1.ping_cm();

    char porta_fechada_str[10];
    char porta_aberta_str[10];
     
    itoa(fechado, porta_fechada_str, 10);
    itoa(aberto, porta_aberta_str, 10);
  
    Serial.print(distancia);
    Serial.println(" cm");
    delay(100);
  
    if (distancia > 0 && distancia <= 30){
      Serial.println("ABRIR PORTA");
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(vel, 220);
      delay(100);
      while (aberto == 1) {
        aberto = digitalRead(fim_curso_aberto);
      }
      Serial.println("FIM DE CURSO ABERTO");
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      client.publish((const char*)"porta0/aberto",porta_aberta_str);
  
      delay(3000);
  
      Serial.println("FECHAR PORTA");
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(vel, 220);
      while (fechado == 1) {
        fechado = digitalRead(fim_curso_fechado);
      }
      Serial.println("FIM DE CURSO FECHADO");
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      client.publish((const char*)"porta0/fechado",porta_fechada_str);
      delay(100);
    }
    client.loop();
    Ethernet.maintain();
  }
}
