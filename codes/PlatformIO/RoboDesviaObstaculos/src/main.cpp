#include <Arduino.h>

// ============================================================
// ROBÔ DESVIADOR DE OBSTÁCULOS - ESP32  (versão 2)
// Foco: reagir ANTES de chegar na parede e girar de verdade
// ============================================================

#define DEBUG 1   // 1 = imprime estado e sensores no Serial a cada 200 ms

// ---------- PINOS ----------
#define TRIG_PIN 5
#define ECHO_PIN 18

#define SENSOR_E 35   // IR esquerdo  (LOW = obstáculo)
#define SENSOR_D 34   // IR direito   (LOW = obstáculo)

#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33
#define ENA 14
#define ENB 32


// ---------- PARÂMETROS AJUSTÁVEIS ----------
// Velocidades (%)
const int VEL_CRUZEIRO   = 70;   // velocidade normal (100% deixa o robô sem tempo de reagir)
const int VEL_ALERTA_MIN = 45;   // velocidade ao chegar perto da DIST_MIN
const int VEL_CURVA      = 70;   // roda de fora nas curvas laterais
const int VEL_PIVO       = 65;   // giro no lugar
const int VEL_RE         = 60;

// PWM mínimo em que o motor realmente gira (abaixo disso ele só zumbe).
// Aumente se o robô não sair do lugar em velocidades baixas.
const int PWM_MIN = 90;

// Distâncias (cm) do ultrassônico
const float DIST_ALERTA = 20.0;  // começa a desacelerar
const float DIST_MIN    = 12.0;  // para e desvia
const float DIST_LIVRE  = 25.0;  // só considera o caminho livre depois de passar disso

// Tempos (ms)
const unsigned long FOLGA_CURVA_MS   = 150;   // continua curvando um pouco depois do IR liberar
const unsigned long TEMPO_RE_MS      = 350;
const unsigned long TEMPO_RE_LONGO   = 900;   // quando está preso (várias tentativas seguidas)
const unsigned long TEMPO_PIVO_MIN   = 250;   // pivô mínimo antes de checar se liberou
const unsigned long TEMPO_PIVO_MAX   = 1800;  // desiste do pivô e tenta de novo
const unsigned long TEMPO_CURVA_MAX  = 4000;  // curva lateral sem sucesso = preso
const unsigned long INTERVALO_US_MS  = 30;    // intervalo entre leituras do ultrassônico
const unsigned long TIMEOUT_ECHO_US  = 4000;  // ~68 cm de alcance (suficiente para DIST_ALERTA)

// ---------- ESTADOS ----------
enum Estado { AVANCANDO, CURVANDO, RE, GIRANDO };
Estado estado = AVANCANDO;

unsigned long tEstado = 0;
unsigned long fimCurva = 0;
unsigned long ultimaLeitura = 0;
unsigned long tUltimaTentativa = 0;
unsigned long ultimoDebug = 0;

bool curvaD = true;     // lado da curva lateral atual
bool fugirD = true;     // true = pivô como frenteD (afasta de obstáculo na direita)
int  tentativas = 0;

float distance = 999;
bool obsE = false, obsD = false;

// ============================================================
// MOTORES  (vel: -100 a 100)
// ============================================================

int velParaPwm(int vel)
{
  vel = constrain(abs(vel), 0, 100);
  if (vel == 0) return 0;
  return map(vel, 1, 100, PWM_MIN, 255);
}

void motorA(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN1, vel < 0 ? HIGH : LOW);
  digitalWrite(IN2, vel > 0 ? HIGH : LOW);
  analogWrite(ENA, velParaPwm(vel));
}

void motorB(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN3, vel > 0 ? HIGH : LOW);
  digitalWrite(IN4, vel < 0 ? HIGH : LOW);
  analogWrite(ENB, velParaPwm(vel));
}

void frente(int p) { motorA(p);  motorB(p); }
void tras(int p)   { motorA(-p); motorB(-p); }
void parar()       { motorA(0);  motorB(0); }

// Mesmo sentido de giro que a frenteD / frenteE do código original:
// "D" = afasta de obstáculo na direita, "E" = afasta de obstáculo na esquerda.
// Se o robô girar PARA o lado do obstáculo, troque os sinais destas 4 funções.
void pivoD(int p)       { motorA(p);  motorB(-p); }  // gira no lugar
void pivoE(int p)       { motorA(-p); motorB(p);  }
void curvaFugirD(int p) { motorA(p);  motorB(0);  }  // curva fechada (uma roda parada)
void curvaFugirE(int p) { motorA(0);  motorB(p);  }

// ============================================================
// ULTRASSÔNICO (mediana de 3 leituras)
// ============================================================

float lerDistanciaUnica()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long d = pulseIn(ECHO_PIN, HIGH, TIMEOUT_ECHO_US);
  return (d == 0) ? 999.0 : d * 0.0343 / 2.0;
}

float lerDistancia()
{
  float a = lerDistanciaUnica(); delay(2);
  float b = lerDistanciaUnica(); delay(2);
  float c = lerDistanciaUnica();

  if ((a >= b && a <= c) || (a <= b && a >= c)) return a;
  if ((b >= a && b <= c) || (b <= a && b >= c)) return b;
  return c;
}

// ============================================================
// AUXILIARES
// ============================================================

void mudarEstado(Estado novo)
{
  estado = novo;
  tEstado = millis();
}

void atualizarSensores(unsigned long agora)
{
  if (agora - ultimaLeitura >= INTERVALO_US_MS)
  {
    distance = lerDistancia();
    ultimaLeitura = agora;
  }
  obsE = (digitalRead(SENSOR_E) == LOW);
  obsD = (digitalRead(SENSOR_D) == LOW);
}

// Para, dá ré e depois gira no lugar até o caminho liberar
void iniciarEscape(unsigned long agora)
{
  if (agora - tUltimaTentativa > 4000) tentativas = 0;  // faz tempo que não travava
  tentativas++;
  tUltimaTentativa = agora;

  tras(VEL_RE);
  mudarEstado(RE);
}

#if DEBUG
void debug(unsigned long agora)
{
  if (agora - ultimoDebug < 200) return;
  ultimoDebug = agora;
  const char* nomes[] = {"AVANCANDO", "CURVANDO", "RE", "GIRANDO"};
  Serial.printf("%-9s dist=%5.1f  E=%d D=%d  tent=%d\n",
                nomes[estado], distance, obsE, obsD, tentativas);
}
#endif

// ============================================================
// SETUP
// ============================================================

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
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  unsigned long agora = millis();
  atualizarSensores(agora);

#if DEBUG
  debug(agora);
#endif

  switch (estado)
  {
    // ------------------------------------------------------
    case AVANCANDO:
    {
      if (distance < DIST_MIN)
      {
        iniciarEscape(agora);
      }
      else if (obsD || obsE)
      {
        curvaD = obsD;
        fimCurva = agora + FOLGA_CURVA_MS;
        mudarEstado(CURVANDO);
      }
      else if (distance < DIST_ALERTA)
      {
        // desacelera conforme se aproxima do obstáculo
        float f = (distance - DIST_MIN) / (DIST_ALERTA - DIST_MIN);
        frente(VEL_ALERTA_MIN + (int)((VEL_CRUZEIRO - VEL_ALERTA_MIN) * f));
      }
      else
      {
        frente(VEL_CRUZEIRO);
      }
      break;
    }

    // ------------------------------------------------------
    case CURVANDO:
    {
      if (distance < DIST_MIN || (agora - tEstado > TEMPO_CURVA_MAX))
      {
        iniciarEscape(agora);
        break;
      }

      if (obsD)      { curvaD = true;  fimCurva = agora + FOLGA_CURVA_MS; }
      else if (obsE) { curvaD = false; fimCurva = agora + FOLGA_CURVA_MS; }

      if (curvaD) curvaFugirD(VEL_CURVA); else curvaFugirE(VEL_CURVA);

      if (agora >= fimCurva) mudarEstado(AVANCANDO);
      break;
    }

    // ------------------------------------------------------
    case RE:
    {
      unsigned long tempoRe = (tentativas >= 3) ? TEMPO_RE_LONGO : TEMPO_RE_MS;

      if (agora - tEstado >= tempoRe)
      {
        // escolhe o lado do giro: foge do lado com obstáculo, senão alterna
        if (obsD && !obsE)      fugirD = true;
        else if (obsE && !obsD) fugirD = false;
        else                    fugirD = !fugirD;

        mudarEstado(GIRANDO);
      }
      break;
    }

    // ------------------------------------------------------
    case GIRANDO:
    {
      if (fugirD) pivoD(VEL_PIVO); else pivoE(VEL_PIVO);

      unsigned long t = agora - tEstado;
      unsigned long tMax = (tentativas >= 3) ? 3000 : TEMPO_PIVO_MAX;
      bool livre = (distance > DIST_LIVRE) && !obsE && !obsD;

      if (t >= TEMPO_PIVO_MIN && livre)
      {
        mudarEstado(AVANCANDO);
      }
      else if (t >= tMax)
      {
        iniciarEscape(agora);   // não liberou: ré de novo e tenta pelo outro lado
      }
      break;
    }
  }
}