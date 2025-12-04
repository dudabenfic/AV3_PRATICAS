# AV3_PRATICAS

1. potenciometro para controlar a posicao do motor
2. sistema com dht e ldr que le valores, e ativa led vermelho caso leia valores criticos, porem sistema pode ser desativado e reativado usando um botao
3. potenciometro para controlar cor de led rgb
4. sistema com dois botoes, um para alternar entre 2 leds e outro para desligar e ligar o sistema usando interrupcao

- Percepcao1: Esse código lê sensores (temperatura, luminosidade e potenciômetro) para controlar automaticamente um LED RGB e um buzzer. Um botão com interrupção permite ligar/desligar o sistema, e os dados são enviados ao monitor serial.
- Percepcao2: Esse código controla dois servos, um buzzer e LEDs com base em leituras de temperatura e potenciômetros, e um sensor adicional. Um botão com interrupção permite habilitar ou desabilitar o sistema, e os dados são enviados periodicamente ao monitor serial.
- Redes1: Esse código conecta o ESP32 a uma rede Wi-Fi e exibe informações como SSID, IP e MAC no monitor serial. Os LEDs indicam visualmente o status da conexão enquanto o LED interno pisca continuamente no loop.
- Esse código cria um sistema de segurança IoT com Wi-Fi e MQTT, permitindo monitorar presença, controlar uma fechadura com servo motor e acionar alarme. Ele publica estados em tópicos MQTT, recebe comandos remotamente e indica eventos com buzzer e LEDs.
- Redes4: Esse código conecta o ESP32 ao Wi-Fi e MQTT para enviar dados de sensores (temperatura, umidade, luz e presença) e receber comandos para controlar dois servos. Ele também envia alertas via WhatsApp quando detecta alta probabilidade de presença e indica o estado com LEDs.


