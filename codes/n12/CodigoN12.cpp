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

long duration;
float distance;

boolean desvio_e = true;

unsigned long inicio_estado, fim_estado, inicio_preso = 0;
int tempo_preso = 5000;

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

  // ==================
  // LÓGICA DE DECISÃO
  // ==================

  // OBSTACULO MUITO PERTO
  if (distance < 10)
  {
    tras(50);
    delay(500);
    valor_IR_E = digitalRead(SENSOR_E);
    valor_IR_D = digitalRead(SENSOR_D);
    if (desvio_e == true && valor_IR_E == HIGH)
    {
      frenteE(80);
      delay(500);
    }
    else if (valor_IR_D == HIGH)
    {
      frenteD(80);
      delay(500);
    }
    desvio_e = !desvio_e;
  }

  // CAMINHO LIVRE
  else if (valor_IR_E == HIGH && valor_IR_D == HIGH)
  {
    if (inicio_preso == 0)
    {
      inicio_preso = millis();
    }

    if (millis() - inicio_preso >= (tempo_preso * 2))
    {
      inicio_preso = 0;

      tras(80);
      delay(500);

      valor_IR_E = digitalRead(SENSOR_E);
      valor_IR_D = digitalRead(SENSOR_D);

      if (desvio_e == true && valor_IR_E == HIGH)
      {
        frenteE(80);
        delay(500);
      }
      else if (valor_IR_D == HIGH)
      {
        frenteD(80);
        delay(500);
      }

      desvio_e = !desvio_e;
    }
    else
    {
      frente(100);
    }
  }

  // OBSTACULOS AO REDOR
  else if (valor_IR_E == LOW && valor_IR_D == LOW)
  {
    tras(50);
    delay(1000);
    valor_IR_E = digitalRead(SENSOR_E);
    valor_IR_D = digitalRead(SENSOR_D);
    if (desvio_e == true && valor_IR_E == HIGH)
    {
      frenteE(80);
      delay(800);
    }
    else if (valor_IR_D == HIGH)
    {
      frenteD(80);
      delay(800);
    }
    desvio_e = !desvio_e;
  }

  // OBSTACULO NA DIREITA
  else if (valor_IR_E == HIGH && valor_IR_D == LOW)
  {
    if (inicio_preso == 0)
    {
      inicio_preso = millis();
    }

    if (millis() - inicio_preso >= tempo_preso)
    {
      inicio_preso = 0;

      tras(80);
      delay(500);

      valor_IR_E = digitalRead(SENSOR_E);
      valor_IR_D = digitalRead(SENSOR_D);

      if (desvio_e == true && valor_IR_E == HIGH)
      {
        frenteE(80);
        delay(500);
      }
      else if (valor_IR_D == HIGH)
      {
        frenteD(80);
        delay(500);
      }

      desvio_e = !desvio_e;
    }
    else
    {
      // vira até o obstaculo sair do sensor direito
      inicio_estado = millis();
      while (valor_IR_D == LOW)
      {
        frenteE(50);
        valor_IR_E = digitalRead(SENSOR_E);
        valor_IR_D = digitalRead(SENSOR_D);
        distance = lerDistancia();
        if (valor_IR_E == LOW || distance < 10 || (millis() - inicio_preso >= tempo_preso))
        {
          break;
        }
      }
      fim_estado = millis() - inicio_estado;
      if (millis() - inicio_preso < tempo_preso)
      {
        frenteD(50);
        delay(fim_estado / 2);
      }
    }
  }

  // OBSTACULO NA ESQUERDA
  else if (valor_IR_E == LOW && valor_IR_D == HIGH)
  {
    if (inicio_preso == 0)
    {
      inicio_preso = millis();
    }

    if (millis() - inicio_preso >= tempo_preso)
    {
      inicio_preso = 0;

      tras(80);
      delay(500);

      valor_IR_E = digitalRead(SENSOR_E);
      valor_IR_D = digitalRead(SENSOR_D);

      if (desvio_e == true && valor_IR_E == HIGH)
      {
        frenteE(80);
        delay(500);
      }
      else if (valor_IR_D == HIGH)
      {
        frenteD(80);
        delay(500);
      }

      desvio_e = !desvio_e;
    }
    else
    {
      // vira até o obstaculo sair do sensor esquerdo
      inicio_estado = millis();
      while (valor_IR_E == LOW)
      {
        frenteD(50);
        valor_IR_E = digitalRead(SENSOR_E);
        valor_IR_D = digitalRead(SENSOR_D);
        distance = lerDistancia();
        if (valor_IR_D == LOW || distance < 10 || (millis() - inicio_preso >= tempo_preso))
        {
          break;
        }
      }
      fim_estado = millis() - inicio_estado;
      if (millis() - inicio_preso < tempo_preso)
      {
        frenteE(50);
        delay(fim_estado / 2);
      }
    }
  }
  // fim do loop
}