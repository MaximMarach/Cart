#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#define MOTOR_RIGHT 5
#define MOTOR_LEFT 4

const char* ssid = "ESP8266";  // название сети
const char* password = "12345678"; // пароль

IPAddress local_ip(192,168,1,1); //ip к которому нужно подключиться  
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(3456);
WiFiUDP udp;
bool is_x_negativ = false;
double stnd_left_speed = 440;
double stnd_right_speed = 440;

/* средняя скорость колес */
double get_speed(double x,double y){
  is_x_negativ = x < 0 ? true : false;
  Serial.print("left: ");
  Serial.println(is_x_negativ);// показывает куда поворачивает пользователь(1 - лево, 0 - право)
  double speed = sqrt(x*x + y*y);
  Serial.print("Speed: ");
  Serial.println(speed);
  return speed;
}
/* возвращает угол поворота в два раза меньше действительности (в радианах)*/
double get_corner(double x, double y){
  y = fabs(y);
  double sin = y / get_speed(x, y);
  double rad = asin(sin);
  double corner = rad * 180 / 3.14;
  if(is_x_negativ){ 
    corner = 180 - corner;
  } 
  corner /= 2;
  rad = corner * 3.14 / 180;
  Serial.print("Corner: ");
  Serial.println(corner);
  return rad;
}


void setup() {
  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(MOTOR_RIGHT, OUTPUT);
  analogWriteResolution(10); // макс. скорость 1024
  Serial.begin(9600);
  delay(1000);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  udp.begin(1234);
  server.begin();
}
void loop(){
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    char packetData[packetSize + 1];
    udp.read(packetData, packetSize);
    packetData[packetSize] = 0;
    Serial.println("Contents:");
    Serial.println(packetData);
    String x_data = ""; // значения по x
    String y_data = ""; // значения по y
    bool is_Find_x = false;

    // парсинг данных из приложения
    for(int i = 1; i <  packetSize; i++){
        if(packetData[i] == ':'){
          i++;          
          is_Find_x = true;
        }
        if(!is_Find_x){
          x_data+=packetData[i];
        }else{
          y_data+=packetData[i];
        }
    } 
    double alpha = 25; // коэффициент для управления скоростью
    double speed = get_speed(x_data.toDouble(), y_data.toDouble());
    double corner = get_corner(x_data.toDouble(), y_data.toDouble());
    double speed_left = alpha * cos(corner) ;
    double speed_right = alpha * sin(corner) ;

    // условие для торможения
    if(y_data.toDouble() >= 100 | (y_data.toDouble() == 0 & x_data.toDouble() ==0)){ 
      analogWrite(MOTOR_LEFT,0);
      analogWrite(MOTOR_RIGHT,0); 
    }else{
      analogWrite(MOTOR_LEFT,speed_left + stnd_left_speed);
      analogWrite(MOTOR_RIGHT,speed_right + stnd_right_speed);
      Serial.print("Speed_left: ");
      Serial.println(speed_left);
      Serial.print("Speed_right: ");
      Serial.println(speed_right);
    }   

  }
}








