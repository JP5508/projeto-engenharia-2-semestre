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

#define DIST_CRITICA 15
#define DIST_ALERTA 25

long duration;
float distance;

// ======
// SETUP
// ======

void setup()
{

  // ESP
  Serial.begin(115200);

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

  // Velocidade PWM
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

  // Velocidade PWM
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

  // Velocidade PWM
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

  // Velocidade PWM
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

  // Velocidade PWM
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

  // Velocidade PWM
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ========================
// LEITURA DO ULTRASSÔNICO
// ========================

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

    soma += duracao * 0.0343 / 2;

    delay(5);
  }

  return soma / 5.0;
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

  int valor_IR_E = digitalRead(SENSOR_E);
  int valor_IR_D = digitalRead(SENSOR_D);

  // ==================
  // LÓGICA DE DECISÃO
  // ==================

  if (distance > DIST_ALERTA)
  {
    // Caminho livre

    if (valor_IR_E == 0 && valor_IR_D == 1)
    {
      // Obstáculo à esquerda
      frenteD(55);
    }

    else if (valor_IR_D == 0 && valor_IR_E == 1)
    {
      // Obstáculo à direita
      frenteE(55);
    }

    else
    {
      frente(70);
    }
  }

  else if (distance > DIST_CRITICA)
  {
    // Aproximando de algo

    frente(50);
  }

  else
  {
    // Obstáculo muito próximo

    frente(40);

    // Esquerda livre
    if (valor_IR_E == 1 && valor_IR_D == 0)
    {
      while (distance < DIST_CRITICA)
      {
        frenteE(70);

        distance = lerDistancia();
      }
    }

    // Direita livre
    else if (valor_IR_D == 1 && valor_IR_E == 0)
    {
      while (distance < DIST_CRITICA)
      {
        frenteD(70);

        distance = lerDistancia();
      }
    }

    // Ambos livres
    else if (valor_IR_D == 1 && valor_IR_E == 1)
    {
      while (distance < DIST_CRITICA)
      {
        frenteD(70);

        distance = lerDistancia();
      }
    }

    // Encurralado
    else
    {
      tras(60);
      delay(400);

      while (distance < DIST_CRITICA)
      {
        frenteD(70);

        distance = lerDistancia();
      }
    }
  }
}