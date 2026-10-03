// ====================
// DEFINIÇÃO DOS PINOS
// ====================

// MOTORES
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33

// CONTROLE DE VELOCIDADE
#define ENA 14
#define ENB 32

// LED
#define LED 2


void setup() {

  // Pinos de direção
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(LED, OUTPUT);
  Serial.begin(115200);
}


void loop() {

  Serial.println("Velocidade 25%");
  frente(64);
  delay(3000);

  parar();
  delay(1000);


  Serial.println("Velocidade 50%");
  frente(128);
  delay(3000);

  parar();
  delay(1000);


  Serial.println("Velocidade 75%");
  frente(191);
  delay(3000);

  parar();
  delay(1000);


  Serial.println("Velocidade 100%");
  frente(255);
  delay(3000);

  parar();
  delay(2000);
}


// ====================
// ANDAR PARA FRENTE
// ====================

void frente(int velocidade) {

  // Motor A
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Motor B
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // Velocidade PWM
  analogWrite(ENA, velocidade);
  analogWrite(ENB, velocidade);
}


// ====================
// PARAR
// ====================

void parar() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
