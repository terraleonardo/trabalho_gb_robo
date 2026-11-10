//Estados da maquina de estados
#define SEGUIR_LINHA 0
#define CURVA 1
#define PARADA 2

//Pinos
#define LED_DEBUG 13
#define IR_CONTADOR_LINHA 3
#define IR_OBSTACULO 2
#define IR_SEGUIDOR_DIREITO 11
#define IR_SEGUIDOR_ESQUERDO 12
#define PWM_MOTOR_DIREITO 5
#define PWM_MOTOR_ESQUERDO 6
#define MOTOR_DIREITO_IN_0 8
#define MOTOR_DIREITO_IN_1 7
#define MOTOR_ESQUERDO_IN_0 10
#define MOTOR_ESQUERDO_IN_1 9

//Variaveis globais
volatile int estado = SEGUIR_LINHA;
volatile int iCount = 0;
volatile int isObstaculo = 0;

volatile int ultimoCount = 0;


//===Variaveis locais do seguidor_de_linha()===

#define VELOCIDADE_BASE 200 // DEFINITIVO
#define VELOCIDADE_CORRECAO 100 // DEFINITIVO

//=============================================

////===Variaveis locais do realizar_a_curva===
#define VELOCIDADE_GIRO 150     // menor que a base: reduz inércia e overshoot
#define TEMPO_MIN_GIRO 600      // ms para sair da linha antiga (ajustar)
#define TIMEOUT_GIRO 2500       // ms, segurança caso a linha não seja achada

volatile bool emCurva = false;


void setup() {
  Serial.begin(9600);

  //Configuração dos pinos
  pinMode(LED_DEBUG, OUTPUT);
  pinMode(IR_CONTADOR_LINHA, INPUT);
  pinMode(IR_OBSTACULO, INPUT);
  pinMode(IR_SEGUIDOR_DIREITO, INPUT);
  pinMode(IR_SEGUIDOR_ESQUERDO, INPUT);
  pinMode(PWM_MOTOR_DIREITO, OUTPUT);
  pinMode(PWM_MOTOR_ESQUERDO, OUTPUT);
  pinMode(MOTOR_DIREITO_IN_0, OUTPUT);
  pinMode(MOTOR_DIREITO_IN_1, OUTPUT);
  pinMode(MOTOR_ESQUERDO_IN_0, OUTPUT);
  pinMode(MOTOR_ESQUERDO_IN_1, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(IR_OBSTACULO), obstaculo, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IR_CONTADOR_LINHA), contador_linha, RISING);

  Serial.println("Sistema iniciado!");
}

void loop() {
static int countAnterior = -1;

if (ultimoCount != countAnterior) {
  countAnterior = ultimoCount;

  Serial.print("Contagem de linhas: ");
  Serial.println(countAnterior);
}

  switch(estado) {
    case SEGUIR_LINHA:
      seguidor_de_linha();
      break;
    case CURVA:
      realizar_a_curva();
      break;
    case PARADA:
      parar();
      break;
  }
}

void seguidor_de_linha() {
  int sensorDir = digitalRead(IR_SEGUIDOR_DIREITO);
  int sensorEsq = digitalRead(IR_SEGUIDOR_ESQUERDO);

  // Linha centralizada - segue em frente
  if (sensorEsq == 0 && sensorDir == 0) {
    motorEsquerdo(VELOCIDADE_BASE);
    motorDireito(VELOCIDADE_BASE);
  }
  
  // Linha deslocada para a esquerda - roda esquerda desacelera
  else if (sensorEsq == 1 && sensorDir == 0) {
    motorEsquerdo(VELOCIDADE_CORRECAO);
    motorDireito(VELOCIDADE_BASE);
  }

  // Linha deslocada para a direita - roda direita desacelera
  else if (sensorEsq == 0 && sensorDir == 1) {
    motorEsquerdo(VELOCIDADE_BASE);
    motorDireito(VELOCIDADE_CORRECAO);
  }
}
  
void realizar_a_curva() {
  emCurva = true;

  //
  parar();
  delay(500);
  motorDireito(-VELOCIDADE_BASE);
  motorEsquerdo(-VELOCIDADE_BASE);
  delay(400);
  //


  unsigned long inicio = millis();

  // Gira para a esquerda no próprio eixo
  motorDireito(VELOCIDADE_GIRO);
  motorEsquerdo(-VELOCIDADE_GIRO);

  while (true) {

    unsigned long decorrido = millis() - inicio;

    // Só aceita a linha nova depois de ter saído da antiga
    if (decorrido > TEMPO_MIN_GIRO) {
      if (digitalRead(IR_SEGUIDOR_DIREITO) == 1) {
        break;  // achou a linha nova
      }
    }
  }

  iCount = 0;
  emCurva = false;
  estado = SEGUIR_LINHA;  // o seguidor de linha assume e centraliza
}

  
void parar() {
  analogWrite(PWM_MOTOR_DIREITO, 0);
  analogWrite(PWM_MOTOR_ESQUERDO, 0);
  
  digitalWrite(MOTOR_DIREITO_IN_0, LOW);
  digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}

  
void contador_linha() {
  if (emCurva) return;   // <-- novo

  static unsigned long tempoAnterior = 0;
  if (millis() - tempoAnterior >= 300) {
    tempoAnterior = millis();
    iCount++;
    ultimoCount = iCount;

    if (iCount == 3) {
      estado = CURVA;
      iCount = 0;
    }
  }
}

  
void obstaculo() {
  isObstaculo =! isObstaculo;                                                                       
  
  if(isObstaculo)
    estado = PARADA;
  else
    estado = SEGUIR_LINHA;
}

//===Funções dos motores===

void motorDireito(int velocidade) {
	if(velocidade > 0) { // "sinal" da velocidade define sentido de rotação
      digitalWrite(MOTOR_DIREITO_IN_0, HIGH);
      digitalWrite(MOTOR_DIREITO_IN_1, LOW);
    } else {
      digitalWrite(MOTOR_DIREITO_IN_0, LOW);
      digitalWrite(MOTOR_DIREITO_IN_1, HIGH);
    }
    
    analogWrite(PWM_MOTOR_DIREITO, abs(velocidade)); // 'abs' retorna o valor absoluto da velocidade informada
  }
  
void motorEsquerdo(int velocidade) {
    if(velocidade > 0) {
      digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH);
      digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
    } else {
      digitalWrite(MOTOR_ESQUERDO_IN_0, LOW);
      digitalWrite(MOTOR_ESQUERDO_IN_1, HIGH);
    }
  
    analogWrite(PWM_MOTOR_ESQUERDO, abs(velocidade));
  }
