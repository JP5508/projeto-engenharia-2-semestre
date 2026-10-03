//terceiro teste dos sensores
//sensor us e sonsores ir

//definindo os pinos
#define TRIG_PIN 4
#define ECHO_PIN 15
#define SENSOR_L 5
#define SENSOR_R 18
#define LED 2

//definindo as variaveis
long duration;
float distance;

void setup(){
  //configurando os pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SENSOR_L, INPUT);
  pinMode(SENSOR_R, INPUT);
  pinMode(LED, OUTPUT);
  //pisca led de inicialização
  for(int i=0; i>3; i++){
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
  }
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
  //SENSOR IR
  //Lendo os sensores
  int valor_IR_L = digitalRead(SENSOR_L);
  int valor_IR_R = digitalRead(SENSOR_R);
  //ACENDER LED
  //quando o sensor detecta algo ele fica LOW
  if((distance < 10)||(valor_IR_L == LOW)||(valor_IR_R == LOW)){
    digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED, LOW);
  }
  delay(100);
}
