#include <Arduino.h>

// ====================
// DEFINICAO DOS PINOS
// ====================

// Sensor ultrassonico HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// Sensores infravermelhos
#define SENSOR_E 35
#define SENSOR_D 34

// Motores - L298N
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33

// Controle de velocidade
#define ENA 14
#define ENB 32

// ====================
// VARIAVEIS
// ====================

long duration;
float distance;

// Alterna a direcao se os dois IR detectarem obstaculos
bool desvio_e = true;

// ====================
// LEITURA DO ULTRASSONICO
// ====================

float medirDistancia()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return 999;
  }

  return duration * 0.0343 / 2;
}

// ====================
// FUNCOES DOS MOTORES
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

void parar()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ====================
// MANOBRA DE DESVIO
// ====================

void desviar()
{
  int irE = digitalRead(SENSOR_E);
  int irD = digitalRead(SENSOR_D);

  // Obstaculo detectado a esquerda
  if (irE == LOW && irD == HIGH)
  {
    frenteD(50);
  }

  // Obstaculo detectado a direita
  else if (irE == HIGH && irD == LOW)
  {
    frenteE(50);
  }

  // Ambos detectam obstaculo
  else
  {
    if (desvio_e)
    {
      frenteE(50);
    }
    else
    {
      frenteD(50);
    }

    desvio_e = !desvio_e;
  }
}

// ====================
// SETUP
// ====================

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

  parar();

  Serial.println("Robo iniciado!");
}

// ====================
// LOOP PRINCIPAL
// ====================

void loop()
{
  distance = medirDistancia();

  int irE = digitalRead(SENSOR_E);
  int irD = digitalRead(SENSOR_D);

  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.print(" cm | IR esquerdo: ");
  Serial.print(irE);
  Serial.print(" | IR direito: ");
  Serial.println(irD);

  // 1. OBSTACULO MUITO PROXIMO
  // A unica condicao que aciona a marcha a re.
  if (distance < 10)
  {
    Serial.println("Muito perto! Recuando.");

    tras(40);
    delay(300);

    desviar();
    delay(350);
  }

  // 2. OBSTACULO PROXIMO
  else if (distance < 30)
  {
    Serial.println("Obstaculo proximo! Reduzindo.");

    // Se os IR detectarem algo, corrigir a direcao.
    if (irE == LOW || irD == LOW)
    {
      desviar();
    }
    else
    {
      frente(45);
    }
  }

  // 3. OBSTACULOS DETECTADOS PELOS IR
  else if (irE == LOW || irD == LOW)
  {
    Serial.println("Desviando pelos sensores IR.");

    desviar();
  }

  // 4. CAMINHO LIVRE
  else
  {
    frente(100);
  }

  delay(20);
}