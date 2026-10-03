// 10º Código
// Leitura dos sensores
// Lógica de decisão
// Controle de velocidade

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

// ==================
// VARIÁVEIS GLOBAIS
// ==================

long duration;
float distance;

// ======
// SETUP
// ======

void setup() {

  //ESP
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

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

// =====
// LOOP
// =====

void loop() {

  // ========================
  // LEITURA DO ULTRASSÔNICO
  // ========================

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    distance = 999;
  } else {
    distance = duration * 0.0343 / 2;
  }

  // ===============
  // LEITURA DOS IR
  // ===============

  int valor_IR_E = digitalRead(SENSOR_E);
  int valor_IR_D = digitalRead(SENSOR_D);

  // =================================
  // OBSTÁCULO MUITO PRÓXIMO À FRENTE
  // =================================

  if (distance < 10) {

    tras(255);
    delay(300);

    // Se o lado direito está bloqueado,
    // tenta escapar pela esquerda
    if (valor_IR_D == LOW) {
      virarEsquerda(80);
    }

    // Caso contrário, tenta pela direita
    else {
      virarDireita(80);
    }

    delay(500);
  }

  // ===================
  // OBSTÁCULO À FRENTE
  // ===================

  else if (distance < 20) {

    // Direito detectou → esquerda
    if (valor_IR_D == LOW) {
      virarEsquerda(80);
      delay(250);
    }

    // Esquerdo detectou → direita
    else if (valor_IR_E == LOW) {
      virarDireita(80);
      delay(250);
    }

    // US viu obstáculo, mas IR não indicou
    // lado → começa a procurar uma saída
    else {
      virarEsquerda(80);
      delay(300);
    }
  }

  // ======================
  // CAMINHO FRONTAL LIVRE
  // ======================

  else {

    // Obstáculo diagonal à direita
    if (valor_IR_D == LOW) {

      virarEsquerda(80);
      delay(150);
    }

    // Obstáculo diagonal à esquerda
    else if (valor_IR_E == LOW) {

      virarDireita(80);
      delay(150);
    }

    // Tudo livre
    else {

      frente(127);
    }
  }


  delay(50);
}

// ====================
// FUNÇÕES DOS MOTORES
// ====================

void frente(int velocidade) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(LED, HIGH);

  // Velocidade PWM
  analogWrite(ENA, velocidade);
  analogWrite(ENB, velocidade);
}

void tras(int velocidade) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Velocidade PWM
  analogWrite(ENA, velocidade);
  analogWrite(ENB, velocidade);
}

void virarEsquerda(int velocidade) {

  // Motor esquerdo anda
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Motor direito para
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  analogWrite(ENA, velocidade);
  analogWrite(ENB, velocidade);
}

void virarDireita(int velocidade) {

  // Motor esquerdo para
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // Motor direito anda
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  analogWrite(ENA, velocidade);
  analogWrite(ENB, velocidade);
}

void parar() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  digitalWrite(LED, LOW);

  // Velocidade PWM
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
