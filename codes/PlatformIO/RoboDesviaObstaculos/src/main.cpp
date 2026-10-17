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
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33

// CONTROLADORES DE VELOCIDADE
#define ENA 14
#define ENB 32

// ==================
// VARIÁVEIS GLOBAIS
// ==================

#define DIST_ALERTA 30
#define DIST_CRITICA 20

float distance;

bool desvio_e = true;

unsigned long inicio_preso = 0;
const unsigned long tempo_preso = 4000;

// ======
// SETUP
// ======

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SENSOR_E, INPUT);
  pinMode(SENSOR_D, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

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

  int vel = (porc * 255) / 100;

  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void tras(int porc)
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  int vel = (porc * 255) / 100;

  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void frenteE(int porc)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  int vel = (porc * 255) / 100;

  analogWrite(ENA, vel);
  analogWrite(ENB, 0);
}

void frenteD(int porc)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  int vel = (porc * 255) / 100;

  analogWrite(ENA, 0);
  analogWrite(ENB, vel);
}

void trasE(int porc)
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  int vel = (porc * 255) / 100;

  analogWrite(ENA, vel);
  analogWrite(ENB, 0);
}

void trasD(int porc)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  int vel = (porc * 255) / 100;

  analogWrite(ENA, 0);
  analogWrite(ENB, vel);
}

void parar()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ==========================
// LEITURA DO ULTRASSÔNICO
// ==========================

float lerDistancia()
{
  float soma = 0;

  for (int i = 0; i < 5; i++)
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

  return soma / 5.0;
}

// =====
// LOOP
// =====

void loop()
{
  distance = lerDistancia();

  int valor_IR_E = digitalRead(SENSOR_E);
  int valor_IR_D = digitalRead(SENSOR_D);

  // EMERGÊNCIA
  if (distance <= DIST_CRITICA)
  {
    tras(70);
    delay(250);

    if (desvio_e)
    {
      frenteE(100);
      delay(250);
    }
    else
    {
      frenteD(100);
      delay(250);
    }

    desvio_e = !desvio_e;
    return;
  }

  // IR esquerdo detectou algo
  if (valor_IR_E == LOW && valor_IR_D == HIGH)
  {
    frenteD(60);
    return;
  }

  // IR direito detectou algo
  if (valor_IR_D == LOW && valor_IR_E == HIGH)
  {
    frenteE(60);
    return;
  }

  // ambos detectando
  if (valor_IR_E == LOW && valor_IR_D == LOW)
  {
    tras(70);
    delay(300);

    if (desvio_e)
    {
      frenteE(100);
      delay(350);
    }
    else
    {
      frenteD(100);
      delay(350);
    }

    desvio_e = !desvio_e;
    return;
  }

  // aproximação frontal
  if (distance <= DIST_ALERTA)
  {
    frente(70);
  }
  else
  {
    frente(100);
  }
}