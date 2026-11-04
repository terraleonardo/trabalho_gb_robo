/*
Universidade do Vale do Rio dos Sinos - UNISINOS
Curso: Engeharia Elétrica
Disciplina: Circuitos Microprocessados - GR16047-00245-979999
Professor: Dr. Jean Schmith
Autor: Regis Gallas
Projeto: Atividade GB - Projeto Robô AGV
Data: 16/06/26
*/
/*
TAREFAS:
• O robô deverá movimentar-se em um grid demarcado no chão. O exemplo
do grid pode ser verificado na figura 1 (matriz 4x4).
• O robô deverá executar um caminho específico que será sorteado. Cada
grupo terá um caminho diferente para ser executado.
• Os robôs podem e devem trafegar no grid AO MESMO TEMPO.
• O robô deve possuir sistemas de anti colisão.
• O robô deve ser capaz de tomar decisões para chegar no seu objetivo.
• Para a simulação de obstáculos ou outros robôs serão utilizadas barreiras
aritficiais
*/
/* ESTADOS DA MÁQUINA DE ESTADOS */
#define SEGUIR_LINHA 0
#define CURVA 1
#define PARADA 2
/* DEFINIÇÃO PINOS */
#define LED_DEBUG_1 13 // LED debug 1
#define LED_DEBUG_2 4 // LED debug 2
#define IR_CONTADOR_LINHA 3 // pino DO - Módulo Seguidor de Linha Sensor Óptico TCRT 5000 - lado direito atrás roda (saída em 1)
#define IR_OBSTACULO 2 // pino DO - Módulo Sensor de Obstáculo Infravermelho IR - frontal (saída em 0)
#define IR_SEGUIDOR_DIREITO 11 // pino DO - Módulo Seguidor de Linha Sensor Óptico TCRT 5000 - frontal lado direito (saída em 1)
#define IR_SEGUIDOR_ESQUERDO 12 // pino DO - Módulo Seguidor de Linha Sensor Óptico TCRT 5000 - frontal lado esquerdo (saída em 1)
#define PWM_MOTOR_DIREITO 5 // pino 1 ENA - Enable A ponte H - saída 1 PWM motor - lado direito (valor entre 0 ~ 255)
#define PWM_MOTOR_ESQUERDO 6 // pino 6 ENB - Enable B ponte H - saída 2 PWM motor - lado esquerdo (valor entre 0 ~ 255)
#define MOTOR_DIREITO_IN_0 8 // pino 2 IN1 - Input 1 ponte H - saída sentido giro horário - lado direito (roda para frente)
#define MOTOR_DIREITO_IN_1 7 // pino 3 IN2 - Input 2 ponte H - saída sentido giro anti-horário - lado direito (roda para trás)
#define MOTOR_ESQUERDO_IN_0 10 // pino 4 IN3 - Input 3 ponte H - saída sentido giro horário - lado esquerdo (roda para frente)
#define MOTOR_ESQUERDO_IN_1 9 // pino 5 IN4 - Input 4 ponte H - saída sentido giro anti-horário - lado esquerdo (roda para trás)
/*VARIÁVEIS GLOBAIS*/
int estado = SEGUIR_LINHA; // estado inicial da máquina de estados
int iCount = 0; // reseta todas as variaveis globais
int isObstaculo = 0;
int iSensor_esq = 0;
int iSensor_dir = 0;
int iCount_ok = 0;
int iCurva_ok = 0;
/* FUNÇÃO SETUP ARDUINO */
void setup() {
pinMode(LED_DEBUG_1, OUTPUT); // configura I/O digital pino 13 como saída
pinMode(LED_DEBUG_2, OUTPUT); // configura I/O digital pino 4 como saída
pinMode(IR_CONTADOR_LINHA, INPUT); // configura I/O digital pino 3 como entrada
pinMode(IR_OBSTACULO, INPUT); // configura I/O digital pino 2 como entrada
pinMode(IR_SEGUIDOR_DIREITO, INPUT); // configura I/O digital pino 11 como entrada
pinMode(IR_SEGUIDOR_ESQUERDO, INPUT); // configura I/O digital pino 12 como entrada
pinMode(PWM_MOTOR_DIREITO, OUTPUT); // configura I/O digital pino 5 como saída
pinMode(PWM_MOTOR_ESQUERDO, OUTPUT); // configura I/O digital pino 6 como saída
pinMode(MOTOR_DIREITO_IN_0, OUTPUT); // configura I/O digital pino 8 como saída
pinMode(MOTOR_DIREITO_IN_1, OUTPUT); // configura I/O digital pino 7 como saída
pinMode(MOTOR_ESQUERDO_IN_0, OUTPUT); // configura I/O digital pino 10 como saída
pinMode(MOTOR_ESQUERDO_IN_1, OUTPUT); // configura I/O digital pino 9 como saída
attachInterrupt(digitalPinToInterrupt(IR_OBSTACULO), obstaculo, CHANGE);
// entrada ativada pela borda - chama função "obstaculo"
attachInterrupt(digitalPinToInterrupt(IR_CONTADOR_LINHA), contador_linha, FALLING); // entrada ativada por pulso - chama função "contador_linha"
}
/* FUNÇÃO LOOP ARDUINO */
void loop() {
if(iCount_ok) // liga LED Debug se contador ok
digitalWrite(LED_DEBUG_1, HIGH);
else
digitalWrite(LED_DEBUG_1, LOW);
if(iCurva_ok)
digitalWrite(LED_DEBUG_2, HIGH); // liga LED Debug se curva ok
else
digitalWrite(LED_DEBUG_2, LOW);
switch(estado) // seleção máquina de estados
{
case SEGUIR_LINHA: // estado 1 - SEGUIR_LINHA
seguidor_de_linha(); // chama a função "seguidor_de_linha" - robô AGV deve seguir para frente
break;
case CURVA: // estado 2 - CURVA
realizar_a_curva(); // chama a função "realizar_a_curva" - robô AGV deve seguir para direita, após contagem da linha
break;
case PARADA: // estado 3 - PARADA
parar(); // chama a função "parar" - robô AGV deve parar
break;
}
}
/*FUNÇÃO SEGUIDOR DE LINHA AGV*/
void seguidor_de_linha()
{
int iSensor_esq = digitalRead(IR_SEGUIDOR_ESQUERDO); // leitura do sensor frontal lado esquerdo (saída em 1 - linha preta)
int iSensor_dir = digitalRead(IR_SEGUIDOR_DIREITO); // leitura do sensor frontal lado direito (saída em 1 - linha preta)
if(iSensor_esq==0 && iSensor_dir==0) // se sensor lado esquerda refletir IR (super. branca) e sensor lado direito refletir IR (super. branca)
seguir_frente(); // chama a função "seguir_frente"
else if(iSensor_esq==1 && iSensor_dir==0) // se sensor lado esquerda absorver IR (linha preta) e sensor lado direito refletir IR (super. branca)
virar_esquerda(); // chama a função "virar_esquerda"
else if(iSensor_esq==0 && iSensor_dir==1) // se sensor lado esquerda refletir IR (super. branca) e sensor lado direito absorver IR (linha preta)
virar_direita(); // chama a função "virar_direita"
else // se sensor lado esquerda absorver IR (linha preta) e sensor lado direito absorver IR (linha preta)
parar(); // chama a função "parar"
}
/*FUNÇÃO REALIZAR A CURVA PARA DIREITA AGV*/
void realizar_a_curva()
{
int iSensor_esq = digitalRead(IR_SEGUIDOR_ESQUERDO); // leitura do sensor frontal lado esquerdo (saída em 1 - linha preta)
int iSensor_dir = digitalRead(IR_SEGUIDOR_DIREITO); // leitura do sensor frontal lado direito (saída em 1 - linha preta)
re(); // chama a função "re".
// como a contagem da linha acontece depois de passar a linha vertical (sensor atrás da roda direita)
// definido que o robô AGV precisa dar uma ré a esquerda, roda esquerda parada e roda direta para trás
// até sensor da direita encontrar a linha preta vertical
if(iSensor_dir==1){ // sensor lado direito absorver IR (encontrar a linha preta)
estado = SEGUIR_LINHA; // altera para o estado SEGUIR_LINHA
iCurva_ok = 1; // ativa a variável auxiliar curva ok
}
}
/*FUNÇÃO CONTADOR DE LINHA AGV*/
void contador_linha()
{
if(iCount_ok) // se a contagem já foi executada, retorna
return;
iCount++; // incrementa a váriavel contador de linha
if(iCount == 3){
estado = CURVA; // quando contabilizar 4 linhas, altera para o estado CURVA
iCount = 0; // reseta a variável contador de linha
iCount_ok = 1; // ativa a variável auxiliar contador ok
}
}
/*FUNÇÃO IDENTIFICAR OBSTÁCULO A FRENTE*/
void obstaculo()
{
isObstaculo =! isObstaculo; // troca o estado da variável obstáculo, baseado na entrada de interrupção
if(isObstaculo){ // se for verdadeiro
estado = PARADA; // altera para o estado PARADA
}
else{ // se for falso
estado = SEGUIR_LINHA; // altera para o estado SEGUIR_LINHA
}
}
/*FUNÇÃO PARAR AGV*/
void parar()
{
analogWrite(PWM_MOTOR_DIREITO, 0); // valor PWM motor direito - 0
analogWrite(PWM_MOTOR_ESQUERDO, 0); // valor PWM motor esquerdo - 0
digitalWrite(MOTOR_DIREITO_IN_0, LOW); // roda esquerda parada
digitalWrite(MOTOR_DIREITO_IN_1, LOW);
digitalWrite(MOTOR_ESQUERDO_IN_0, LOW); // roda esquerda parada
digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}
/*FUNÇÃO SEGUIR EM FRENTE AGV*/
void seguir_frente()
{
analogWrite(PWM_MOTOR_DIREITO, 200); // valor PWM motor direito - 100
analogWrite(PWM_MOTOR_ESQUERDO, 200); // valor PWM motor esquerdo - 100
digitalWrite(MOTOR_DIREITO_IN_0, HIGH); // sentido giro horário - lado direito (roda para frente)
digitalWrite(MOTOR_DIREITO_IN_1, LOW);
digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); // sentido giro horário - lado esquerdo (roda para frente)
digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}
/*FUNÇÃO VIRAR A ESQUERDA AGV*/
void virar_esquerda()
{
analogWrite(PWM_MOTOR_DIREITO, 100); // valor PWM motor direito - 100
analogWrite(PWM_MOTOR_ESQUERDO, 100); // valor PWM motor esquerdo - 100
digitalWrite(MOTOR_DIREITO_IN_0, HIGH); // sentido giro horário - lado direito (roda para frente)
digitalWrite(MOTOR_DIREITO_IN_1, LOW);
digitalWrite(MOTOR_ESQUERDO_IN_0, LOW);
digitalWrite(MOTOR_ESQUERDO_IN_1, HIGH); // sentido giro anti-horário - lado esquerdo (roda para trás)
}
/*FUNÇÃO VIRAR A DIREITA AGV*/
void virar_direita()
{
analogWrite(PWM_MOTOR_DIREITO, 100); // valor PWM motor direito - 100
analogWrite(PWM_MOTOR_ESQUERDO, 100); // valor PWM motor esquerdo - 100
digitalWrite(MOTOR_DIREITO_IN_0, LOW);
digitalWrite(MOTOR_DIREITO_IN_1, HIGH); // sentido giro anti-horário - lado direito (roda para trás)
digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); // sentido giro horário - lado esquerdo (roda para frente)
digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}
/*FUNÇÃO RÉ AGV*/
void re()
{
analogWrite(PWM_MOTOR_DIREITO, 135); // valor PWM motor direito - 135
analogWrite(PWM_MOTOR_ESQUERDO, 0); // valor PWM motor esquerdo - 0
digitalWrite(MOTOR_DIREITO_IN_0, LOW);
digitalWrite(MOTOR_DIREITO_IN_1, HIGH); // sentido giro anti-horário - lado direito (roda para trás)
digitalWrite(MOTOR_ESQUERDO_IN_0, LOW); // roda esquerda parada
digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}
/*
INFORMAÇÕES COMPLEMENTARES:
1. Módulo Seguidor de Linha Sensor Óptico TCRT 5000
DO - saída em 1 - linha preta (sem reflexo - absorve a luz infravermelha)
DO - saída em 0 - superfície branca (com reflexo - reflete a luz infravermelha)
2. Informação Módulo Sensor de Obstáculo Infravermelho IR KY-032
DO - saída em 1 - não obstáculo
DO - saída em 0 - obstáculo
*/