#include <SoftwareSerial.h>
SoftwareSerial esp8266(19,18);
#include <Servo.h>
//Initialize variables 
int topLeftLight = 0;     
int topRightLight = 0;
int bottomLeftLight = 0;
int bottomRightLight = 0;
int LeftLight = 0;
int RightLight = 0;
int TopLight = 0;
int BottomLight = 0;
int i = 0;
int battPin = A0;
//Declare two servos
Servo servo_9;
Servo servo_10;

void setup()
{
    //pinMode(battPin, INPUT);
    pinMode(A2, INPUT);   //Light sensor up - left
    pinMode(A3, INPUT);   //Light sensor bottom - left
    pinMode(A4, INPUT);   //Light sensor bottom - right
    pinMode(A5, INPUT);   //Light sensor top - right
    servo_9.attach(9);    //Servo motor right - left movement
    servo_10.attach(10);  //Servo motor up - down movement
    Serial.begin(9600);      // PC Arduino Serial Monitor
    Serial1.begin(115200);   // Arduino to ESP01 Communication
    connectWiFi();           // To connect to Wifi
}
void loop()
{
    if( i == 360 ){
        Serial.println("Connect Battery");
        delay(5000);
        int voltReading = analogRead(battPin);
        float v = voltReading/600.0;
        int volts = v*100;
        Serial1.println("AT+CIPMUX=0\r\n");      // To Set MUX = 0
        delay(1000);                             // Wait for 1 sec
  
        // TCP connection 
        String cmd = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n\r\n";   // TCP connection with https://thingspeak.com server
        Serial1.println(cmd);                    // Display above Command on PC
        Serial.println(cmd);                     // Send above command to Rx1, Tx1
        delay(1000);                            // Wait for 1 Sec
        if(Serial1.find("ERROR"))                // If returns error in TCP connection
        { 
            Serial.println("AT+CIPSTART error");   // Display error msg to PC 
        }
  
        // prepare GET string 
        String getStr = "GET /update?api_key=W6PALPXVIU8UBW62&field1=" + (String)volts + "\r\n\r\n"; // Add apiKey and Sensor Value
        Serial.println(getStr);                 // Display GET String on PC
        cmd = "AT+CIPSEND="+String(getStr.length())+"\r\n";                    // send data length 
        Serial.println(cmd);                   // Display Data length on PC
        Serial1.println(cmd);                  // Send Data length command to Tx1, Rx1
        delay(3000);                           // wait for 3 sec
        if(Serial1.find(">"))                  // If prompt opens //verify connection with cloud
        {
            Serial.println("connected to Cloud");  // Display confirmation msg to PC
            Serial1.print(getStr);                 // Send GET String to Rx1, Tx1
        }
        else
        { 
            Serial1.println("AT+CIPCLOSE\r\n");    // Send Close Connection command to Rx1, Tx1
            Serial.println("AT+CIPCLOSE");         // Display Connection closed command on PC
        } 
  
        // thingspeak free version needs 16 sec delay between updates 
        delay(16000);       
        i = 0;
    } else {
        //if mode is auto map the sensor values to 0-100 ligh intensity. 
        //Every light sensor has different sensitivity and must be first tested 
        //for it's high and low values
        topLeftLight = map(analogRead(A2),50,900,0,100);
        topRightLight = map(analogRead(A5),200,990,0,100);
        bottomLeftLight = map(analogRead(A3),130,970,0,100);
        bottomRightLight = map(analogRead(A4),250,950,0,100);
        //Calculate the average light conditions
        TopLight = ((topRightLight + topLeftLight) / 2);
        BottomLight = ((bottomRightLight + bottomLeftLight) / 2);
        LeftLight = ((topLeftLight + bottomLeftLight) / 2);
        RightLight = ((topRightLight + bottomRightLight) / 2);
        //Rotate the servos if needed
        if (abs((RightLight - LeftLight)) > 10) {    //Change position only if light difference is bigger then 4%
            if (RightLight < LeftLight) {
                if (servo_9.read() <= 180) {
                    servo_9.write((servo_9.read() - 5));
                    delay(100);
                }
            }
            if (RightLight > LeftLight) {
                if (servo_9.read() >= 0) {
                    servo_9.write((servo_9.read() + 5));
                    delay(100);
                }
            }
        }
        if (abs((TopLight - BottomLight)) > 10) {  //Change position only if light difference is bigger then 4%
            if (TopLight < BottomLight) {
                if (servo_10.read() <= 180) {
                    servo_10.write((servo_10.read() - 5));
                    delay(100);
                }
            }
            if (TopLight > BottomLight) {
                if (servo_10.read() >= 0) {
                    servo_10.write((servo_10.read() + 5));
                    delay(100);
                }
            }
        }
        i = i + 1;
    }
}
boolean connectWiFi() {               // Connect to Wifi Function
    Serial1.println("AT+CWMODE=1\r\n"); // Setting Mode = 1 
    delay(1000);                         // wait for 100 mSec
  
    String cmd = "AT+CWJAP=\"aniket\",\"9619433241\"\r\n";         // Connect to WiFi
    Serial.println(cmd);                // Display Connect Wifi Command on PC
    Serial1.println(cmd);               // send Connect WiFi command to Rx1, Tx1 
    delay(5000);                       // wait for 10 sec
    Serial1.println("AT+CWJAP?");       // Verify Connected WiFi
    if(Serial1.find("+CWJAP"))        
    {
        Serial.println("OK, Connected to WiFi.");         // Display Confirmation msg on PC
        return true;
    }
    else
    {
        Serial.println("Can not connect to the WiFi.");   // Display Error msg on PC
        return false;
    }
}
