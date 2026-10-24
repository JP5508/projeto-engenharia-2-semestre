# projeto engenharia da computação 2º semestre

Este repositório contém a documentação e códigos do **projeto robô desviador de obstáculos** desenvolvido pelo grupo **MK2** no 2° semestre de engenharia da computação no senac.

----

## EQUIPE
- Artur Oliveira Nunes
- João Pedro Alves Oliveira
- Mateus Gueiros Luiz
- Renan Rocha Monteiro

Centro Universitário Senac Santo Amaro — Bacharelado em Engenharia da Computação
Ano letivo: 2026

----

## 📘 Descrição do Projeto
O projeto é composto por duas partes integradas:

### 🪫 HARDWARE
Um robô/veículo autônomo montado nas seguintes especificações:
- Microcontrolador ESP32
- 2 motores DC na traseira do veículo;
- 1 roda boba na frente;
- Ponte H para controle de velocidade;
- 2 sensores infravermelhos nas laterais;
- 1 sensor ultrassônico na frente;
- Alimentado por 2 baterias de 4.2V;
- Chassi em MDF.

### 💻 SOFTWARE
Código criado em **C++** para o **ESP32**, com as seguintes funcionalidades:
- Mover para frente e para os lado;
- Controle da velocidade dos motores;
- Detecção de obstáculos pelos sensores infravermelhos;
- Detecção de obstáculos pelo sensor ultrassônico;
- Lógica de decisão para desvio de obstáculos.