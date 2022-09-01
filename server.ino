#include <ArduinoJson.h>
#include <ESP8266WiFi.h> // ESP 8266 와이파이 라이브러리
///////////////////////////
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

int value;
HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);
///////////////////////////

int Relaypin = 0;
int count = 0;
//MAC주소
uint8_t MAC_array[6];
char MAC_char[18];
String buf;

char ssid[] = "bssm_guest";
char pass[] = "bssm_guest";
int status = WL_IDLE_STATUS;

WiFiClient client;
String line;
String li;

void setup() {
  Serial.begin(115200);
  ///////////////////////////////
  Wire.begin();
  while (!huskylens.begin(Wire))
  {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
  //////////////////////////////
  //MAC주소
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    sprintf(MAC_char, "%s%02x:", MAC_char, MAC_array[i]);
  }
  delay(500);
  WiFi.begin("bssm_guest", "bssm_guest"); // 공유기 이름과 비밀번호

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) // 와이파이 접속하는 동안 "." 출력
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP()); // 접속된 와이파이 주소 출력
  
}


void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("ID=") + result.ID);
    value = result.ID;
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}


void fetch(int objectId){
    String jsondata = ""; //문자열 형태로 전송하기위해 문자열을 선언한다
  StaticJsonBuffer<200> jsonBuffer; //JSON변환에 필요한 StaticJsonBuffer을 선언정하고 용량을 설정한다.
  JsonObject& root = jsonBuffer.createObject(); //JsonObject를 선언하여 JSON형식을 만들어 낼 수 있언는 객체를 선언
  root["macadress"] = MAC_char;
  root["objectId"] = objectId;
  root.printTo(jsondata); //JSON으로 포멧된 데이터를 String변수로~

  Serial.println(jsondata); //jsondata를 String으로 출력


  int re = client.connect("192.168.10.220", 8000);
  if (re) {
    client.println("GET /test HTTP/1.1");
    client.println("Host: 192.168.10.220:8000");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsondata.length());
    client.println();
    client.println(jsondata);
    li = client.readString();
    for (int i = 0; li[i] != '\0'; i++) {
      if (count > 7) {
        buf += li[i];
      }
      if (li[i] == '\r') {
        count++;
      }
    }
    Serial.println(buf);
    buf = "";
    count = 0;
  }
}




void loop() {
  /////////////////

  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else
  {
    Serial.println(F("###########"));
    while (huskylens.available())
    {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
    }
    fetch(value);
  }
}



//앞으로 buf에 저장된 String을 Parsing 해야함//
