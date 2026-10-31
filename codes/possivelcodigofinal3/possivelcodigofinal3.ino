// ============================================================
// ROBÔ DESVIADOR DE OBSTÁCULOS - ESP32
// Versão otimizada: máquina de estados sem delay() bloqueante
// ============================================================

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
const float         DIST_MIN_CM       = 10.0;   // abaixo disso = obstáculo à frente
const unsigned long TEMPO_PRESO_MS    = 5000;   // girando/desviando sem sucesso
const unsigned long TEMPO_LIVRE_MS    = 10000;  // andando "livre" por muito tempo (possível travamento)
const unsigned long INTERVALO_US_MS   = 60;     // intervalo entre leituras do ultrassônico
const unsigned long TIMEOUT_ECHO_US   = 8000;   // ~137 cm de alcance máximo
const int           BOOST_CURVA       = 25;     // % extra na roda de fora durante a curva

// ---------- ESTADOS ----------
enum Estado { AVANCANDO, RE, DESVIANDO, GIRANDO_E, GIRANDO_D, AJUSTE };

Estado estado = AVANCANDO;
unsigned long tEstado = 0;        // quando o estado atual começou
unsigned long durEstado = 0;      // duração planejada do estado atual
unsigned long tempoDesvioPend = 0;// duração do desvio que vem depois da ré

unsigned long inicioLivre = 0;    // início do período contínuo de caminho livre
unsigned long inicioProblema = 0; // início do período contínuo com obstáculo lateral
unsigned long ultimaLeitura = 0;

bool desvioE = true;              // alterna a direção preferida do desvio
bool ajusteParaE = false;         // direção do ajuste após terminar uma curva

float distance = 999;
bool obsE = false, obsD = false;

// ============================================================
// MOTORES  (vel: -100 a 100)
// ============================================================

void motorA(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN1, vel < 0 ? HIGH : LOW);
  digitalWrite(IN2, vel > 0 ? HIGH : LOW);
  analogWrite(ENA, abs(vel) * 255 / 100);
}

void motorB(int vel)
{
  vel = constrain(vel, -100, 100);
  digitalWrite(IN3, vel > 0 ? HIGH : LOW);
  digitalWrite(IN4, vel < 0 ? HIGH : LOW);
  analogWrite(ENB, abs(vel) * 255 / 100);
}

void frente(int p)  { motorA(p);  motorB(p); }
void tras(int p)    { motorA(-p); motorB(-p); }
void frenteE(int p) { motorA(p);  motorB(min(100, p + BOOST_CURVA)); }
void frenteD(int p) { motorA(min(100, p + BOOST_CURVA)); motorB(p); }
void parar()        { motorA(0);  motorB(0); }

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

  // mediana de 3: ignora uma leitura ruim isolada
  if ((a >= b && a <= c) || (a <= b && a >= c)) return a;
  if ((b >= a && b <= c) || (b <= a && b >= c)) return b;
  return c;
}

// ============================================================
// AUXILIARES DE ESTADO
// ============================================================

void mudarEstado(Estado novo, unsigned long dur = 0)
{
  estado = novo;
  tEstado = millis();
  durEstado = dur;
}

// Ré + desvio (substitui os 5 blocos repetidos do código original)
void iniciarRe(int velRe, unsigned long tempoRe, unsigned long tempoDesvio)
{
  tras(velRe);
  tempoDesvioPend = tempoDesvio;
  inicioLivre = 0;
  inicioProblema = 0;
  mudarEstado(RE, tempoRe);
}

void iniciarGiro(Estado giro)
{
  mudarEstado(giro);
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

  switch (estado)
  {
    // ------------------------------------------------------
    case AVANCANDO:
    {
      if (distance < DIST_MIN_CM)                 // obstáculo muito perto
      {
        iniciarRe(50, 500, 500);
      }
      else if (obsE && obsD)                      // obstáculos dos dois lados
      {
        iniciarRe(50, 1000, 800);
      }
      else if (obsD || obsE)                      // obstáculo em um lado
      {
        inicioLivre = 0;
        if (inicioProblema == 0) inicioProblema = agora;

        if (agora - inicioProblema >= TEMPO_PRESO_MS)
        {
          iniciarRe(80, 500, 500);                // preso: escapa de ré
        }
        else
        {
          iniciarGiro(obsD ? GIRANDO_D : GIRANDO_E);
        }
      }
      else                                        // caminho livre
      {
        inicioProblema = 0;
        if (inicioLivre == 0) inicioLivre = agora;

        if (agora - inicioLivre >= TEMPO_LIVRE_MS)
        {
          iniciarRe(80, 500, 500);                // possível travamento físico
        }
        else
        {
          frente(100);
        }
      }
      break;
    }

    // ------------------------------------------------------
    case RE:
    {
      if (agora - tEstado >= durEstado)
      {
        if (desvioE && !obsE)       { frenteE(80); mudarEstado(DESVIANDO, tempoDesvioPend); }
        else if (!obsD)             { frenteD(80); mudarEstado(DESVIANDO, tempoDesvioPend); }
        else                        { mudarEstado(AVANCANDO); }

        desvioE = !desvioE;
      }
      break;
    }

    // ------------------------------------------------------
    case DESVIANDO:
    {
      if (agora - tEstado >= durEstado) mudarEstado(AVANCANDO);
      break;
    }

    // ------------------------------------------------------
    case GIRANDO_D:   // gira até o obstáculo sair do sensor direito
    case GIRANDO_E:   // gira até o obstáculo sair do sensor esquerdo
    {
      bool giraD = (estado == GIRANDO_D);
      bool aindaBloqueado = giraD ? obsD : obsE;
      bool outroLado      = giraD ? obsE : obsD;

      if (giraD) frenteD(50); else frenteE(50);

      if (agora - inicioProblema >= TEMPO_PRESO_MS || outroLado || distance < DIST_MIN_CM)
      {
        mudarEstado(AVANCANDO);                   // reavalia a situação
      }
      else if (!aindaBloqueado)
      {
        unsigned long ajuste = (agora - tEstado) / 2;
        ajusteParaE = giraD;                      // corrige girando para o lado oposto
        if (ajuste > 0)
        {
          if (ajusteParaE) frenteE(50); else frenteD(50);
          mudarEstado(AJUSTE, ajuste);
        }
        else
        {
          mudarEstado(AVANCANDO);
        }
      }
      break;
    }

    // ------------------------------------------------------
    case AJUSTE:
    {
      if (agora - tEstado >= durEstado) mudarEstado(AVANCANDO);
      break;
    }
  }
}
