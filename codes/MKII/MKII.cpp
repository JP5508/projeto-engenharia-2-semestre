// =====================
//  MKII - CÓDIGO FINAL
// =====================

#include <Arduino.h>

// ====================
// DEFINIÇÃO DOS PINOS
// ====================

// SENSOR ULTRASSÔNICO
#define TRIG_PIN 5
#define ECHO_PIN 18

// SENSORES IR
#define SENSOR_E 35
#define SENSOR_D 34

// MOTORES
#define IN1 25 // esquerda
#define IN2 26
#define IN3 27 // direita
#define IN4 33

// CONTROLADORES DE VELOCIDADE
#define ENA 32 // esquerda
#define ENB 14 // direita

// ==================
// VARIÁVEIS GLOBAIS
// ==================

long duration;
float distance;
int pwm_adj = 253;

// bool desvio_e = true;
int lado_desvio = 0; // 2 esquerda, 1 direita.

// ======
// SETUP
// ======

void setup()
{
  // Ultrassônico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Sensores IR
  pinMode(SENSOR_E, INPUT);
  pinMode(SENSOR_D, INPUT);

  // Motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Velocidade
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
}

// ====================
// FUNÇÕES DOS MOTORES
// ====================

void frente(int porc)
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void tras(int porc)
{

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void frenteE(int porc)
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void frenteD(int porc)
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void trasE(int porc)
{

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void trasD(int porc)
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Velocidade PWM
  int velA = (porc * 255) / 100;
  int velB = (porc * pwm_adj) / 100;
  analogWrite(ENA, velA);
  analogWrite(ENB, velB);
}

void parar()
{

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =======================
// FUNÇÃO DO ULTRASSÔNICO
// =======================

float lerDistancia()
{
  float soma = 0;

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duracao == 0)
    {
      soma += 999;
    }
    else
    {
      soma += duracao * 0.0343 / 2;
    }

    delay(5);
  }

  return soma / 3.0;
}

// ====================
// FUNÇÃO DESVIO
// ====================

/*
void decisaoDesvio()
{
  if (desvio_e == true)
  {
    frenteE(100);
    delay(200);
    desvio_e = false;
  }
  else
  {
    frenteD(100);
    delay(200);
    desvio_e = true;
  }
}*/

void decisaoDesvio()
{
  if (lado_desvio < 2)
  {
    frenteE(100);
    delay(200);
    lado_desvio += 1;
  }
  else
  {
    frenteD(100);
    delay(200);
    lado_desvio = 0;
  }
}

// =====
// LOOP
// =====

void loop()
{
  // ========================
  // LEITURA DO ULTRASSÔNICO
  // ========================

  distance = lerDistancia();

  // ===============
  // LEITURA DOS IR
  // ===============

  int IR_E = digitalRead(SENSOR_E);
  int IR_D = digitalRead(SENSOR_D);

  // ==================
  // LÓGICA DE DECISÃO
  // ==================

  if (distance < 10) // OBSTÁCULO MUITO PERTO
  {
    while (distance < 18) // pegar distância do obstáculo
    {
      tras(100);
      delay(500);
      distance = lerDistancia();
    }
    decisaoDesvio(); // decidir lado do desvio
  }

  else if (distance < 30) // SE APROXIMANDO DE ALGUM OBSTÁCULO
  {
    if (IR_E == LOW && IR_D == HIGH) // obstaculo à esquerda
    {
      trasE(80);
      delay(100);
      frenteD(80);
      delay(100);
    }
    else if (IR_E == HIGH && IR_D == LOW) // obstaculo à direita
    {
      trasD(80);
      delay(100);
      frenteE(80);
      delay(100);
    }
    else if (IR_E == LOW && IR_D == LOW) // obstaculos em ambos os lados
    {
      while (IR_E == LOW && IR_D == LOW) // pegar distância dos obstáculos
      {
        tras(100);
        delay(500);
        IR_E = digitalRead(SENSOR_E);
        IR_D = digitalRead(SENSOR_D);
      }
    }
    else // continuar em frente
    {
      frente(100);
    }
  }

  else // CAMINHO FRONTAL "LIVRE"
  {
    if (IR_E == LOW && IR_D == HIGH) // obstaculo à esquerda
    {
      trasE(80);
      delay(100);
      frenteD(80);
      delay(100);
    }
    else if (IR_E == HIGH && IR_D == LOW) // obstaculo à direita
    {
      trasD(80);
      delay(100);
      frenteE(80);
      delay(100);
    }
    else if (IR_E == LOW && IR_D == LOW) // obstaculos em ambos os lados
    {
      while (IR_E == LOW && IR_D == LOW) // pegar distância dos obstáculos
      {
        tras(100);
        delay(500);
        IR_E = digitalRead(SENSOR_E);
        IR_D = digitalRead(SENSOR_D);
      }
    }
    else // continuar em frente
    {
      frente(100);
    }
  }
}