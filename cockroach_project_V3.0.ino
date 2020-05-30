
/*
 * Project: [Mechanical Cockroach] - Wilhelm Ludwig, Ferhenbacher Nikolay [MEB-1]
 * Autor of this code: Nikolay Fehrenbacher MEB-1
 * 
 * 
 * Project work:
 * Wilhelm Ludwig => cosntruction of the cockroach {electronics, mechanics, circuit board}
 * Fehrenbacher Nikolay => programming, web-design, electronics
 * 
 * 
 * GPiO Pins on ESP32_Dev_Module:
 * 
 * servo 1 = 14 // Moovement
 * servo 2 = 12 // Tilt
 * power_button = 15
 * left_button = 4
 * right_button = 2
 */

 
/****************************************************{LIBRARIES}***************************************************/
#include <WiFi.h>   /* WiFi_Library */
#include <Servo.h>  /* Servo_Library */

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

TaskHandle_t Task_0;  // Create Task_0
TaskHandle_t Task_1;  // Create Task_1


/***************************************************{WiFi_SETUP}***************************************************/
const char* ssid     = "Project U-001";  /* SSID Name of the ESP32_Access_Point */
const char* password = "esp32u001";     /* password. Set [password = NULL;] to connect to ESP32 WiFi with no password */

WiFiServer server(80);    /* Set web server port number to 80 */

String header;  /* Variable to store the HTTP request */



/******************************************************{PINS}*****************************************************/
static const int PIN_SERVO_1 = 14;
static const int PIN_SERVO_2 = 12;

const byte PIN_PWR = 15;
const byte PIN_LEFT = 4;
const byte PIN_RIGHT = 2;


/****************************************************{VARIABLES}***************************************************/
Servo servo_1;
Servo servo_2;

int i = 0;
char m = 4;

char state = 0;

String forward = "off";
String backward = "off";
String left = "off";
String right = "off";
String hold = "off";
String power = "off";
String power_on = "off";
String power_off = "off";


/*************************************************{HARDWARE_TIMERS}*************************************************/
/* create a hardware timer */
hw_timer_t * timer_0 = NULL;

void IRAM_ATTR onTimer_0(){
  i++;
      if(i == 4){
        i=0;
      
      }
}

/****************************************************{INTERRUPTS}***************************************************/

void IRAM_ATTR btn_triggered(){
  m = 5;
}

void IRAM_ATTR pwr_bbtn(){
  state = 0;
}


/******************************************************{SETUP}******************************************************/
void setup() {
  Serial.begin(115200); 

  pinMode(PIN_PWR, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  
  servo_1.attach(PIN_SERVO_1);
  servo_2.attach(PIN_SERVO_2);

  attachInterrupt(digitalPinToInterrupt(PIN_PWR), pwr_bbtn, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_LEFT), btn_triggered, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), btn_triggered, FALLING);
  
  /* timer setup */
  timer_0 = timerBegin(0, 80, true);                /* (Use timer_0 (4 Timers on board), 1 tick take 1/(80MHZ/80) = 1us so divider = 80, count up) */
  timerAttachInterrupt(timer_0, &onTimer_0, true);  /* Attach onTimer_0 function to timer_0 */
  timerAlarmWrite(timer_0, 850000, true);          /* (Set alarm to call onTimer_0,  [1 tick = 1us] => [1000000 ticks = 1000000us = 1s], repeat the alarm) */
  timerAlarmEnable(timer_0);                        /* Start an alarm */
  Serial.println("start timer");
  
  /* 2_cores_setup */
  xTaskCreatePinnedToCore(Task_0_code, "Task_0", 10000, NULL, 1, &Task_0, 0); /*create a task that will be executed in the "Task_0_code()"- function, with priority 1 and executed on core 0*/
  delay(500);             /* (Task function, name of task, Stack size of task, parameter of the task, priority of the task, Task handle to keep track of created task, pin task to core 0) */                  
  xTaskCreatePinnedToCore(Task_1_code, "Task_1", 10000, NULL, 1, &Task_1, 1); /*create a task that will be executed in the "Task_1_code()"- function, with priority 1 and executed on core 1*/
  delay(500);             /* (Task function, name of task, Stack size of task, parameter of the task, priority of the task, Task handle to keep track of created task, pin task to core 1) */                 

  /* Wifi Setup */
  Serial.print("Setting AP (Access Point)…"); /* Connect to Wi-Fi network with SSID and password */
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

/******************************************************{CORE_0}****************************************************/
void Task_0_code( void * pvParameters ){
  Serial.print("Task_0 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed=1;
    TIMERG0.wdt_wprotect=0;

    if(state == 1){
      moovement(m);
    }else if (state == 0){
      moovement(4);
    }
    
  } 
}


/******************************************************{CORE_1}****************************************************/
void Task_1_code( void * pvParameters ){
  Serial.print("Task_1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    server_function();
  }
}


/****************************************************{FUNCTIONS}****************************************************/
void loop() {
  
}

void moovement(char m){
      switch(m){
        case 0: func_forward(i);  break;
        case 1: func_backward(i); break;
        case 2: func_left(i);     break;
        case 3: func_right(i);    break;
        case 4: func_hold();      break;
        case 5: func_wall(i);     break;
        
        default:   Serial.println("Error in movvement function");   break;
    }
}

void func_wall(char wa){  
             switch(wa){
                  case 0:servo_1.write(62);    break;  
                  case 1:servo_2.write(10);    break;
                  case 2:servo_1.write(120);   break;
                  case 3:servo_2.write(170);   break;

                  default:  Serial.println("Error in func_wall function");   break;
             }
}

void func_forward(char fw){
             switch(fw){
                  case 0 : servo_1.write(120);  break;  
                  case 1 : servo_2.write(10);   break;
                  case 2 : servo_1.write(62);   break;
                  case 3 : servo_2.write(170);  break;

                  default :   Serial.println("Error in func_forward function");  break;
             }
}


void func_backward(char bw){
             switch(bw){
                  case 0:servo_1.write(62);    break;  
                  case 1:servo_2.write(10);    break;
                  case 2:servo_1.write(120);   break;
                  case 3:servo_2.write(170);   break;

                  default:   Serial.println("Error in func_backward function");    break;
             }
}

void func_left(char lt){
             switch(lt){
                  case 0:servo_1.write(62);   break;  
                  case 1:servo_2.write(90);   break;
                  case 2:servo_1.write(120);  break;
                  case 3:servo_2.write(10);  break;

                  default:   Serial.println("Error in func_left function");  break;
             }
}


void func_right(char rt){
             switch(rt){
                  case 0:servo_1.write(120);   break;  
                  case 1:servo_2.write(90);   break;
                  case 2:servo_1.write(62);  break;
                  case 3:servo_2.write(170);  break;

                  default:   Serial.println("Error in func_right function");  break;
             }
}

void func_hold(){
  servo_1.write(90);
  servo_2.write(90);
}


void server_function (){

  WiFiClient client = server.available();   /*  Listen for incoming clients */

  if (client) {                             /* If a new client connects, */
    Serial.println("New Client.");          /* print a message out in the serial port */
    String currentLine = "";                /* make a String to hold incoming data from the client */
    while (client.connected()) {            /* loop while the client's connected */
      if (client.available()) {             /* if there's bytes to read from the client, */
        char c = client.read();             /* read a byte, then */
        Serial.write(c);                    /* print it out the serial monitor */
        header += c;
        if (c == '\n') {                    /* if the byte is a newline character */

          if (currentLine.length() == 0) {  /* if the current line is blank, you got two newline characters in a row. that's the end of the client HTTP request, so send a response: */

            client.println("HTTP/1.1 200 OK");          /* HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) */
            client.println("Content-type: text/html");  /* and a content-type so the client knows what's coming, then a blank line: */
            client.println("Connection: close");
            client.println();
            
            /*********************************************{ PIN_LOGIC }*********************************/
            if (header.indexOf("GET /forward") >= 0) {  /* turns the GPIOs on and off */
              Serial.println("Mooving forward");
              forward = "on";
              backward = "off";
              left = "off";
              right = "off";
              hold = "off";
              m = 0;
              
            } else if (header.indexOf("GET /backward") >= 0) {
              Serial.println("Mooving backward");
              forward = "off";
              backward = "on";
              left = "off";
              right = "off";
              hold = "off";
              m = 1;
              
            } else if (header.indexOf("GET /left") >= 0) {
              Serial.println("Mooving left");
              forward = "off";
              backward = "off";
              left = "on";
              right = "off";
              hold = "off";
              m = 2;
              
            } else if (header.indexOf("GET /right") >= 0) {
              Serial.println("Mooving right");
              forward = "off";
              backward = "off";
              left = "off";
              right = "on";
              hold = "off";
              m = 3;
              
            } else if (header.indexOf("GET /hold") >= 0) {
              Serial.println("Mooving hold");
              forward = "off";
              backward = "off";
              left = "off";
              right = "off";
              hold = "on";
              m = 4;
              
            } else if (header.indexOf("GET /power_on") >= 0) {
              power = "on";
              state = 1;
              
            } else if (header.indexOf("GET /power_off") >= 0) {
              power = "off";
              state = 0;
              
            }           
            

            
            /*  Display the HTML web page */
            client.println("<!DOCTYPE html> <html>");
            client.println("<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            /* CSS to style the on/off buttons */
            client.println("<style> html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #9e0202; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #179e02;}</style></head>");
                                     
            client.println("<body> <h1> ESP32 Web Server </h1>"); /* Web Page Heading */

            /*********************************************{ wFORWRD }*********************************/
            if (forward=="off") { /* If the output26State is off, it displays the ON button */
              client.println("<p> <a href=\"/forward\"> <button class=\"button\"> Nach vorne </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/forward\"> <button class=\"button button2\"> Nach vorne </button> </a> </p>");
            } 


            /*********************************************{ wBACKWARD }*********************************/   
            if (backward=="off") {   /* If the output27State is off, it displays the ON button  */
              client.println("<p> <a href=\"/backward\"> <button class=\"button\"> Nach hinten </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/backward\"> <button class=\"button button2\"> Nach hinten </button> </a> </p>");
            }

            /*********************************************{ wLEFT }*********************************/   
            if (left=="off") {   /* If the output27State is off, it displays the ON button  */
              client.println("<p> <a href=\"/left\"> <button class=\"button\"> Links </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/left\"> <button class=\"button button2\"> Links </button> </a> </p>");
            }

            /*********************************************{ wRIGHT }*********************************/   
            if (right=="off") {   /* If the output27State is off, it displays the ON button  */
              client.println("<p> <a href=\"/right\"> <button class=\"button\"> Rechts </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/right\"> <button class=\"button button2\"> Rechts </button> </a> </p>");
            }

            /*********************************************{ wHOLD }*********************************/   
            if (hold=="off") {   /* If the output27State is off, it displays the ON button  */
              client.println("<p> <a href=\"/hold\"> <button class=\"button\"> Halten </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/hold\"> <button class=\"button button2\"> Halten </button> </a> </p>");
            }

            /*********************************************{ wPOWER }*********************************/   
            if (power=="off") {   /* If the output27State is off, it displays the ON button  */
              client.println("<p> <a href=\"/power_on\"> <button class=\"button\"> AUS </button> </a> </p>");
            } else {
              client.println("<p> <a href=\"/power_off\"> <button class=\"button button2\"> AN </button> </a> </p>");
            }


            
            client.println("</body> </html>");
            
            client.println(); /* The HTTP response ends with another blank line */
  
            break;  /* Break out of the while loop */
          } else { /* if you got a newline, then clear currentLine */ 
            currentLine = "";
          }
        } else if (c != '\r') {  /* if you got anything else but a carriage return character, */ 
          currentLine += c;      /* add it to the end of the currentLine */ 
        }
      }
    }
    
    header = "";  /* Clear the header variable */
    
    client.stop();    /* Close the connection */
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
