# Projeto: Matriz de LEDs WS2812 com Botões e Debouncing

Este projeto utiliza a placa BitDogLab (Raspberry Pi Pico W) para controlar uma matriz de 25 LEDs WS2812 (configurada como 5x5) e um LED RGB (usando somente o canal vermelho para piscar).  
A exibição na matriz mostra dígitos de 0 a 9, com padrões invertidos verticalmente para melhor visualização de acordo com a orientação da placa.

## Funcionalidades

- **Exibição de dígitos (0 a 9) na matriz 5x5:**  
  Cada dígito é desenhado a partir de um padrão pré-definido armazenado em um array de 25 valores booleanos.  
  Os padrões são invertidos verticalmente na exibição, para que a imagem fique na orientação correta.

- **Controle via Botões:**  
  - **Botão A (GPIO 5):** Incrementa o dígito exibido (cíclico de 0 a 9).
  - **Botão B (GPIO 6):** Decrementa o dígito exibido (cíclico de 0 a 9).  
  O controle utiliza debouncing por software e interrupções para detectar as transições dos botões.

- **LED RGB (canal vermelho no GPIO 13):**  
  Pisca a 5 Hz (via timer repetitivo) para fins de depuração.

- **Controle dos LEDs WS2812 via PIO:**  
  O PIO é configurado para enviar os dados necessários para acionar os LEDs WS2812, utilizando o arquivo `ws2812.pio`.

- **Brilho Ajustado:**  
  A intensidade dos LEDs é ajustada com um fator de brilho (10% da intensidade máxima).

## Arquivos do Projeto

- **tarefa_interrupcoes.c:**  
  Código-fonte principal, que inclui a lógica de controle dos botões, debouncing, atualização da matriz de LEDs e controle do PIO.

- **ws2812.pio:**  
  Arquivo de programa PIO para controlar os LEDs WS2812.  
  Este arquivo é processado pelo CMake para gerar o header `ws2812.pio.h`.

- **CMakeLists.txt:**  
  Arquivo de configuração do CMake para compilar o projeto com o Pico SDK.

## Requisitos

- Raspberry Pi Pico ou Pico W com a placa de desenvolvimento BitDogLab.
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) devidamente instalado.
- Ambiente de desenvolvimento com suporte ao CMake (por exemplo, VS Code com a extensão do Pico).

## Instruções de Compilação e Execução

1. **Clone o repositório (ou copie os arquivos):**

   ```bash
   git clone https://seu-repositorio-url.git
   cd seu-repositorio

2. **Faça a importação do projeto pela extensaõ pico project no vscode:**

3. **Compile o projeto e, com a placa em modo bootsel, copie o arquivo .uf2 para o MCU.**

4. **Teste o funcionamento:**
 - Pressione os botões A e B para alternar os dígitos exibidos na matriz.
 - O LED RGN (canal vermelho, GPIO 13) deverá piscar, confirmando que o timer está ativo.

## Demonstração no YouTube

Confira a demonstração completa deste projeto no YouTube: [Demonstração do Projeto]( https://youtube.com/shorts/B9CQdJD1834?si=GK2yo7ibDDhgPFT2 )

## Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
