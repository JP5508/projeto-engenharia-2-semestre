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

unsigned long inicio_estado, fim_estado;

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

  parar();
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

  // ==================
  // LÓGICA DE DECISÃO
  // ==================

    //OBSTACULO MUITO PERTO
  if (distance < 10) {
    tras(50);
    delay(500);
  } else if (valor_IR_E == HIGH && valor_IR_D == HIGH) {
    frente(100);

    //OBSTACULOS AO REDOR
  } else if (valor_IR_E == LOW && valor_IR_D == LOW) {
    parar();
    
    //OBSTACULO NO DIREITA
  } else if (valor_IR_E == HIGH && valor_IR_D == LOW) {
    //vira até o obstaculo sair do sensor direito
    inicio_estado = millis();
    while(valor_IR_D == LOW){
    frenteE(50);
    valor_IR_D = digitalRead(SENSOR_D);
    } fim_estado = millis() - inicio_estado;
    frenteD(50);
    delay(fim_estado);

    //OBSTACULO NA ESQUERDA
  } else if (valor_IR_E == LOW && valor_IR_D == HIGH) {
    //vira até o obstaculo sair do sensor esquerdo
    inicio_estado = millis();
    while(valor_IR_E == LOW){
    frenteD(50);
    valor_IR_E = digitalRead(SENSOR_E);
    } fim_estado = millis() - inicio_estado;
    frenteE(50);
    delay(fim_estado);
  }
  
  /*
  inicio_estado = millis()
  fim_estado = millis() - inicio_estado;
  */
//fim do loop
}

// ====================
// FUNÇÕES DOS MOTORES
// ====================

void frente(int porc) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(LED, HIGH);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void tras(int porc) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void frenteE(int porc) {

  // Motor esquerdo anda
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Motor direito para
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void frenteD(int porc) {

  // Motor esquerdo para
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // Motor direito anda
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}
void trasE(int porc) {

  // Motor esquerdo anda
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Motor direito para
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

void trasD(int porc) {

  // Motor esquerdo para
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // Motor direito anda
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // Velocidade PWM
  int vel = (porc * 255) / 100;
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
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
