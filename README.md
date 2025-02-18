# Tarefa ADC - Embarcatech

Neste projeto, utilizo um display SSD1306 e um joystick para controlar a interação visual e luminosa em um sistema baseado no Raspberry Pi Pico. O joystick é responsável por movimentar um quadrado na tela do display, enquanto seus eixos X e Y controlam a intensidade de LEDs vermelho e azul, respectivamente. Ao mover o joystick para cima, a intensidade do LED azul aumenta, e ao movê-lo para baixo, diminui. Da mesma forma, movendo para a direita, o LED vermelho fica mais forte, e para a esquerda, mais fraco. Quando o joystick está centralizado, ambos os LEDs permanecem desligados.

O botão do joystick alterna o estado de um LED verde e muda a borda do display entre ativada e desativada. Além disso, o botão A desativa os LEDs vermelho e azul via PWM, enquanto o botão B coloca a placa Raspberry Pi Pico W no modo BOOTSEL, permitindo a reprogramação do dispositivo.

## Requisitos

- Placa Bitdoglab
- Compilador C
- VS Code instalado
- Pico SDK configurado
- Simulador Wokwi integrado ao VS Code
- Git instalado
- Extensão C/C++ instalada no VS Code
- Extensão Raspberry Pi Tools instalada no VS Code

## Componentes

- 1 LED RGB
- 1 Display 128x64 OLED SSD1306
- 2 Botões
- 1 Joystick

## Como executar

1. Clone o repositório:

```bash
git clone [https://github.com/matheusssilva991/tarefa_adc_U4C8.git]
```

2. Configure o ambiente de desenvolvimento seguindo as instruções do Pico SDK

3. Abra o projeto no VS Code

4. Importe o projeto através da extensão Raspberry Pi Tools

5. Execute a simulação através do Wokwi ou da placa Bitdoglab

## Demonstração

A seguir, um vídeo demonstrando o funcionamento do projeto:

[![Vídeo de demonstração](https://drive.google.com/file/d/1CHxF7dhVEcHRjCrRR7X02vJ_K2S8gDTZ/view?usp=sharing)](https://drive.google.com/file/d/1CHxF7dhVEcHRjCrRR7X02vJ_K2S8gDTZ/view?usp=sharing)

## 🤝 Equipe

Membros da equipe de desenvolvimento do projeto:
<table>
  <tr>
    <td align="center">
      <a href="https://github.com/matheusssilva991">
        <img src="https://github.com/matheusssilva991.png" width="100px;" alt="Foto de Matheus Santos Silva no GitHub"/><br>
        <b>Matheus Santos Silva (matheusssilva991)</b>
        <p>Desenvolvedor Back-end - NestJS</p>
      </a>
    </td>
  <tr>
</table>
