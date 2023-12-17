#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <stdlib.h>

#define FIREBASE_HOST   "histology-746c2-default-rtdb.firebaseio.com"  //https://histology-746c2-default-rtdb.firebaseio.com/
#define FIREBASE_AUTH   "969udpwVGtP0FR0vw90SrZXEr8EMXUAkfl93gltg"

//#define WIFI_SSID "HSBP ROBOTICS"
//#define WIFI_PASSWORD "robot123".

#define WIFI_SSID "histobot"
#define WIFI_PASSWORD "histo12345"

#define ev3clock 5 //D1
#define datain 4 //D2
#define dataout 12 //D6
#define request 14 //D5

int i;
int x;
int timing = 0;
int data = 0;
int sum = 0;
int type = 0;
int notified = 0;
bool requestmode = false;
int done = 0;

void ConnectWiFi();
void SetParameter();
void SetPin();
void ReadInput();
void ReadBinary();
void WaitRequest();
void WriteBinary();
void Mode();

void setup() {
  Serial.begin(115200);
  ConnectWiFi();
  SetPin();
  SetParameter();
}

void loop() {
  if (requestmode == false) {
    ReadBinary();
    Mode();
  }
  else {
    WaitRequest();
  }
}



void ConnectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("/Histomed/Firebase Status", "Started");
}

void SetParameter() {
  Firebase.setString("/Histomed/HistoBot Status", "Offline");
  Firebase.setString("/Histomed/ID database/100/Status" , "Waiting");
  Firebase.setString("/Histomed/ID database/105/Status" , "Waiting");
  Firebase.setString("/Histomed/ID database/117/Status" , "Waiting");
  Firebase.setString("/Histomed/ID database/124/Status" , "Waiting");

  Firebase.setString("/Histomed/ID database/133/Status" , "Dehydration2");
  Firebase.setString("/Histomed/ID database/145/Status" , "Dehydration2");
  Firebase.setString("/Histomed/ID database/151/Status" , "Dehydration2");

  Firebase.setString("/Histomed/ID database/179/Status" , "Infiltration");
  Firebase.setString("/Histomed/ID database/193/Status" , "Infiltration");
  Firebase.setString("/Histomed/Notification" , "0");
  Firebase.setString("/Histomed/Sectioning" , "0");
  Firebase.setString("/Histomed/Request" , "0");
  Firebase.setString("/Histomed/Busy" , "1");

  Firebase.setString("/Histomed/Embedding/1", "0");
  Firebase.setString("/Histomed/Embedding/2", "193");
  Firebase.setString("/Histomed/Embedding/3", "197");
  Firebase.setString("/Histomed/Embedding/4", "180");
  Firebase.setString("/Histomed/Embedding/5", "195");
  Firebase.setString("/Histomed/Embedding/6", "198");
}

void SetPin() {
  pinMode(ev3clock, INPUT); //D1
  pinMode(datain, INPUT); //D2
  pinMode(dataout, OUTPUT); //D6
  pinMode(request, OUTPUT); //D5
  digitalWrite(request, LOW);
}

void ReadInput() {
  timing = digitalRead(ev3clock);
  data = digitalRead(datain);
  //Serial.print(timing); Serial.print("\t"); Serial.println(data);

  delay(10);
}

void ReadBinary() {
  sum = 0;
  while (timing != 1) {
    ReadInput();
  }
  while (timing != 0) {
    ReadInput();
  }

  for (int i = 7; i >= 0; i--) {
    while (timing != 1) {
      ReadInput();
    }

    sum = sum + (data * pow(2, i));

    if (data == 1) {
      digitalWrite(dataout, HIGH);
    }

    while (timing != 0) {
      ReadInput();
    }

    digitalWrite(dataout, LOW);
  }

  Serial.print("Sum = "); Serial.println(sum);


  //Firebase.setFloat("/Histomed/test",sum);
  //x=Firebase.getString("/Histomed/num").toInt();
  //Serial.println(x);
}

void Mode() {
  type = sum;
  switch (type) {
    case 1:
      Firebase.setString("/Histomed/HistoBot Status", "Running");
      break;

    case 2:
      Firebase.setString("/Histomed/ID database/100/Status" , "Formalin");
      Firebase.setString("/Histomed/ID database/105/Status" , "Formalin");
      Firebase.setString("/Histomed/ID database/117/Status" , "Formalin");
      Firebase.setString("/Histomed/ID database/124/Status" , "Formalin");
      break;

    case 3:
      Firebase.setString("/Histomed/ID database/133/Status" , "Clearing");
      Firebase.setString("/Histomed/ID database/145/Status" , "Clearing");
      Firebase.setString("/Histomed/ID database/151/Status" , "Clearing");  
      break;

    case 4:
      Firebase.setString("/Histomed/ID database/179/Status" , "Embedding");
      Firebase.setString("/Histomed/ID database/193/Status" , "Embedding");
      break;

    case 5:
      Firebase.setString("/Histomed/Notification" , "198");
      notified = 0;
      while (notified == 0) {
        notified = Firebase.getString("/Histomed/Sectioning").toInt();
      }
      digitalWrite(request, HIGH);
      Firebase.setString("/Histomed/Embedding/6", "198");
      Firebase.setString("/Histomed/Sectioning" , "0");
      break;

    case 6:
      digitalWrite(request, LOW);
      delay(1000);
      requestmode = true;
      break;

    default:
      break;
  }
}

void WaitRequest() {
  digitalWrite(request, LOW);
  int block = 0;
  done = 0;
  Firebase.setString("/Histomed/Busy", "0");

  while (block == 0) {
    block = Firebase.getString("/Histomed/Request").toInt();
  }
  WriteBinary(block);

  Firebase.setString("/Histomed/Request", "0");
  
  while (done == 0) {
    done = digitalRead(datain);
  }
}


void WriteBinary(int number) {
  number = number + 16;
  for (int i = 4; i >= 0; i--) {
    byte bitvalue = bitRead(number, i);
    digitalWrite(request, bitvalue);
    delay(100);
    digitalWrite(request, LOW);
    delay(200);
  }
}
