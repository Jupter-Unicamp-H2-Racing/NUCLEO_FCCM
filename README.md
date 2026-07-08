| Categoria       | Sinal/Função | GPIO | Conector |
| --------------- | ------------ | ---- | -------- |
| CAN             | FDCAN1_RX    | PB5  | CN10-29  |
| CAN             | FDCAN1_TX    | PB3  | CN10-31  |
| UART            | USART1_TX    | PB14 | CN10-3   |
| UART            | USART1_RX    | PB15 | CN10-19  |
| Alimentação     | 3.3V         | —    | CN7-16   |
| Alimentação     | 3.3V         | —    | CN7-23   |
| Alimentação     | 3.3V         | —    | CN10-17  |
| Alimentação     | 5V           | —    | CN7-18   |
| Terra           | GND          | —    | CN7-8    |
| Terra           | GND          | —    | CN7-19   |
| Terra           | GND          | —    | CN7-20   |
| Terra           | GND          | —    | CN7-22   |
| Terra           | GND          | —    | CN10-9   |
| Terra           | GND          | —    | CN10-20  |
| Terra Analógico | AGND         | —    | CN10-32  |
| Saída           | ECU          | PC0  | CN7-38   |
| Saída           | Warm Up      | PC4  | CN10-21  |
| Entrada         | TEMPERATURE  | PC3  | CN7-37   |
| Entrada         | VOLTAGE      | PA0  | CN7-28   |
| Entrada         | CURRENT      | PA1  | CN7-30   |
| Entrada         | PRESSURE     | PB0  | CN7-34   |
| Saída           | MAIN         | PB12 | CN10-16  |
| Saída           | PURGE        | PB13 | CN10-30  |
| PWM             | PWM_FAN      | PA8  | CN10-23  |
| Saída           | FORCE_SUPPLY | PC2  | CN7-35   |
| Saída           | FORCE_PURGE  | PC1  | CN7-36   |
| Saída           | SUPPLY       | PB10 | CN10-25  |


| Timer  | Tipo                      | Uso típico                                   |
| ------ | ------------------------- | -------------------------------------------- |
| TIM1   | Advanced Control          | PWM avançado, dead-time, controle de motores |
| TIM2   | General Purpose (32 bits) | Base de tempo, contadores longos             |
| TIM3   | General Purpose           | PWM, interrupções periódicas                 |
| TIM4   | General Purpose           | PWM, interrupções periódicas                 |
| TIM5   | General Purpose (32 bits) | Base de tempo longa                          |
| TIM6   | Basic Timer               | Apenas geração de interrupção/DAC            |
| TIM7   | Basic Timer               | Apenas geração de interrupção/DAC            |
| TIM15  | General Purpose           | PWM e interrupções                           |
| TIM16  | General Purpose           | PWM e interrupções                           |
| TIM17  | General Purpose           | PWM e interrupções                           |
| LPTIM1 | Low Power Timer           | Temporizações em baixo consumo               |
| LPTIM2 | Low Power Timer           | Temporizações em baixo consumo               |
