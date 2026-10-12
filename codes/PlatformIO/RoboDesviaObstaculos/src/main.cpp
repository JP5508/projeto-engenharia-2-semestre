#include <Arduino.h>

// 11º Código
// Código final do projeto
// Lógica de decisão

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

// LED DO ESP
#define LED 2

// ==========
// VARIÁVEIS
// ==========

long duration;
float distance;

const float DISTANCIA_OBSTACULO = 15.0;

// Tempos em milissegundos
const unsigned long TEMPO_RECUO = 350;
const unsigned long TEMPO_CURVA = 400;

// Tempo maximo avancando sem detectar situacao
// que indique obstaculo ou borda.
// E uma estimativa para testar, nao uma medida real.
const unsigned long TEMPO_RECUPERACAO = 10000;

enum Estado
{
  SEGUINDO,
  RECUANDO,
  VIRANDO_ESQUERDA,
  VIRANDO_DIREITA
};

Estado estado = SEGUINDO;

unsigned long inicioEstado = 0;
unsigned long inicioAvanco = 0;

bool proximaViradaEsquerda = true;

// ======
// SETUP
// ======

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

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

  digitalWrite(LED, HIGH);

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
  analogWrite(ENB, vel);
}

void frenteD(int porc)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
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
  analogWrite(ENB, vel);
}

void trasD(int porc)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void parar()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  digitalWrite(LED, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =========================
// INICIAR MANOBRA DE RECUO
// =========================

void iniciarRecuo()
{
  estado = RECUANDO;
  inicioEstado = millis();

  // Alterna o lado escolhido quando nao ha
  // informacao suficiente dos sensores.
  proximaViradaEsquerda = !proximaViradaEsquerda;
}

// ============================================================================
//      LOOP
// ============================================================================

void loop()
{
  // ====================
  // LEITURA DOS SENSORES
  // ====================

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    distance = 999;
  }
  else
  {
    distance = duration * 0.0343 / 2;
  }

  int valor_IR_E = digitalRead(SENSOR_E);
  int valor_IR_D = digitalRead(SENSOR_D);

  unsigned long agora = millis();

  // ====================
  // MAQUINA DE ESTADOS
  // ====================

  switch (estado)
  {
  case SEGUINDO:

    // Obstaculo a frente:
    // recua antes de escolher para qual lado virar.
    if (distance < DISTANCIA_OBSTACULO)
    {
      iniciarRecuo();
    }

    // Os dois sensores laterais detectam algo.
    else if (valor_IR_E == LOW &&
             valor_IR_D == LOW)
    {
      iniciarRecuo();
    }

    // Obstaculo ou borda detectado a esquerda:
    // tenta virar para a direita.
    else if (valor_IR_E == LOW)
    {
      estado = VIRANDO_DIREITA;
      inicioEstado = agora;
    }

    // Obstaculo ou borda detectado a direita:
    // tenta virar para a esquerda.
    else if (valor_IR_D == LOW)
    {
      estado = VIRANDO_ESQUERDA;
      inicioEstado = agora;
    }

    // Recuperacao temporizada:
    // tenta escapar se ficou avancando por muito tempo
    // sem que nenhum sensor indique uma situacao.
    else if (agora - inicioAvanco >=
             TEMPO_RECUPERACAO)
    {
      iniciarRecuo();
    }

    else
    {
      frente(100);
    }

    break;

  case RECUANDO:

    tras(40);

    if (agora - inicioEstado >= TEMPO_RECUO)
    {

      // Se apenas um lado esta livre segundo
      // os sensores, tenta virar para esse lado.
      if (valor_IR_E == LOW &&
          valor_IR_D == HIGH)
      {
        estado = VIRANDO_DIREITA;
      }
      else if (valor_IR_D == LOW &&
               valor_IR_E == HIGH)
      {
        estado = VIRANDO_ESQUERDA;
      }
      else
      {
        // Sem uma direcao evidente, alterna o lado.
        if (proximaViradaEsquerda)
        {
          estado = VIRANDO_ESQUERDA;
        }
        else
        {
          estado = VIRANDO_DIREITA;
        }
      }

      inicioEstado = agora;
    }

    break;

  case VIRANDO_ESQUERDA:

    // A roda direita avanca para virar a esquerda.
    frenteD(50);

    if (agora - inicioEstado >= TEMPO_CURVA)
    {
      estado = SEGUINDO;
      inicioAvanco = agora;
    }

    break;

  case VIRANDO_DIREITA:

    // A roda esquerda avanca para virar a direita.
    frenteE(50);

    if (agora - inicioEstado >= TEMPO_CURVA)
    {
      estado = SEGUINDO;
      inicioAvanco = agora;
    }

    break;
  }
}