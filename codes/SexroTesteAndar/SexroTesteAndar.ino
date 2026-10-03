//sexto teste
//andar

//definindo os pinos
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SENSOR_E 35
#define SENSOR_D 34
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 33

//definindo as variaveis
long duration;
float distance;

void setup(){
  //configurando os pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SENSOR_E, INPUT);
  pinMode(SENSOR_D, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop(){
  E_frente();
}

void E_frente(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}
