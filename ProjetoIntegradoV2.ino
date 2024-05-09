// Inclue as bibliotecas
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <MT8816_ESP.h>
#include <WebServer.h>
#include <WiFi.h>

//-----------------------------------------------------------------------------------------

//Define os pinos do CI MT8816 e os arrays que serão utilizados para os endereços de broadcast
//Pinos de broadcast (Rede local) X14 X15 X6 e Y1
//Pinos de Fibra X8 e X9
//Pinos de Radio X10 e X11

//Para acionar os pinos X do CI utilize a seguinte lógica, que não faz o menor sentido :)
//1=X1, 2=X2, 3=X3, 4=X4, 5=X5, 6=X12, 7=X13, 8=X6, 9=X7, 10=X8, 11=X9, 12=X10, 13=X11, 14=X14, 15=X15
//Para os pinos de Y do CI a lógica realmente existe :)

MT8816 MT8816(22, 25, 13, 14, 26, 27, 18, 21, 19, 23); // reset, strobe, data, ay0, ay1, ay2, ax0, ax1, ax2, ax3

//Define a rota de broadcast do exemplo da demonstração
int x_el[3] = {14, 15, 8}, y_el[1] = {2};
Array x_arr(x_el, sizeof(x_el) / sizeof(x_el[0])), y_arr(y_el, sizeof(y_el) / sizeof(y_el[0]));

//Variável de controle que impede que o CI fique resetando infinitamente sem necessidade
int interface = -1;

//-----------------------------------------------------------------------------------------

//Define a senha e a rede WiFI
const char *ssid = "TELECO_2k24";
const char *password = "analelealan";

//Variáveis para coletar a informação do clima
String Latitude = "", Longitude = "";
String Rota = "";
float LatitudeFloat = 0, LongitudeFloat = 0, Chuva = 0, ResultadoCusto[6] = {0};
int counter = 0;
//-----------------------------------------------------------------------------------------

//Cria um servidor na porta 80
WebServer server(80);

//-----------------------------------------------------------------------------------------

//Função que conectar o ESP32 no WiFi
void IniciarWiFi() {

  WiFi.begin(ssid, password);

  //Espera até que o ESP32 se conecte ao WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi!");

  //Exibe o endereço de IP do ESP32 no monitor serial
  Serial.print("Endereço IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

//-----------------------------------------------------------------------------------------

//Envia a string para o servidor com o conteudo HTML, com o código HTTP 200 (Ok)
void handleRoot() {

  //String que contem o HTML da landing page do servidor
  String htmlContent =
    "<!DOCTYPE html>"
    "<html lang=\"pt-br\" style=\"padding: 0; margin: 0; box-sizing: "
    "border-box; font-family: 'Lucida Sans', sans-serif;\">"
    "<head>"
    "<meta charset=\"UTF-8\" />"
    "<meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\" />"
    "<title>Configuração Projeto Integrado</title>"
    "</head>"
    "<body style=\"max-width: 100vw; min-height: 100vh; padding: 0; margin: "
    "0; box-sizing: border-box; background-color: rgb(178, 216, 154);\">"
    "<header style=\"width: 100vw; height: 100px; background: "
    "linear-gradient(135deg, rgba(50, 70, 155, 0.826), rgba(72, 20, 145, "
    "0.801));\">"
    "<div class=\"glass\" style=\"width: 100vw; height: 96px; padding: 0; "
    "margin: 0; box-sizing: border-box; background: linear-gradient(135deg, "
    "rgb(255, 255, 255, 0.1), rgba(255, 255, 255, 0)); backdrop-filter: "
    "blur(10px); -webkit-backdrop-filter: blur(10px); box-shadow: 0 10px "
    "20px 0 rgba(0, 0, 0, 0.591); text-align: center; padding-top: 35px; "
    "font-size: 1.8rem; letter-spacing: 3px; font-weight: 700; color: "
    "#ffea00;\">TELECOM</div>"
    "</header>"

    "<p style=\"margin-top: 50px; margin-left: 10px; font-size: 1.1em; "
    "font-weight: 600; color: rgba(0, 0, 0, 0.942);\">"
    "Seja bem-vindo ao site de configuração do primeiro projeto integrado "
    "de telecomunicações de 2024. Para que o projeto funcione corretamente, "
    "digite as coordenadas abaixo, sendo ambos os valores de latitude e "
    "longitude compreendidos entre -90 e 90. Escolha também a rota pela "
    "qual a informação deve transitar, tendo em vista que o modo interno de "
    "comunicação realiza broadcast entre 4 terminais do CI, e que o modo "
    "externo de comunicação escolhe um dentre 4 outros terminais do CI para transitar a "
    "mensagem, como o clima, o tipo de comunicação e o custo OSPF de cada rota "
    "</p>"
    "<form method='GET' action='/process_form' style=\"width: 75%; "
    "min-height: 440px; margin: auto; margin-top: 50px; display: flex; "
    "flex-direction: column; justify-content: start; align-items: center; "
    "border-radius: 7px;\">"
    "<div class=\"bloco-valores\" style=\"background: linear-gradient(90deg, "
    "rgba(43, 62, 36, 0.642) 0%, rgba(7, 51, 60, 0.626) 100%); width: 100%; "
    "height: 240px; display: flex; flex-direction: column; justify-content: "
    "start; align-items: center; border-radius: 7px;\">"
    "<h2 style=\"font-size: 1.4rem; font-weight: 700; color: rgb(255, 255, "
    "255); letter-spacing: 1px; margin-right: 3.5%;\">Insira a posição</h2>"
    "<input type='text' name='latitude' placeholder='Latitude' "
    "required style=\"background-color: #ffffff; width: "
    "80%; height: 40px; margin-top: 20px; font-size: 1.1em; letter-spacing: "
    "1px; font-weight: 500; padding-left: 10px; font-family: 'Trebuchet MS', "
    "sans-serif;\" />"
    "<input type='text' name='longitude' placeholder='Longitude' "
    "required style=\"background-color: #ffffff; width: "
    "80%; height: 40px; margin-top: 20px; font-size: 1.1em; letter-spacing: "
    "1px; font-weight: 500; padding-left: 10px; font-family: 'Trebuchet MS', "
    "sans-serif;\" />"
    "</div>"
    "<div class=\"bloco-route\" style=\"width: 100%; height: 250px; display: "
    "flex; flex-direction: column; justify-content: start; align-items: "
    "center; border-radius: 7px;\">"
    "<h1 style=\"text-align: center; margin-top: 40px; font-family: Verdana, "
    "sans-serif;\">Escolha o tipo de roteamento</h1>"
    "<button type='submit' name='route' value='Interno' style=\"width: 90%; "
    "height: 60px; border-radius: 15px; border: 2px solid black; background: "
    "linear-gradient(90deg, rgba(2, 0, 36, 1) 0%, rgba(9, 9, 121, 1) 35%, "
    "rgb(0, 132, 255) 100%); margin: 4px; margin-left: 0; font-size: 1.3em; "
    "letter-spacing: 2px; font-weight: 700; color: #ffffffd2;\" "
    "onmouseover=\"this.style.background = '#ff5733'; this.style.color = "
    "'white'; this.style.cursor = 'pointer';\" "
    "onmouseout=\"this.style.background = '#006cc4f5'; this.style.color = "
    "'#ffffffd2';\">Roteamento Interno</button>"
    "<button type='submit' name='route' value='Externo' style=\"width: 90%; "
    "height: 60px; border-radius: 15px; border: 2px solid black; background: "
    "linear-gradient(90deg, rgba(2, 0, 36, 1) 0%, rgba(9, 9, 121, 1) 35%, "
    "rgb(0, 132, 255) 100%); margin: 4px; margin-left: 0; font-size: 1.3em; "
    "letter-spacing: 2px; font-weight: 700; color: #ffffffd2;\" "
    "onmouseover=\"this.style.background = '#ff5733'; this.style.color = "
    "'white'; this.style.cursor = 'pointer';\" "
    "onmouseout=\"this.style.background = '#006cc4f5'; this.style.color = "
    "'#ffffffd2';\">Roteamento Externo</button>"
    "</div>"
    "</form>"

    "<footer style=\"width: 100vw; height: 100px; background-color: rgba(0, "
    "0, 0, 0.965); margin-top: 20px; display: flex; align-items: start; "
    "justify-content: center;\">"
    "<p class=\"text-footer\" style=\"color: #ffffffb1; margin-right: 10px; "
    "margin-top: 44px;\">© 2024 Projeto Integrado e Telecom</p>"
    "</footer>"
    "</body>"
    "</html>";

  //Envia o html para o servidor
  server.send(200, "text/html", htmlContent);
}

//Envia um texto para o servidor, com o código HTTP 404 (ERRO)
void handle_not_found() {
  server.send(404, "text/plain", "Página não encontrada");
}

//-----------------------------------------------------------------------------------------

//Função do site para a rota "/process"
void handleProcessForm() {

  //Envia uma mensagem para o usuário, em formato de site, de que as informações foram incorretamente coletadas e com o código 200
  String htmlError =
    "<!DOCTYPE html>"
    "<html lang=\"pt-br\" style=\"padding: 0; margin: 0; box-sizing: "
    "border-box; font-family: 'Lucida Sans', sans-serif;\">"
    "<head>"
    "<meta charset=\"UTF-8\" />"
    "<meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\" />"
    "<title>Error</title>"
    "</head>"
    "<body style=\""
    "max-width: 100vw;"
    "min-height: 100vh;"
    "display: flex;"
    "flex-direction: column;"
    "justify-content: space-between;"
    "align-items: center;"
    "padding: 0;"
    "margin: 0;"
    "box-sizing: border-box;"
    "background-color: rgb(178, 216, 154);"
    "\">"
    "<header style=\""
    "width: 100vw;"
    "height: 100px;"
    "background: linear-gradient("
    "135deg,"
    "rgba(50, 70, 155, 0.826),"
    "rgba(72, 20, 145, 0.801)"
    ");"
    "\">"
    "<div class=\"glass\" style=\""
    "width: 100vw;"
    "height: 96px;"
    "padding: 0;"
    "margin: 0;"
    "box-sizing: border-box;"
    "background: linear-gradient("
    "135deg,"
    "rgb(255, 255, 255, 0.1),"
    "rgba(255, 255, 255, 0)"
    ");"
    "backdrop-filter: blur(10px);"
    "-webkit-backdrop-filter: blur(10px);"
    "box-shadow: 0 10px 20px 0 rgba(0, 0, 0, 0.591);"
    "text-align: center;"
    "padding-top: 35px;"
    "font-size: 1.8rem;"
    "letter-spacing: 3px;"
    "font-weight: 700;"
    "color: #ffea00;"
    "\">"
    "TELECOM"
    "</div>"
    "</header>"
    "<div style=\""
    "width: 85vw;"
    "height: 180px;"
    "border-radius: 20px;"
    "background-color: rgba(0, 0, 0, 0.651);"
    "padding: 5px 20px;"
    "display: flex;"
    "flex-direction: column;"
    "justify-content: space-evenly;"
    "align-items: center;"
    "\">"
    "<div class=\"text-title\" style=\""
    "width: 250px;"
    "height: 35px;"
    "display: flex;"
    "align-items: center;"
    "justify-content: center;"
    "padding: 2px;"
    "box-sizing: border-box;"
    "\">"
    "<p style=\""
    "background-image: linear-gradient(to bottom, rgb(255, 0, 0), rgb(209, 0, 0), rgb(255, 255, 255));"
    "color: black;"
    "background-clip: text;"
    "-webkit-background-clip: text;"
    "-webkit-text-fill-color: transparent;"
    "font-weight: 800;"
    "letter-spacing: 2px;"
    "font-size: 1.5rem;"
    "\">"
    "Coordenadas"
    "</p>"
    "</div>"
    "<div class=\"text-title\" style=\""
    "width: 250px;"
    "height: 35px;"
    "display: flex;"
    "align-items: center;"
    "justify-content: center;"
    "padding: 2px;"
    "box-sizing: border-box;"
    "\">"
    "<p style=\""
    "background-image: linear-gradient(to bottom, rgb(255, 0, 0), rgb(209, 0, 0), rgb(255, 255, 255));"
    "color: black;"
    "background-clip: text;"
    "-webkit-background-clip: text;"
    "-webkit-text-fill-color: transparent;"
    "font-weight: 800;"
    "letter-spacing: 2px;"
    "font-size: 1.5rem;"
    "\">"
    "Inválidas!"
    "</p>"
    "</div>"
    "<p style=\""
    "color: white;"
    "width: 290px;"
    "font-size: 1.1rem;"
    "\">"
    "Por favor, retorne até a pagina inicial e insira valores entre -90 e 90."
    "</p>"
    "</div>"
    "<footer style=\""
    "width: 100vw;"
    "height: 100px;"
    "background-color: rgba(0, 0, 0, 0.965);"
    "display: flex;"
    "align-items: start;"
    "justify-content: center;"
    "\">"
    "<p class=\"text-footer\" style=\""
    "color: #ffffffb1;"
    "margin-right: 10px;"
    "margin-top: 44px;"
    "\">"
    "© 2024 Projeto Integrado e Telecom"
    "</p>"
    "</footer>"
    "</body>"
    "</html>";

  //Envia uma mensagem para o usuário, em formato de site, de que as informações foram corretamente coletadas e com o código 200
  String htmlProcess =
    "<!DOCTYPE html>"
    "<html lang=\"pt-br\" style=\"padding: 0; margin: 0; box-sizing: "
    "border-box; font-family: 'Lucida Sans', sans-serif;\">"
    "<head>"
    "<meta charset=\"UTF-8\" />"
    "<meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\" />"
    "<title>Sucesso!</title>"
    "</head>"
    "<body style=\""
    "max-width: 100vw;"
    "min-height: 100vh;"
    "display: flex;"
    "flex-direction: column;"
    "justify-content: space-between;"
    "align-items: center;"
    "padding: 0;"
    "margin: 0;"
    "box-sizing: border-box;"
    "background-color: rgb(178, 216, 154);"
    "\">"
    "<header style=\""
    "width: 100vw;"
    "height: 100px;"
    "background: linear-gradient("
    "135deg,"
    "rgba(50, 70, 155, 0.826),"
    "rgba(72, 20, 145, 0.801)"
    ");"
    "\">"
    "<div class=\"glass\" style=\""
    "width: 100vw;"
    "height: 96px;"
    "padding: 0;"
    "margin: 0;"
    "box-sizing: border-box;"
    "background: linear-gradient("
    "135deg,"
    "rgb(255, 255, 255, 0.1),"
    "rgba(255, 255, 255, 0)"
    ");"
    "backdrop-filter: blur(10px);"
    "-webkit-backdrop-filter: blur(10px);"
    "box-shadow: 0 10px 20px 0 rgba(0, 0, 0, 0.591);"
    "text-align: center;"
    "padding-top: 35px;"
    "font-size: 1.8rem;"
    "letter-spacing: 3px;"
    "font-weight: 700;"
    "color: #ffea00;"
    "\">"
    "TELECOM"
    "</div>"
    "</header>"
    "<div style=\""
    "width: 85vw;"
    "height: 180px;"
    "border-radius: 20px;"
    "background-color: rgba(0, 0, 0, 0.651);"
    "padding: 5px 20px;"
    "display: flex;"
    "flex-direction: column;"
    "justify-content: space-evenly;"
    "align-items: center;"
    "\">"
    "<div class=\"text-title\" style=\""
    "width: 250px;"
    "height: 35px;"
    "display: flex;"
    "align-items: center;"
    "justify-content: center;"
    "padding: 2px;"
    "box-sizing: border-box;"
    "\">"
    "<p style=\""
    "background-image: linear-gradient(to bottom, rgb(60, 255, 0), rgb(0, 209, 0), rgb(255, 255, 255));"
    "color: black;"
    "background-clip: text;"
    "-webkit-background-clip: text;"
    "-webkit-text-fill-color: transparent;"
    "font-weight: 800;"
    "letter-spacing: 2px;"
    "font-size: 1.5rem;"
    "\">"
    "Coordenadas"
    "</p>"
    "</div>"
    "<div class=\"text-title\" style=\""
    "width: 250px;"
    "height: 35px;"
    "display: flex;"
    "align-items: center;"
    "justify-content: center;"
    "padding: 2px;"
    "box-sizing: border-box;"
    "\">"
    "<p style=\""
    "background-image: linear-gradient(to bottom, rgb(60, 255, 0), rgb(0, 209, 0), rgb(255, 255, 255));"
    "color: black;"
    "background-clip: text;"
    "-webkit-background-clip: text;"
    "-webkit-text-fill-color: transparent;"
    "font-weight: 800;"
    "letter-spacing: 2px;"
    "font-size: 1.5rem;"
    "\">"
    "Corretas!"
    "</p>"
    "</div>"
    "<p style=\""
    "color: white;"
    "width: 290px;"
    "font-size: 1.1rem;"
    "\">"
    "Para escolher uma nova configuração, retorne até a página inicial e altere as configurações."
    "</p>"
    "</div>"
    "<footer style=\""
    "width: 100vw;"
    "height: 100px;"
    "background-color: rgba(0, 0, 0, 0.965);"
    "display: flex;"
    "align-items: start;"
    "justify-content: center;"
    "\">"
    "<p class=\"text-footer\" style=\""
    "color: #ffffffb1;"
    "margin-right: 10px;"
    "margin-top: 44px;"
    "\">"
    "© 2024 Projeto Integrado e Telecom"
    "</p>"
    "</footer>"
    "</body>"
    "</html>";

  //Coleta as informações digitadas no site:
  Latitude = server.arg("latitude");
  Longitude = server.arg("longitude");
  Rota = server.arg("route");

  //Converte a latitude e longitude digitadas no tipo String para o tipo Float:
  LatitudeFloat = Latitude.toFloat();
  LongitudeFloat = Longitude.toFloat();

  //Logica que impede que valores invalidos sejam passados para o ESP32
  if ((LatitudeFloat >= -90 && LatitudeFloat <= 90) && (LongitudeFloat >= -90 && LongitudeFloat <= 90)) {

    //Envia a string para o servidor, com o código HTTP 200 (Ok), informando ao navegador que se trata de um HTML
    server.send(200, "text/html", htmlProcess);
  }
  else {

    //Envia a string para o servidor, com o código HTTP 200 (Ok), informando ao navegador que se trata de um HTML
    server.send(200, "text/html", htmlError);
    Serial.println("\nCoordenadas recebidas com valores fora do esperado");
    delay(500);
  }
}

//-----------------------------------------------------------------------------------------

// Função que coleta a informação de chuva do local, baseado nas informações de latitude e longitude digitadas pelo usuário
float getRainData(const String &Latitude, const String &Longitude) {

  // Inicia a requisição HTTP
  HTTPClient http;

  // Cria a url da rquisição
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(Latitude) + "&longitude=" + String(Longitude) + "&current=rain";

  //Exibe a url da requisição da Open Meteo no monitor serial
  Serial.println("\nA url digitada foi: " + String(url));

  // Inicia a requisição GET para a url escolhida
  http.begin(url);
  //Serial.println("A requisicao foi iniciada");
  int httpResponseCode = http.GET();

  //Exibe o codigo de retorno da requisicao HTTP da Open Meteo
  Serial.println("\nCodigo da resposta da OpenMeteoAPI:" + String(httpResponseCode));

  // Valor padrão que indica erros
  float rainValue = -1;

  // Caso o servidor tenha retornado algo
  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Analisa o JSON, com um buffer de tamanho 1024
    const size_t capacity = 1024;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);

    // Coleta o valor do parâmetro rain da chave current
    if (!error) {
      //Serial.println("\nO retorno foi analisado");

      if (doc.containsKey("current")) {
        //Serial.println("O JSON contem a chave current");

        JsonObject current = doc["current"];
        if (current.containsKey("rain")) {
          //Serial.println("A chave curent contem a chave rain \n");

          rainValue = current["rain"];
        }
      }
    }
    else {
      //Serial.println("O retorno não foi analisado");
    }
  }
  else {
    //Serial.println("A requisicao http para a API da Open Meteo não funcionou como deveria");
  }

  // Finaliza a requisição HTTP
  http.end();

  // Returna o valor de chuva como resposta ou -1
  return rainValue;
}

//-----------------------------------------------------------------------------------------

// Função que obtem o custo de uma rota externa específica
void getTemposDeFila(float *returnList) {

  HTTPClient http;

  http.begin("192.168.100.122", 8080);
  int TempoDeFilaHttpResponseCode = http.GET();

  // Valores padrão para indicar erro ou ausência de dados
  float TempoDeFilaR1 = -1, TempoDeFilaR2 = -1, TempoDeFilaR3 = -1, TempoDeFilaR4 = -1, TempoDeFilaR5 = -1, TempoDeFilaR6 = -1, TempoDeFilaR7 = -1, TempoDeFilaR8 = -1, TempoDeFilaR9 = -1, TempoDeFilaR10 = -1;

  //Serial.print("O codigo da requisicao http da API local de Tempo de Fila foi: ");
  //Serial.println(TempoDeFilaHttpResponseCode);

  //Prossegue com o código caso a requisição HTTP para a API local de tempo de fila tenha retornado algo
  if (TempoDeFilaHttpResponseCode > 0) {

    String TempoDeFilaPayload = http.getString();
    const size_t TempoDeFilaCapacity = 2048;
    DynamicJsonDocument doc(TempoDeFilaCapacity);

    DeserializationError error = deserializeJson(doc, TempoDeFilaPayload);

    // Condição que confere se não ocorreram erros na operação com o Json
    if (!error) {
      //Verifica se as chaves de tempo de fila realmente existem no Json
      if (doc.containsKey("r1") && doc.containsKey("r2") && doc.containsKey("r3") && doc.containsKey("r4") && doc.containsKey("r5") && doc.containsKey("r6") && doc.containsKey("r7") && doc.containsKey("r8") && doc.containsKey("r9") && doc.containsKey("r10")) {
        TempoDeFilaR1 = doc["r1"];
        TempoDeFilaR2 = doc["r2"];
        TempoDeFilaR3 = doc["r3"];
        TempoDeFilaR4 = doc["r4"];
        TempoDeFilaR5 = doc["r5"];
        TempoDeFilaR6 = doc["r6"];
        TempoDeFilaR7 = doc["r7"];
        TempoDeFilaR8 = doc["r8"];
        TempoDeFilaR9 = doc["r9"];
        TempoDeFilaR10 = doc["r10"];
      }
    }
    else {
      Serial.println("Ocorreu um erro na desserializacao da requisicao http para a API local de tempo de fila");
    }
  }

  // Exibe os valores de tempo de fila no monitor serial
  Serial.print("\nOs valores de tempo de fila obtidos foram: " + String(TempoDeFilaR1) + ", " + String(TempoDeFilaR2) + ", " + String(TempoDeFilaR3) + ", " + String(TempoDeFilaR4) + ", " + String(TempoDeFilaR5) + ", " + String(TempoDeFilaR6) + ", " + String(TempoDeFilaR7) + ", " + String(TempoDeFilaR8) + ", " + String(TempoDeFilaR9) + " e " + String(TempoDeFilaR10) + "\n");

  http.end();//Finaliza a conexão HTTP

  //Define um tamanho de pacote para realizar a demonstração de funcionamento
  float TP = 1; //1 Megabit

  // Calcula o custo das rotas, baseado em Megabits
  float Fibra1 = TP / 1000 + TempoDeFilaR1 + TP / 800 + TempoDeFilaR5 + TP / 1200 + TempoDeFilaR8;
  float Fibra2 = TP / 750 + TempoDeFilaR2 + TP / 150 + TempoDeFilaR9;
  float Radio1 = TP / 100 + TempoDeFilaR3 + TP / 150 + TempoDeFilaR6 + TP / 80;
  float Radio2 = TP / 120 + TempoDeFilaR4 + TP / 1500 + TempoDeFilaR7;

  Serial.println("\nO custo da rota Fibra1 é de " + String(Fibra1) + ", Fibra2 é de " + String(Fibra2) + ", Radio1 é de " + String(Radio1) + " e Radio2 é de " + String(Radio2) + "\n");

  //Retorna todos os valores de custo de fila
  returnList[0] = Fibra1;
  returnList[1] = Fibra2;
  returnList[2] = Radio1;
  returnList[3] = Radio2;

  //Retorna os melhores valores de custo de cada um dos métodos nas posições 4 e 5:
  if (Fibra1 < Fibra2) {
    returnList[4] = Fibra1;
  }
  else {
    returnList[4] = Fibra2;
  }

  if (Radio1 < Radio2) {
    returnList[5] = Radio1;
  }
  else {
    returnList[5] = Radio2;
  }
}

//-----------------------------------------------------------------------------------------

//Cria as variaveis para a utilização do Millis()
unsigned long millis_Atual = 0, millis_Referencia = 0;

void setup() {

  //Inicia o monitor serial
  Serial.begin(115200);

  //Conecta o ESP32 ao WiFi da rede
  IniciarWiFi();

  //Inicia o CI MT8816 com as configurações definidas anteriormente, declarando seus pinos
  MT8816.begin();

  //Configura o site do projeto e suas rotas
  server.on("/", handleRoot); //Rota principal, onde o site é carregado
  server.on("/process_form", handleProcessForm); //Rota para indicar que todo ocorreu como o esperado

  // Adiciona a função "handle_not_found" quando o servidor estiver offline
  server.onNotFound(handle_not_found);

  //Inicia o servidor
  server.begin();
  Serial.println("\n\n------Servidor Ativo------\n\n");
}

//-----------------------------------------------------------------------------------------

void loop() {

  //Define um intervalo de tempo para lidar somente com a requisição do cliente
  while (millis_Atual < millis_Referencia) {
    millis_Atual = millis();
    //Processa as solicitações padrão do servidor
    server.handleClient();
  }

  //Coleta as informações digitadas no site:
  Latitude = server.arg("latitude");
  Longitude = server.arg("longitude");
  Rota = server.arg("route");

  //Caso algo tenha sido digitado, prossegue com o codigo
  if (Rota != "" && Latitude != "" && Longitude != "") {

    //Converte a latitude e longitude digitadas no tipo String para o tipo Float:
    LatitudeFloat = Latitude.toFloat();
    LongitudeFloat = Longitude.toFloat();

    //Exibe os valores de rota, latitude e longitude obtidos digitados no site
    Serial.println("\nOs valores foram inseridos, sendo Latitude = " + String(Latitude) + ", Longitude = " + String(Longitude) + " e Rota = " + String(Rota));

    //Processo feito pelo ESP32 para a transmissão interna
    if (Rota == "Interno") {
      // Define o CI para realizar broadcast internamente
      Serial.println("\n--Modo de Roteamento Interno--\n");

      // Define o endereço para realizar broadcast internamente no MT8816
      if (interface != 0) {
        MT8816.reset();
        MT8816.broadcast(x_arr, y_arr);
        interface = 0;
      }

      //Delay que impede que essa condição seja executada infinitamente de maneira rápida
      delay(500);
    }

    else if (Rota == "Externo") {
      if ((LatitudeFloat >= -90 && LatitudeFloat <= 90) && (LongitudeFloat >= -90 && LongitudeFloat <= 90)) {

        //Counter para não fazer muitas requisições:
        if (counter == 0) {

          Serial.println("Verificando coordenas, valores dentro do esperado [-90, 90]");

          //Descobre se está chovendo
          Chuva = -1;

          while (Chuva < 0) {
            Serial.println("Obtendo valores da chuva...");
            Chuva = getRainData(Latitude, Longitude);
          }

          //Exibe o valor de chuva obtido
          Serial.print("Chuva atual (mm): ");
          Serial.println(Chuva);

          //Faz a requisição do tempo das rotas no IP 192.168.100.122 e configura o array ResultadoCusto
          getTemposDeFila(ResultadoCusto);

          //Redefine x para 0 para que o ciclo recomeçar
          counter = 6;
        }

        counter--; //Dimunui o valor x para após 6 ciclos chamar as API's novamente

        if (Chuva > 0) {

          Serial.println("\n--Modo de Roteamento Externo via Fibra Optica--\n");

          float CustoFibra1 = ResultadoCusto[0];
          float CustoFibra2 = ResultadoCusto[1];
          float MenorCustoFibra = ResultadoCusto[4];

          if (CustoFibra1 >= 0 && CustoFibra2 >= 0) {
            if (CustoFibra1 == MenorCustoFibra) {

              Serial.println("Fibra Optica 1");

              if (interface != 1) {
                MT8816.reset();
                MT8816.setConnection(9, 2);
                interface = 1;
              }
            }
            else if (CustoFibra2 == MenorCustoFibra) {

              Serial.println("Fibra Optica 2");

              if (interface != 2) {
                MT8816.reset();
                MT8816.setConnection(10, 2);
                interface = 2;
              }
            }
          }
        }
        else if (Chuva == 0) {

          Serial.println("\n--Modo de Roteamento Externo via Radio--\n");

          float CustoRadio1 = ResultadoCusto[2];
          float CustoRadio2 = ResultadoCusto[3];
          float MenorCustoRadio = ResultadoCusto[5];

          if (CustoRadio1 >= 0 && CustoRadio2 >= 0) {
            if (CustoRadio1 == MenorCustoRadio) {

              Serial.println("Radio 1");

              if (interface != 3) {
                MT8816.reset();
                MT8816.setConnection(11, 2);
                interface = 3;
              }
            }

            else if (CustoRadio2 == MenorCustoRadio) {

              Serial.println("Radio 2");

              if (interface != 4) {
                MT8816.reset();
                MT8816.setConnection(12, 2);
                interface = 4;
              }
            }
          }
        }
      }
      delay(500);
    }
  }
  else {
    Serial.println("A rota ainda nao foi escolhida pelo usuario");
    delay(1000);
  }

  //Retorna os valores abaixo para o padrão inicial
  Rota = "";
  Latitude = "";
  Longitude = "";
  LatitudeFloat = 0.0;
  LongitudeFloat = 0.0;

  //Configura o contador para que o servidor funcione corretamente no próximo ciclo do looping
  millis_Atual = millis();
  millis_Referencia = millis_Atual + 1500;
}
