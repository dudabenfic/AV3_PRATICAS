#include <DHT.h>
// #include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES  2

// --- Mapeamento de Pinos (Conforme solicitado) ---
// Sensores e Entradas
#define POT_PIN 32      // Pino para o Potenciômetro
#define LDR_PIN 35      // Pino para o LDR (Fotoresistor)
#define BUTTON_PIN 13   // Pino para o Botão de interrupção
#define DHTPIN 4        // Pino de dados do sensor DHT22

// Atuadores (Saídas)
#define BUZZER_PIN 23   // Pino para o Buzzer
#define LED_R_PIN 27    // Pino para o componente Vermelho (R) do LED RGB
#define LED_G_PIN 26    // Pino para o componente Verde (G) do LED RGB
#define LED_B_PIN 25    // Pino para o componente Azul (B) do LED RGB

// --- Configurações e Constantes ---
#define DHTTYPE DHT11   // Define o tipo do sensor DHT
#define LUX_THRESHOLD 1500 // Valor nominal de luminosidade. Abaixo disso, é considerado "escuro".
                           // Este valor depende do seu circuito e deve ser ajustado. (0-4095 para ESP32)

// --- Instância dos Objetos ---
DHT dht(DHTPIN, DHTTYPE);
// LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


const float GAMMA = 0.7;      // Coeficiente do LDR (ajuste conforme o sensor)
const float RL10 = 50;        // Resistência do LDR em 10 lux (em kOhms)


// --- Variáveis Globais ---
// Flag para controlar o estado geral do sistema (ligado/desligado pelo botão)
volatile bool rgb_enabled = true; // 'volatile' é importante para variáveis usadas em interrupções

// Variável para debouncing do botão (evitar múltiplos acionamentos)
volatile unsigned long last_interrupt_time = 0;

// Variáveis para armazenar os valores dos sensores
float temperature = 0.0;
int lux_value = 0;
int pot_value = 0;

bool temp_warning = false;

// --- Função da Interrupção (ISR - Interrupt Service Routine) ---
// Esta função é chamada AUTOMATICAMENTE quando o botão é pressionado
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long interrupt_time = millis();
   
  // Lógica de debounce: só processa a interrupção se já passou 200ms desde a última
  if (interrupt_time - last_interrupt_time > 200) {
    rgb_enabled = !rgb_enabled; // Inverte o estado do sistema (liga -> desliga, desliga -> liga)
    last_interrupt_time = interrupt_time;
  }
}

// --- Funções Auxiliares ---

// Função para escrever os valores nos pinos do LED RGB usando analogWrite
void writeRgb(int r, int g, int b) {
  Serial.printf("%d %d %d \n",r ,g ,b);
  analogWrite(LED_R_PIN, r);
  analogWrite(LED_G_PIN, g);
  analogWrite(LED_B_PIN, b);
}

// Função para ler todos os sensores e atualizar as variáveis globais
void readSensors() {
  temperature = dht.readTemperature();

  if((temperature < 0 || temperature > 30)){
    temp_warning = true;
  }else{
    temp_warning  = 0;
  }
    int analogValue = analogRead(LDR_PIN);
    lux_value = analogValue;
    // float volts = analogValue / 4095.0 * 3.3;
    // float resistance = 2000 * volts / (3.3 - volts);
    // float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1.0 / GAMMA));
    // lux_value = lux / 10; 
 
  pot_value = analogRead(POT_PIN);
}


void setLedColorFromPot(int value) {
  int r=0, g=0, b=0;

  if (value < 820) { // Vermelho -> Laranja
    r = 4095;
    g = map(value, 0, 819, 0, 2650); // 2650 ≈ 165/255 * 4095
    b = 0;
  } else if (value < 1640) { // Laranja -> Amarelo
    r = 4095;
    g = map(value, 820, 1639, 2650, 4095);
    b = 0;
  } else if (value < 2460) { // Amarelo -> Branco
    r = 4095;
    g = 4095;
    b = map(value, 1640, 2459, 0, 4095);
  } else if (value < 3280) { // Branco -> Azul Claro
    r = map(value, 2460, 3279, 4095, 0);
    g = map(value, 2460, 3279, 4095, 3200); // 3200 ≈ 200/255 * 4095
    b = 4095;
  } else { // Azul Claro -> Azul
    r = 0;
    g = map(value, 3280, 4095, 3200, 0);
    b = 4095;
  }

  writeRgb(r, g, b); // aqui você manda para os canais PWM configurados em 12 bits
}



// Função para controlar a lógica de ligar/desligar o LED RGB
void updateRgbLed() {
  // Condições para o LED estar DESLIGADO:
  // 1. O sistema foi desabilitado pelo botão.
  // 2. A temperatura está fora da faixa segura (0 a 30 ºC).
  // 3. O ambiente está claro (valor do LDR acima do limiar).
  if (!rgb_enabled || temp_warning || lux_value > LUX_THRESHOLD) {
    writeRgb(0, 0, 0); // Desliga o LED
  } else {
    // Se todas as condições são favoráveis, liga o LED com a cor definida pelo potenciômetro.
    setLedColorFromPot(pot_value);
  }
}

// Função para controlar a lógica do Buzzer
void updateBuzzer() {
  // Condições para o Buzzer LIGAR:
  // 1. O sistema está habilitado.
  // 2. A temperatura é inferior a 0ºC OU superior a 30ºC.
  if (rgb_enabled && temp_warning) {
    tone(BUZZER_PIN, 4000); // Liga o buzzer
  } else {
    noTone(BUZZER_PIN);
  }
}

// --- Funções Principais ---
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configura os pinos de entrada
  pinMode(LDR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  // Configura o botão com pull-up interno. O botão deve conectar o pino ao GND.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configura os pinos de saída para o LED RGB e o Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  // Garante que os atuadores comecem desligados
  digitalWrite(BUZZER_PIN, LOW); 
  writeRgb(0, 0, 0);

  // Configura a interrupção no pino do botão
  // `digitalPinToInterrupt` converte o número do pino para o número da interrupção
  // `handleButtonInterrupt` é a função que será chamada
  // `FALLING` aciona a interrupção quando o sinal vai de HIGH para LOW (ao pressionar o botão com PULLUP)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
  // lcd.init();
  // lcd.backlight();

}

void loop() {
  if(rgb_enabled){
  readSensors();

  // 2. Atualiza o estado do Buzzer com base na temperatura e no botão
  updateBuzzer();

  // 3. Atualiza o estado e a cor do LED com base em todos os sensores e no botão
  updateRgbLed();
    Serial.println("Temp: "+ String(temperature));
    Serial.println("lux: "+ String(lux_value));
    Serial.println("ON");

     Serial.printf("Temp: %.2f C, LDR: %d, Pot: %d, System Enabled: %s\n",
               temperature, lux_value, pot_value, rgb_enabled ? "true" : "false");

  // lcd.setCursor(0, 0);
  // lcd.print("Temp: " +  String(temperature) );
  // lcd.setCursor(0, 1);
  // lcd.print("Lux: " + String(lux_value) );
  
  }
  else{
    updateBuzzer();
    updateRgbLed();
    // lcd.clear();
    // lcd.setCursor(0, 0);
    Serial.println("OFF");
    // lcd.print("OFF");
    
    
  }
            

  // Pequeno delay para estabilidade do loop
  delay(100);
}

