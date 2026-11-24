#include <Arduino.h>

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
#define ENA 32
#define ENB 14

// ==================
// VARIÁVEIS GLOBAIS
// ==================

long duration;
float distance;
int pwm_adj = 253;

// ======
// SETUP
// ======

void setup()
{
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

// ====================
// FUNÇÕES DOS MOTORES
// ====================

void frente(int porc)
{

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void tras(int porc)
{

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void frenteE(int porc)
{

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void frenteD(int porc)
{

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void trasE(int porc)
{

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void trasD(int porc)
{

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    // Velocidade PWM
    int velA = (porc * 255) / 100;
    int velB = (porc * pwm_adj) / 100;
    analogWrite(ENA, velA);
    analogWrite(ENB, velB);
}

void parar()
{

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    // Velocidade PWM
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

// =====
// LOOP
// =====

void loop()
{
    // ========================
    // LEITURA DO ULTRASSÔNICO
    // ========================

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0)
    {
        distance = 999;
    }
    else
    {
        distance = duration * 0.0343 / 2;
    }

    // ===============
    // LEITURA DOS IR
    // ===============

    int IR_E = digitalRead(SENSOR_E);
    int IR_D = digitalRead(SENSOR_D);

    // ==================
    // LÓGICA DE DECISÃO
    // ==================

    if (distance < 10)
    {
        tras(100);
    }
    else if (distance < 30)
    {
        if (IR_E == LOW)
        {
            frenteD(80);
        }
        else if (IR_D == LOW)
        {
            frenteE(80);
        }
        else if (IR_E == LOW && IR_D == LOW)
        {
            tras(100);
        }
        else
        {
            frente(100);
        }
    }
    else
    {
        frente(100);
    }
}