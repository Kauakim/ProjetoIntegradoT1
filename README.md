# ProjetoIntegradoT1

Desenvolvido por: Victor Alberti, Kauã Ribeiro, Júlio Magalhães e Samuel Farias

## Descrição do Projeto
Este projeto foi desenvolvido como parte de uma avaliação escolar no curso de telecomunicações em 2024. Ele utiliza um ESP32 para gerenciar a comutação de rotas através do CI MT8816, controlando a transmissão de dados via broadcast entre diferentes terminais. O projeto também coleta dados meteorológicos de uma API externa e realiza a escolha automática da rota de comunicação com base nos custos e condições climáticas.

## Funcionalidades
1. Roteamento Dinâmico: Seleciona rotas de comunicação internas ou externas automaticamente, baseado nos parâmetros de entrada.
2. Broadcast Interno: Executa a comunicação interna entre terminais definidos no CI MT8816.
3. Seleção de Rota Externa: Com base nos dados de chuva e custo de fila, o sistema escolhe se a transmissão será feita por fibra óptica ou rádio.
4. Integração com API de Clima: Coleta informações sobre o clima atual (chuva) usando a API Open Meteo e adapta as rotas de acordo.
5. Interface Web: Disponibiliza uma página web para que o usuário insira coordenadas e selecione a rota de comunicação.

## Componentes Utilizados
1. ESP32
2. MT8816: Chip de comutação analógica que controla os terminais de comunicação.
3. WiFi: O ESP32 conecta-se à rede WiFi para obter informações meteorológicas e disponibilizar a interface web.
4. WebServer: O servidor web embutido no ESP32 permite a interação com o projeto via navegador.
5. API Open Meteo: Coleta dados meteorológicos, como o nível de chuva atual, baseado em coordenadas geográficas.

## Estrutura do Código
1. Bibliotecas Utilizadas:
   
    - ArduinoJson: Para lidar com os dados JSON retornados pelas APIs.
    - HTTPClient: Para realizar requisições HTTP à API externa.
    - MT8816_ESP: Gerencia o CI MT8816, responsável pela comutação de rotas, foi uma biblioteca desenvolvida pela equipe.
    - WebServer: Servidor HTTP que serve a página de configuração e lida com as requisições do usuário.
    - WiFi: Conecta o ESP32 à rede WiFi.

2. Conexão ao WiFi
   
    A função IniciarWiFi() conecta o ESP32 à rede WiFi definida no código. O monitor serial exibe o endereço IP do ESP32 após a conexão bem-sucedida.
   
3. Interface Web
   
    A interface web é acessível via um navegador de internet e apresenta:
  
    - Um formulário para entrada de coordenadas (latitude e longitude).
    - Opções de roteamento, permitindo que o usuário escolha entre Roteamento Interno e Roteamento Externo.
  
4. Roteamento Interno
   
    Utiliza os pinos predefinidos no CI MT8816 para realizar um broadcast interno entre terminais específicos (X14, X15, X8 e Y2).

5. Roteamento Externo
   
    O roteamento externo usa os dados de chuva e o custo OSPF das rotas para selecionar se a transmissão será feita via Fibra Óptica ou Rádio.

6. Processamento das Coordenadas
    
    Os valores de latitude e longitude são processados pelo ESP32. Se estiverem fora do intervalo esperado (-90 a 90), uma mensagem de erro será exibida.

7. Requisições à API de Clima
    
    A função getRainData() faz uma requisição à API Open Meteo e retorna o valor de chuva atual na localização especificada pelo usuário.

8. Cálculo de Custo de Fila
    
    A função getTemposDeFila() obtém os tempos de fila das rotas de comunicação de um servidor local, calcula os custos das rotas, e determina o melhor caminho com base no menor custo.
