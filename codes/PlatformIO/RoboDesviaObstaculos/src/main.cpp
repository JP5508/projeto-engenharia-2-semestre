
#include <Arduino.h>

 
    // ==================== // DEFINICAO DOS PINOS // ====================
 
    // Sensor ultrassonico #define TRIG_PIN 5 #define ECHO_PIN 18
 
    // Sensores infravermelhos #define SENSOR_E 35 #define SENSOR_D 34
 
    // Motores - L298N #define IN1 25 #define IN2 26 #define IN3 27 #define IN4 33
 
    // PWM #define ENA 14 #define ENB 32
 
    // LED integrado do ESP32 #define LED 2
 
    // ==================== // VELOCIDADES // ====================
 
    // Ajustaveis de 0 a 100% const int VEL_RETA = 100; const int VEL_APROXIMACAO = 65; const int VEL_CURVA = 75; const int VEL_RE = 40;
 
    // ==================== // VARIAVEIS // ====================
  long duration;
float distance;
  bool reJaExecutada = false;
 
    // ==================== // LEITURA ULTRASSONICA // ====================
  float medirDistancia()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
   
  digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
   
  digitalWrite(TRIG_PIN, LOW);
   
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
   
  if (duration == 0)  
  {
        return 999;
     
  }
   
  return duration * 0.0343 / 2;
}
 
    // ==================== // CONTROLE DOS MOTORES // ====================
  int pwm(int porcentagem)
{
    return map(constrain(porcentagem, 0, 100),              0, 100, 0, 255);
}
  void frente(int velocidade)
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
   
  digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
   
  analogWrite(ENA, pwm(velocidade));
    analogWrite(ENB, pwm(velocidade));
   
  digitalWrite(LED, HIGH);
}
  void tras(int velocidade)
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
   
  digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
   
  analogWrite(ENA, pwm(velocidade));
    analogWrite(ENB, pwm(velocidade));
   
  digitalWrite(LED, LOW);
}
 
    // Mantem o mesmo sentido de giro da // funcao virarEsquerda da programacao antiga. void virarEsquerda(int velocidade) {   // Motor esquerdo ativo   digitalWrite(IN1, LOW);   digitalWrite(IN2, HIGH);
 
  // Motor direito parado   digitalWrite(IN3, LOW);   digitalWrite(IN4, LOW);
 
  analogWrite(ENA, pwm(velocidade));
  analogWrite(ENB, 0);
}
 
    // Mantem o mesmo sentido de giro da // funcao virarDireita da programacao antiga. void virarDireita(int velocidade) {   // Motor esquerdo parado   digitalWrite(IN1, LOW);   digitalWrite(IN2, LOW);
 
  // Motor direito ativo   digitalWrite(IN3, HIGH);   digitalWrite(IN4, LOW);
 
  analogWrite(ENA, 0);
  analogWrite(ENB, pwm(velocidade));
}
  void parar()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
   
  digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
   
  analogWrite(ENA, 0);
    analogWrite(ENB, 0);
   
  digitalWrite(LED, LOW);
}
 
    // ==================== // SETUP // ====================
  void setup()
{
    Serial.begin(115200);
   
  pinMode(LED, OUTPUT);
   
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
   
  Serial.println("Robo autonomo com PWM iniciado");
}
 
    // ==================== // LOOP PRINCIPAL // ====================
  void loop()
{
    // Leitura dos sensores   distance = medirDistancia();
 
  int valor_IR_E = digitalRead(SENSOR_E);
    int valor_IR_D = digitalRead(SENSOR_D);
   
  // Monitor serial   Serial.print("Distancia: ");   Serial.print(distance);   Serial.print(" cm | IR E: ");   Serial.print(valor_IR_E);   Serial.print(" | IR D: ");   Serial.println(valor_IR_D);
 
  // Emergencia: os tres sensores detectam   bool emergencia =       distance < 10 &&       valor_IR_E == LOW &&       valor_IR_D == LOW;
 
  // ==========================   // 1. RE DE EMERGENCIA   // ==========================
 
  if (emergencia)  
  {
        if (!reJaExecutada)    
    {
            Serial.println("Emergencia: recuando");
       
      parar();
            delay(100);
       
      tras(VEL_RE);
            delay(300);
       
      parar();
            delay(100);
       
      // A re nao sera repetida enquanto       // a mesma condicao continuar ativa.       reJaExecutada = true;
 
      // O IR direito esta detectando:       // tenta escapar pela esquerda.       virarEsquerda(VEL_CURVA);       delay(500);     }     else     {       // Continua tentando escapar sem       // executar outra marcha a re.       virarEsquerda(VEL_CURVA);     }   }
 
  // Libera uma nova re somente depois   // que a condicao de emergencia desaparecer.   else   {     reJaExecutada = false;
 
    // ==========================     // 2. ULTRASSONICO ABAIXO DE 10 CM     // ==========================
 
    if (distance < 10)    
      {
              Serial.println("Obstaculo muito proximo");
         
      if (valor_IR_D == LOW)      
        {
                  virarEsquerda(VEL_CURVA);
               
        }
              else      
        {
                  virarDireita(VEL_CURVA);
               
        }
         
      delay(300);
           
      }
       
    // ==========================     // 3. ULTRASSONICO ABAIXO DE 20 CM     // ==========================
 
    else if (distance < 20)    
      {
              Serial.println("Obstaculo frontal");
         
      if (valor_IR_D == LOW)      
        {
                  virarEsquerda(VEL_CURVA);
                  delay(250);
               
        }
              else if (valor_IR_E == LOW)      
        {
                  virarDireita(VEL_CURVA);
                  delay(250);
               
        }
              else      
        {
                  // Procurar uma saida         virarEsquerda(VEL_CURVA);         delay(300);       }     }
 
    // ==========================     // 4. CAMINHO FRONTAL MAIS LIVRE     // ==========================
 
    else    
          {
                  // Obstaculo diagonal a direita       if (valor_IR_D == LOW)       {         virarEsquerda(VEL_CURVA);         delay(150);       }
 
      // Obstaculo diagonal a esquerda       else if (valor_IR_E == LOW)       {         virarDireita(VEL_CURVA);         delay(150);       }
 
      // Caminho livre       else       {         if (distance < 30)         {           frente(VEL_APROXIMACAO);         }         else         {           frente(VEL_RETA);         }       }     }   }
 
  delay(20);
          }
           
