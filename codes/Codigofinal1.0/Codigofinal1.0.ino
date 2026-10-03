//DEFININDO SENSOR US
#define TRIG_PIN 5
#define ECHO_PIN 18

//DEFININDO SENSORES IR
#define SENSOR_E 35
#define SENSOR_D 34

//DEFININDO MOTORES
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33

//DEFININDO CONTROLADORES DE VELOCIDADE
#define ENA 14
#define ENB 32

//DEFININDO LED DO ESP
#define LED 2

//VARIAVEIS GLOBAIS
long duration;
float distance;

//CONFIGURAÇÃO INICIAL
void setup() {
  
  //SERIAL E LED DO ESP
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  
  //SENSOR US
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  //SENSORES IR
  pinMode(SENSOR_E, INPUT);
  pinMode(SENSOR_D, INPUT);
  
  //MOTORES
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  //VELOCIDADE
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  //PARAR
  parar();
}

//LOOP
void loop() {
  
  // =========================
  // LEITURA DO US
  // =========================
  
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

  // =========================
  // LEITURA DOS IR
  // =========================

  int valor_IR_E = digitalRead(SENSOR_E);
  int valor_IR_D = digitalRead(SENSOR_D);

  // =========================
  // DECISÃO
  // =========================

  //OBSTÁCULO PERTO
  if (distance < 10) {

    // Obstáculo na esquerda
    if (valor_IR_E == LOW && valor_IR_D == HIGH) {
      virarDireita();
    }

    // Obstáculo na direita
    else if (valor_IR_E == HIGH && valor_IR_D == LOW) {
      virarEsquerda();
    }

    // Obstáculo na frente
    else if (valor_IR_E == LOW && valor_IR_D == LOW) {
      frente();
    }

    // Sem obstáculos
    else {
      frente();
    }
  }

  //OBSTÁCULO LONGE
  else {

    // Obstáculo na esquerda
    if (valor_IR_E == LOW && valor_IR_D == HIGH) {
      virarDireita();
    }

    // Obstáculo na direita
    else if (valor_IR_E == HIGH && valor_IR_D == LOW) {
      virarEsquerda();
    }

    // Sem obstáculos
    else {
      frente();
    }
  }
/*
  // Debug
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.print(" | IR E: ");
  Serial.print(valor_IR_E);
  Serial.print(" | IR D: ");
  Serial.println(valor_IR_D);
*/
  delay(50);
}


// =================================
// FUNÇÕES DOS MOTORES LH-HL
// =================================

void frente() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 100);
  analogWrite(ENB, 100);

  digitalWrite(LED, HIGH);
}

void tras() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
}

void virarEsquerda() {

  // Motor esquerdo anda
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Motor direito para
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 255);
  analogWrite(ENB, 0);
}

void virarDireita() {
  
  // Motor esquerdo para
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  
  // Motor direito anda
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 255);
}

void parar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(LED, LOW);
}
