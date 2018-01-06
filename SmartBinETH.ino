//Project
//#include <VirtualWire.h>
#include <SPI.h>
#include <Ethernet.h>

#define Data_Types int
#define DELAY 500
#define CONDITION FALLING
#define DUSTBIN_NUM 0001

void format_Data (void);
void check_distance (void);
void sendMsgToServer (void);

void sendWebPage (void);


volatile int position = 0;
volatile boolean direction = false;

unsigned long lastmillis = 0;
unsigned long lastmillis0 = 0;
boolean checkForOnFlag = false;


int tankHight = 0;//in cm meters
int sensorHight = 6; //in cm meters

const int trigPin = 9;
const int echoPin = 6;

// defines variables
long duration;
int pos = 0;
//const int  Transmitter_data_pin = 7;

int distance = 9;

Data_Types id = DUSTBIN_NUM;
Data_Types level = 6;

String serialRead = "";
char data[50] = "";

Data_Types BitwiseData = 0;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(169, 254, 158, 61);
//IPAddress ip(192, 168, 1, 177);


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()  
{
  Serial.begin(9600);

  // Initialize the IO and ISR
//  vw_setup(2000); // Bits per sec
//  vw_set_tx_pin(Transmitter_data_pin);

  //attachInterrupt(0,updatePositionLOW, LOW);
  attachInterrupt(0, updatePositionRISING, RISING);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  SPI.begin();
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  //   delay(2000);
  sendWebPage ();
}

/*
void format_Data (void)
{
  BitwiseData = 0xffff & id;

  if (sizeof (Data_Types) == 2)
  {
    //if short or int
    BitwiseData = BitwiseData | (level << 8);
    Serial.println("SHORT or INT  ");
  }
  else
  {
    //if char
    BitwiseData = BitwiseData | (level << 4);
    Serial.println("CHAR  ");
  }
  Serial.print("BitwiseData  ");
  Serial.println(BitwiseData);

  String thisString = String(BitwiseData);
  thisString.toCharArray(data, 50);
  send(data);

}

void send (char *message)
{
  vw_send((uint8_t *)message, strlen(message));
  delay (5000);
  //vw_wait_tx(); // Wait until the whole message is gone
}
*/


void sendMsgToServer (void)
{
  int temp = 0;
  temp = tankHight - (distance - sensorHight);
  if (temp > 0)
  {
    level = temp * 10 / tankHight;
  }
  else
  {
    Serial.println("wrong reading");
    return;
  }

  Serial.print("level  ");
  Serial.println(level);

  if ((distance <= (sensorHight + tankHight)) && (distance >= (sensorHight + (tankHight / 2))))
  {
    //EMPTY
    Serial.println("EMPTY");
  }
  else if ((distance <= (sensorHight + tankHight / 2)) && (distance >= sensorHight))
  {
    //HALF FULL
    Serial.println("HALF FULL");
  }
  else if (distance < sensorHight)
  {
    //FULL
    Serial.println("FULL");
  }

//  format_Data ();
}

void updatePositionLOW ()
{
  Serial.println("IR interrupt --updatePositionLOW");
  detachInterrupt (0);
  delay (500);
  attachInterrupt(0, updatePositionRISING, RISING);

}


void updatePositionRISING()
{
  Serial.println("IR interrupt --Sensor RISINFG");


  if (lastmillis == 0)
  {
    detachInterrupt (0);
    delay (500);
    Serial.println("RISINFG --- lastmillis zero");
    lastmillis = millis ();
    check_distance ();
    tankHight = distance - sensorHight;
    attachInterrupt(0, updatePositionLOW, LOW);
  }
  if ((millis () - lastmillis) >= (1000 * 4))
  {
    detachInterrupt (0);
    delay (500);

    lastmillis = millis ();

    Serial.println("RISINFG --- lastmillis non zero");

    check_distance ();
    sendMsgToServer();
    attachInterrupt(0, updatePositionLOW, LOW);
  }
}




void check_distance (void)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}



void sendWebPage (void)
{
  // listen for incoming clients
  // Serial.println("new client");
  EthernetClient client = server.available();
//  Serial.println(client);
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();

          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("<p>Bin No. ");
          client.print(id);
          client.println("  </p>");
          client.println("<head>");
          client.println("<style>");
          client.println("table, th, td {");
          client.println("border: 1px solid black;");
          client.println("border-collapse: collapse;");
          client.println("}");
          client.println("th, td {");
          client.println("padding: 15px;");
          client.println("}");
          client.println("</style>");
          client.println("</head>");
          client.println("<body>");
          client.println("<table style=\"width:10%\">");
          for (int analogChannel = 0; analogChannel < ( 10 - level); analogChannel++) {
            client.println("<tr>");
            client.println("<th></th>");
            client.println("</tr>");
          }
          for (int analogChannel = 0; analogChannel < level; analogChannel++) {
            client.println("<tr>");
            client.println("<td bgcolor=\"#FF0000\"></td>");
            client.println("</tr>");
          }
          client.println("</table>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}