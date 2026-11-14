#include <Arduino.h>

// ============================================================
// ROBÔ DESVIADOR DE OBSTÁCULOS - ESP32  (versão 3)
// Projeto MK2 - Engenharia da Computação - Senac Santo Amaro
//
// Ideia: lógica simples e reativa (como a que funcionou melhor),
// mas sem delay() bloqueante e com curvas terminadas por SENSOR
// (não por tempo fixo), para não depender da carga da bateria.
//
// Estados: ANDANDO -> DESVIANDO -> (RE) -> DESVIANDO -> ANDANDO
// Todos os parâmetros de ajuste ficam no bloco "AJUSTES".
// ============================================================

#define DEBUG 1                    // 1 = imprime estado/sensores no Serial (115200)
#define MODO_SOMENTE_SENSORES 0    // 1 = motores desligados, só mostra os sensores (teste de bancada)

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

// Pela lógica dos testes anteriores: motor A = roda DIREITA, motor B = roda ESQUERDA
// (rodar só o motor A faz o carro virar para a ESQUERDA).
// Se no teste o carro virar PARA o obstáculo em vez de fugir, troque para true.
const bool INVERTER_LADOS = false;

// ============================================================
// AJUSTES
// ============================================================

// Velocidades (%)
const int VEL_CRUZEIRO  = 85;   // reta livre
const int VEL_APROX     = 55;   // chegando perto de algo
const int VEL_CURVA_EXT = 80;   // roda de fora nas curvas
const int VEL_CURVA_INT = 0;    // roda de dentro (0 = parada, como funcionou melhor; negativo = gira no lugar)
const int VEL_RE        = 60;

// PWM mínimo em que o motor realmente move o carro (abaixo disso só zumbe)
const int PWM_MIN = 80;

// Distâncias do ultrassônico (cm)
const float DIST_ALERTA = 35.0;  // começa a desacelerar
const float DIST_DESVIO = 20.0;  // começa a desviar (curva) sem dar ré
const float DIST_PARE   = 10.0;   // muito perto: ré
const float DIST_LIVRE  = 30.0;  // só considera o caminho livre acima disso (histerese)

// Tempos (ms)
const unsigned long IR_DEBOUNCE_MS      = 30;    // IR precisa ficar ativo esse tempo (filtra ruído de luz)
const unsigned long FOLGA_CURVA_MS      = 150;   // continua curvando um pouco depois do IR liberar
const unsigned long FOLGA_DESVIO_MS     = 150;   // continua desviando um pouco depois de liberar
const unsigned long TEMPO_RE_MS         = 300;
const unsigned long TEMPO_RE_LONGO_MS   = 700;   // quando está preso (várias tentativas seguidas)
const unsigned long TEMPO_DESVIO_MAX_MS = 1500;  // desvio que não libera = tenta de novo pelo outro lado
const unsigned long JANELA_PRESO_MS     = 5000;  // 3 rés dentro dessa janela = "preso"
const unsigned long LADO_RECENTE_MS     = 1500;  // quanto tempo lembra de qual lado o IR viu algo
const unsigned long INTERVALO_US_MS     = 30;    // intervalo entre leituras do ultrassônico
const unsigned long TIMEOUT_ECHO_US     = 5000;  // ~85 cm de alcance
const unsigned long ATRASO_INICIAL_MS   = 1000;  // espera ao ligar (tire a mão). Use 0 para largar direto

// Lado preferido quando não há nenhuma informação dos IR
enum Lado { ESQ, DIR };
const Lado LADO_PADRAO = ESQ;

// ============================================================
// ESTADO
// ============================================================

enum Estado { ANDANDO, RE, DESVIANDO };
Estado estado = ANDANDO;

unsigned long tEstado = 0;
unsigned long ultimaLeitura = 0;
unsigned long ultimoDebug = 0;

// leituras
float distance = 999;
bool obsE = false, obsD = false;
unsigned long irE_desde = 0, irD_desde = 0;   // desde quando o IR está em LOW (0 = livre)
int zerosSeguidos = 0;

// memória do último lado visto pelos IR
bool ultimoObsFoiE = false;
unsigned long tUltimoObs = 0;

// curva lateral (IR)
Lado curvaLado = ESQ;
unsigned long curvaAte = 0;

// desvio / ré
Lado ladoDesvio = ESQ;
Lado proximoPadrao = LADO_PADRAO;
unsigned long livreDesde = 0;
unsigned long tempoReAtual = TEMPO_RE_MS;
unsigned long tUltimaRe = 0;
int tentativas = 0;

// ============================================================
// PWM (funciona no core Arduino-ESP32 2.x e 3.x)
// ============================================================

const int PWM_FREQ = 1000;
const int PWM_BITS = 8;

void pwmInit()
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(ENA, PWM_FREQ, PWM_BITS);
  ledcAttach(ENB, PWM_FREQ, PWM_BITS);
#else
  ledcSetup(0, PWM_FREQ, PWM_BITS);
  ledcAttachPin(ENA, 0);
  ledcSetup(1, PWM_FREQ, PWM_BITS);
  ledcAttachPin(ENB, 1);
#endif
}

void pwmA(int duty)
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(ENA, duty);
#else
  ledcWrite(0, duty);
#endif
}

void pwmB(int duty)
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(ENB, duty);
#else
  ledcWrite(1, duty);
#endif
}

// ============================================================
// MOTORES  (vel: -100 a 100)
// ============================================================

int velParaPwm(int vel)
{
  vel = constrain(abs(vel), 0, 100);
  if (vel == 0) return 0;
  return map(vel, 1, 100, PWM_MIN, 255);
}

// motor A (mesma fiação dos testes: IN2 HIGH = frente)
void motorA(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN1, vel < 0 ? HIGH : LOW);
  digitalWrite(IN2, vel > 0 ? HIGH : LOW);
  pwmA(velParaPwm(vel));
}

// motor B (IN3 HIGH = frente)
void motorB(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN3, vel > 0 ? HIGH : LOW);
  digitalWrite(IN4, vel < 0 ? HIGH : LOW);
  pwmB(velParaPwm(vel));
}

// Controle por roda física: esquerda e direita
void rodas(int esq, int dir)
{
  if (!INVERTER_LADOS)
  {
    motorB(esq);
    motorA(dir);
  }
  else
  {
    motorA(esq);
    motorB(dir);
  }
}

void frente(int p) { rodas(p, p); }
void tras(int p)   { rodas(-p, -p); }
void parar()       { rodas(0, 0); }

// Curva em arco: ESQ = foge para a esquerda (roda direita faz força)
void arco(Lado lado)
{
  if (lado == ESQ) rodas(VEL_CURVA_INT, VEL_CURVA_EXT);
  else             rodas(VEL_CURVA_EXT, VEL_CURVA_INT);
}

Lado oposto(Lado l) { return (l == ESQ) ? DIR : ESQ; }

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
// SENSORES
// ============================================================

void atualizarSensores(unsigned long agora)
{
  // ---- ultrassônico ----
  if (agora - ultimaLeitura >= INTERVALO_US_MS)
  {
    float nova = lerDistancia();

    // Sem eco logo depois de uma leitura próxima, andando para a frente, quase sempre é
    // superfície inclinada (triângulo/círculo) ou obstáculo abaixo da distância mínima do
    // sensor - NÃO é caminho livre. Assume que continua se aproximando por até 3 leituras.
    if (nova >= 999.0)
    {
      if (estado == ANDANDO && zerosSeguidos < 3 && distance < 40.0)
      {
        zerosSeguidos++;
        nova = distance * 0.75;
      }
    }
    else
    {
      zerosSeguidos = 0;
    }

    distance = nova;
    ultimaLeitura = agora;
  }

  // ---- IR (LOW = obstáculo) com debounce por tempo ----
  if (digitalRead(SENSOR_E) == LOW) { if (irE_desde == 0) irE_desde = agora; }
  else irE_desde = 0;

  if (digitalRead(SENSOR_D) == LOW) { if (irD_desde == 0) irD_desde = agora; }
  else irD_desde = 0;

  obsE = (irE_desde != 0) && (agora - irE_desde >= IR_DEBOUNCE_MS);
  obsD = (irD_desde != 0) && (agora - irD_desde >= IR_DEBOUNCE_MS);

  // lembra de qual lado o IR viu algo (só se foi um lado)
  if (obsE && !obsD) { ultimoObsFoiE = true;  tUltimoObs = agora; }
  if (obsD && !obsE) { ultimoObsFoiE = false; tUltimoObs = agora; }
}

// ============================================================
// DECISÃO
// ============================================================

// Escolhe para qual lado fugir
Lado escolherLado(unsigned long agora)
{
  if (obsD && !obsE) return ESQ;   // obstáculo na direita -> foge para a esquerda
  if (obsE && !obsD) return DIR;

  // IR viu algo há pouco: foge do lado onde estava
  if (tUltimoObs != 0 && (agora - tUltimoObs) < LADO_RECENTE_MS)
    return ultimoObsFoiE ? DIR : ESQ;

  // sem informação: alterna
  Lado l = proximoPadrao;
  proximoPadrao = oposto(proximoPadrao);
  return l;
}

void mudarEstado(Estado novo, unsigned long agora)
{
  estado = novo;
  tEstado = agora;
}

void iniciarDesvio(Lado lado, unsigned long agora)
{
  ladoDesvio = lado;
  livreDesde = 0;
  mudarEstado(DESVIANDO, agora);
}

void iniciarRe(Lado ladoDepois, unsigned long agora)
{
  if (agora - tUltimaRe > JANELA_PRESO_MS) tentativas = 0;  // faz tempo que não travava
  tentativas++;
  tUltimaRe = agora;

  tempoReAtual = (tentativas >= 3) ? TEMPO_RE_LONGO_MS : TEMPO_RE_MS;
  ladoDesvio = ladoDepois;
  curvaAte = 0;

  tras(VEL_RE);
  mudarEstado(RE, agora);
}

// Desacelera conforme se aproxima de algo
int velocidadePorDistancia()
{
  if (distance >= DIST_ALERTA) return VEL_CRUZEIRO;
  float f = (distance - DIST_DESVIO) / (DIST_ALERTA - DIST_DESVIO);
  f = constrain(f, 0.0f, 1.0f);
  return VEL_APROX + (int)((VEL_CRUZEIRO - VEL_APROX) * f);
}

#if DEBUG
void debug(unsigned long agora)
{
  if (agora - ultimoDebug < 200) return;
  ultimoDebug = agora;
  const char* nomes[] = {"ANDANDO", "RE", "DESVIANDO"};
  Serial.printf("%-9s dist=%6.1f  E=%d D=%d  lado=%s  tent=%d\n",
                nomes[estado], distance, obsE, obsD,
                ladoDesvio == ESQ ? "ESQ" : "DIR", tentativas);
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

  pwmInit();
  parar();

  delay(ATRASO_INICIAL_MS);
  tEstado = millis();
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

#if MODO_SOMENTE_SENSORES
  parar();
  return;
#endif

  switch (estado)
  {
    // ------------------------------------------------------
    case ANDANDO:
    {
      if (distance < DIST_PARE)
      {
        // encostou: ré e depois desvia
        iniciarRe(escolherLado(agora), agora);
      }
      else if (obsE && obsD && distance < DIST_ALERTA)
      {
        // dois lados + frente fechada: beco/quina
        iniciarRe(escolherLado(agora), agora);
      }
      else if (distance < DIST_DESVIO)
      {
        // obstáculo à frente: já começa a desviar, sem ré
        iniciarDesvio(escolherLado(agora), agora);
      }
      else
      {
        // IR lateral: curva leve para longe do obstáculo
        if (obsD && !obsE)      { curvaLado = ESQ; curvaAte = agora + FOLGA_CURVA_MS; }
        else if (obsE && !obsD) { curvaLado = DIR; curvaAte = agora + FOLGA_CURVA_MS; }

        if (agora < curvaAte)
        {
          arco(curvaLado);
        }
        else
        {
          int vel = velocidadePorDistancia();
          if (obsE && obsD) vel = min(vel, VEL_APROX);   // passagem estreita
          frente(vel);
        }
      }
      break;
    }

    // ------------------------------------------------------
    case RE:
    {
      unsigned long t = agora - tEstado;

      // termina quando passou o tempo E já abriu um pouco de espaço (ou dobra o tempo)
      bool abriuEspaco = distance > (DIST_PARE + 3.0);
      if ((t >= tempoReAtual && abriuEspaco) || t >= tempoReAtual * 2)
      {
        iniciarDesvio(ladoDesvio, agora);
      }
      break;
    }

    // ------------------------------------------------------
    case DESVIANDO:
    {
      arco(ladoDesvio);

      // sensor do lado para onde estamos virando
      bool obsAlvo = (ladoDesvio == ESQ) ? obsE : obsD;
      unsigned long t = agora - tEstado;

      if (distance < DIST_PARE || obsAlvo)
      {
        // bateria de obstáculo na direção da fuga: ré e tenta pelo outro lado
        iniciarRe(oposto(ladoDesvio), agora);
        break;
      }

      bool livre = (distance > DIST_LIVRE) && !obsE && !obsD;

      if (livre)
      {
        if (livreDesde == 0) livreDesde = agora;
        if (agora - livreDesde >= FOLGA_DESVIO_MS)
        {
          mudarEstado(ANDANDO, agora);
          break;
        }
      }
      else
      {
        livreDesde = 0;
      }

      if (t >= TEMPO_DESVIO_MAX_MS)
      {
        // não liberou: ré e tenta pelo outro lado
        iniciarRe(oposto(ladoDesvio), agora);
      }
      break;
    }
  }
}