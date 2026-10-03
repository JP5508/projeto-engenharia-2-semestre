//definindo os pinos
#define TRIG_PIN 21
#define ECHO_PIN 19
#define LED 2

//definindo as variaveis
long duration;
float distance;

void setup(){
  //configurando os pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED, OUTPUT);
}

void loop(){
  //trig desativado por 2ms
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  //trig ativado por 10ms
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  //calcula o tempo que o echo ficou em high
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  //funcionalidade do LED
  if (distance < 10){
    digitalWrite(LED,HIGH);
  }else {
    digitalWrite(LED,LOW);
  }
}
