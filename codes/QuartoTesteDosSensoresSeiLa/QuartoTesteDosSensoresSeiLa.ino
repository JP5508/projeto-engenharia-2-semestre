//quarto teste dos sensores
//sei la

//definindo os pinos
#define TRIG_PIN 4
#define ECHO_PIN 15
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
  //SENSOR US
  //trig desativado por 2ms
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  //trig ativado por 10ms
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  //calcula o tempo que o echo ficou em high
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  distance = duration * 0.034 / 2;
  //ACENDER LED
  //quando o sensor detecta algo ele fica LOW
  if(distance < 10){
    digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED, LOW);
  }
  delay(100);
}
