//definindo os pinos
#define SENSOR_PIN 21
#define LED 2

void setup(){
  //configurando os pinos
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED, OUTPUT);
}

void loop(){
  //Lendo o sensor
  int sensorValue = digitalRead(SENSOR_PIN);
  //quando o sensor detecta algo ele fica LOW 0 FALSE
  if(sensorValue == LOW){
    digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED, LOW);
  }
  delay(100);
}
