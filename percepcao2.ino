#include <DHT.h>
#include <ESP32Servo.h>

// --- Definições de Pinos ---
#define POT_1_PIN 32      // Potenciômetro 1 (Servo 1)
#define POT_2_PIN 35      // Potenciômetro 2 (Servo 2)
#define BUTTON_PIN 14     // Botão para ativar/desativar sistema
#define DHTPIN 4          // Sensor DHT22
#define DHTTYPE DHT11
#define INC_PIN 34

#define BUZZER_PIN 22     // Buzzer
#define LED_R_PIN 26      // LED Vermelho
#define LED_G_PIN 27      // LED Verde

#define SERVO_1_PIN 33    // Servo 1
#define SERVO_2_PIN 13    // Servo 2

#define LDR_PIN POT_1_PIN // Reutilizando o pot_1 como LDR (corrigir se tiver LDR real)
#define LDR_THRESHOLD 1000  // Limiar para ambiente claro (ajustável)

// --- Objetos Globais ---
DHT dht(DHTPIN, DHTTYPE);
Servo servo1;
Servo servo2;

// --- Variáveis Globais ---
volatile bool production_enable = true;
volatile unsigned long last_interrupt_time = 0;
volatile bool button_state = false;

float temperature = 0.0;
int pot_1_value = 0;
int pot_2_value = 0;

bool temp_warning = false;

bool inc_sensor = false;

// --- Interrupção do Botão ---
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    button_state = !button_state;
    production_enable = !production_enable;
    last_interrupt_time = interrupt_time;
  }
}

// --- Função para escrever cor no LED RGB (apenas R e G usados) ---
void writeLED(bool r, bool g) {
  digitalWrite(LED_R_PIN, r);
  digitalWrite(LED_G_PIN, g);
}

// --- Leitura dos Sensores ---
void readSensors() {
  temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("Erro ao ler a temperatura!");
    temperature = 25.0; // Valor padrão para evitar falhas
  }

  temp_warning = (temperature < 0 || temperature > 30);

  pot_1_value = analogRead(POT_1_PIN);
  pot_2_value = analogRead(POT_2_PIN);
}



// --- Controle do Buzzer ---
void updateBuzzer() {
  if (production_enable && temp_warning) {
    tone(BUZZER_PIN, 2000);
  } else {
    noTone(BUZZER_PIN);
  }
}

// --- Controle dos Servos com base nos Potenciômetros ---
void updateServos() {
  if (!production_enable || temp_warning) {
    return; // Não move os servos se a produção estiver desligada
  }

  int angle1 = map(pot_1_value, 0, 4095, 0, 180);
  int angle2 = map(pot_2_value, 0, 4095, 0, 180);

  servo1.write(angle1);
  servo2.write(angle2);
}

// --- Controle dos LEDs de Status ---
void updateStatusLeds() {

  inc_sensor =  digitalRead(INC_PIN);

  if (inc_sensor == true && production_enable == true){
    production_enable = false;
  }else if (inc_sensor == false && production_enable != false  && !temp_warning){
    production_enable = true;
  }

  if (!production_enable || temp_warning) {
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_R_PIN, HIGH);
  } else {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, HIGH);
  }
}


unsigned long tempoAnterior = 0;
const unsigned long intervalo = 2000; // 2 segundos (2000 ms)

bool delay2s() {
unsigned long tempoAtual = millis();
if (tempoAtual - tempoAnterior >= intervalo) {
tempoAnterior = tempoAtual;
return true; // Já passou 2 segundos
}
return false; // Ainda não passou
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Pinos de entrada
  pinMode(POT_1_PIN, INPUT);
  pinMode(POT_2_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(INC_PIN, INPUT);

  // Pinos de saída
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);

  // Inicializa LEDs e Buzzer desligados
  digitalWrite(BUZZER_PIN, LOW);

  // Anexa os servos
  servo1.attach(SERVO_1_PIN);
  servo2.attach(SERVO_2_PIN);

  // Interrupção do botão
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
}

// --- Loop principal ---
void loop() {
  
  readSensors();
  updateBuzzer();
  updateServos();
  updateStatusLeds();


  if(delay2s()){
    Serial.printf("INC: %d\n |Temp: %.2f ºC | Pot1: %d | Pot2: %d | System: %s\n",
                  inc_sensor, temperature, pot_1_value, pot_2_value,
                  production_enable ? "ON" : "OFF");
  }


  delay(10);
}

