/*
* ir Pin = 3
* curl -X GET -H "Content-Type: application/json" http://192.168.1.73/ -d "{'data': 'data'}"
*/
#include <uHTTP.h>
#include <ArduinoJson.h>

byte macaddr[6] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

/*
* En caso de usar ip estatica
* byte ip4addr[4] = {192, 168, 0, 1};
*/

uHTTP *Server = new uHTTP(80);
EthernetClient Response;

void setup(){
  //Inicializamos la libreria de ethernet y sus confirugraciones
  Ethernet.begin(macaddr);
  /*
  * En caso de usar ip estatica
  * Ethernet.begin(macaddr, ip4addr);
  */
  //Inicializamos Servidor de peticciones
  Server->begin();
  //Inicializamos Serial
  Serial.begin(115200);
  Serial.print(F("Starting at "));
  Serial.print(Ethernet.localIP());
  Serial.println(":80");
}

void loop(){
  //Mientras exista una peticion creamos logica
  if(Response = Server->available()){
    //Validamos la ruta que envia el cliente
    if(Server->uri("/")){
      // Logica del metodo GET
      if(Server->method(uHTTP_METHOD_GET)){
          //Usando ArduinoJson obtenemos el JSON del body
          StaticJsonDocument<200> doc;
          deserializeJson(doc, Server->body());
          //Extraemos un valos por individual
          const char* data = doc["data"];
          Serial.print(data);
          String message = "success message";
          //Retornamos con los headers necesarios
          return_headers(200, message);
      }
      // Logica del metodo POST
      if(Server->method(uHTTP_METHOD_POST)){
          //Retornamos con los headers necesarios
          return_headers(405,"");
      }
    }
    //Si la ruta no existe mandamos error
    else{
      //Retornamos con los headers necesarios
      return_headers(400,"");
    }
  }
}

void return_headers(uint16_t code, String message){
  //Necesario para el contenido correcto del header
  header_t head = Server->head();
  //Inicializamos el JSON de retorno 
  StaticJsonDocument<256> doc;
  JsonObject root = doc.to<JsonObject>();
  //Inicializamos Datos en el contenido del JSON
  JsonObject data = root.createNestedObject("data");
  //Agregamos Headers y contenido en el JSON
  switch(code){
    case 200:
      root["status"] = "success";
      Response.println(F("HTTP/1.0 200 OK"));
      data["detail"] = message;
      serializeJsonPretty(root, Serial);
      break;
    case 400:
      root["status"] = "error";
      Response.println(F("HTTP/1.0 400 Bad Request"));
      data["detail"] = "Bad Request";
      serializeJsonPretty(root, Serial);
      break;
    case 405:
      root["status"] = "error";
      Response.println(F("HTTP/1.0 405 Method Not Allowed"));
      data["detail"] = "Method Not Allowed";
      serializeJsonPretty(root, Serial);
      break;
  }
  //Agregamos mas headers
  Response.println(F("Content-Type: application/json"));
  Response.println(F("Connection: close"));
  Response.print(F("Content-Length: "));
  Response.println(measureJsonPretty(root));
  Response.println();
  //Retornamos JSON a cliente
  serializeJsonPretty(root, Response);
  Response.println();
  //Mandamos a cerrar el request
  Response.stop();
}