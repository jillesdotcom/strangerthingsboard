#include <M5Stack.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <Adafruit_NeoPixel.h>

#define DNS_PORT    53
#define LED_PIN     21
#define NUM_OF_LEDS 49

boolean debug = true;

uint32_t  led_color[NUM_OF_LEDS+10];

Adafruit_NeoPixel strip(NUM_OF_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

String     characters = "@:.0987654321     NOPQRSTUVWXYZ    MLKJIHGFEDCBA ";
IPAddress LocalIP(  8,   8,   4,   4);
IPAddress Gateway(  8,   8,   4,   4); 
IPAddress  Subnet(255, 255, 255,   0); 

DNSServer  dnsServer;
WiFiServer server(80);

char ssid_name[] = "StrangerThings";
int nr_of_passphrases = 21;
char passphrases[][32] = { 
  "Joyce Byers", 
  "Jim Hopper", 
  "Mike Wheeler", 
  "Jane Hopper", 
  "Dustin Henderson", 
  "Lucas Sinclair", 
  "Nancy Wheeler", 
  "Jonathan Byers", 
  "Karen Wheeler", 
  "Martin Brenner", 
  "Will Byers", 
  "Steve Harrington", 
  "Max Mayfield", 
  "Billy Hargrove", 
  "Bob Newby", 
  "Sam Owens", 
  "Robin Buckley", 
  "Erica Sinclair", 
  "Murray Bauman", 
  "Henry Creel", 
  "Eddie Munson"};

byte randomnumber = random( nr_of_passphrases );

char qrcode[128];

const char responseHTML[] PROGMEM = ""
  "<!DOCTYPE html>"
  "<head>"
  "<META HTTP-EQUIV=CACHE-CONTROL CONTENT=NO-CACHE><title>CaptivePortal</title>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style>"
  "input[type=submit]{"
    "font-family: Roboto, sans-serif;"
    "font-weight: 0;"
    "font-size: 12px;"
    "color: #000000;"
    "background-color: #ffff88;"
    "padding: 0px 0px;"
    "border: solid #ffff88 4px;"
    "box-shadow: rgb(0, 0, 0) 0px 0px 0px 0px;"
    "border-radius: 5px;"
    "transition: 0ms;"
    "transform: translateY(0);"
    "display: inline;"
    "flex-direction: row;"
    "align-items: center;"
    "cursor: pointer;"
  "}"
  "</style>"
  "</head>"
  "<body bgcolor=#000000 link=yellow>"
  "<font color=#ffff88 face=Arial,Helvetica><center>"
  "<form action=\"/blink\" method=\"GET\">"
  "<br/><img src=data:image/jpeg;base64,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><br/>"
  "<br/><h1>Welcome to<br/>The Upside Down</h1><p>What do you want to signal Joyce?<br/>"
  "<input type=text name=text size=32 maxlength=80> <input type=SUBMIT value=Blink><br/><br/>"
  "<input name=text type=SUBMIT value=\"Help me\"> "
  "<input name=text type=SUBMIT value=Run> "
  "<input name=text type=SUBMIT value=\"Right here\"> "
  "<input name=text type=SUBMIT value=Yes> "
  "<input name=text type=SUBMIT value=No> "
  "<input name=text type=SUBMIT value=Maybe><br/><br/>"
  "<input name=patern type=SUBMIT value=Rainbow> "
  "<input name=patern type=SUBMIT value=Rainbowcycle> "
  "<input name=patern type=SUBMIT value=Chase> "
  "<input name=patern type=SUBMIT value=Lightning><br/><br/>"
  "<input name=flag type=SUBMIT value=NL> "
  "<input name=flag type=SUBMIT value=BE> "
  "<input name=flag type=SUBMIT value=DE> "
  "<input name=flag type=SUBMIT value=FR> "
  "<input name=flag type=SUBMIT value=Bi> "
  "<input name=flag type=SUBMIT value=Gay> "
  "<input name=text type=SUBMIT value=Rick>"
  "</form></p></center></font></body></html>";

void setup() {
  randomSeed(analogRead(0));  
  delay(1000);
  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();  
  M5.lcd.setRotation(90);
  M5.Lcd.clear(WHITE);
  M5.Lcd.setTextSize(2);
  sprintf(qrcode,"WIFI:S:%s;T:WPA;P:%s;;",ssid_name,passphrases[randomnumber]);
  M5.Lcd.qrcode(qrcode, 0, 0, 240, 6);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.drawCentreString("Scan/Connect to:",120,225,2);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.drawCentreString(ssid_name,120,255,2);
  M5.Lcd.drawCentreString(passphrases[randomnumber],120,280,2);
  Serial.println("Created QR Code");
  Serial.print("SSID: ");
  Serial.println("ssid_name");
  Serial.print("PSK: ");
  Serial.println("ssid_passphrase");
  Serial.println("");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  setupxmascolors();
  xmas();
  Serial.println("Setup LEDs");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_name,passphrases[randomnumber]);
  WiFi.softAPConfig(LocalIP, Gateway, Subnet);
  Serial.println("Setup WiFi");
  
  dnsServer.start(DNS_PORT, "*", Gateway);
  Serial.println("Setup Captive Portal");

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
      
        if (c != '\r' && c != '\n' ) {
          currentLine += c;
        } else {
          if (c == '\n') {
            // Serial.println(currentLine);
            if (currentLine.startsWith("GET /blink")) {
              currentLine.replace(" HTTP/1.1","");
              currentLine.replace("text=&","");
              currentLine.replace("+"," ");
              if( currentLine.endsWith("patern=Rainbow")      ) { rainbow(10); xmas();}
              if( currentLine.endsWith("patern=Rainbowcycle") ) { theaterChaseRainbow(10); xmas();}
              if( currentLine.endsWith("patern=Lightning")    ) { lightning(); xmas();}
              if( currentLine.endsWith("patern=Chase")        ) { chase(50); xmas();}

              if( currentLine.endsWith("flag=NL")             ) { flag("NL");  delay(2000); xmas();}
              if( currentLine.endsWith("flag=BE")             ) { flag("BE");  delay(2000); xmas();}
              if( currentLine.endsWith("flag=DE")             ) { flag("DE");  delay(2000); xmas();}
              if( currentLine.endsWith("flag=FR")             ) { flag("FR");  delay(2000); xmas();}
              if( currentLine.endsWith("flag=Bi")             ) { flag("Bi");  delay(2000); xmas();}
              if( currentLine.endsWith("flag=Gay")            ) { flag("Gay"); delay(2000); xmas();}
              if( currentLine.startsWith("GET /blink?text=")  ) { 
                currentLine.replace("GET /blink?text=","");
                currentLine.toUpperCase();                
                blink();
                darkness();
                if(currentLine="RICK") {
                  writeMessage("NEVER GONNA GIVE YOU UP. NEVER GONNA LET YOU DOWN. NEVER GONNA RUN AROUND AND DESERT YOU. NEVER GONNA MAKE YOU CRY. NEVER GONNA SAY GOODBYE. NEVER GONNA TELL A LIE AND HURT YOU.");
                } else {
                  writeMessage(currentLine);
                }
                xmas();
              }

              Serial.println("Blink");
              Serial.println(currentLine);
            }
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.print(responseHTML);
              Serial.println("Captive Portal");
              break;
            }
            currentLine = "";
          }
        }
      }
    }
    client.stop();
  } else {}
}

/****************************************** OUIJA Messages ******************************************/

void writeMessage(String msg){
  darkness();
  boolean rcc = true;
  msg.toUpperCase();
  Serial.println(msg);
  int dot;
  
  delay(1000);
  for(int pos=0;pos<msg.length();pos++){
    darkness();
    delay(100);
    if (debug) Serial.print("pos: "); 
    if (debug) Serial.println(pos);
    char character=msg.charAt(pos);
    if (character==32) {
      delay(500);       
      if (debug) Serial.println("WAIT");
    } else {
      if (debug) Serial.print("character: "); 
      if (debug) Serial.print(character);
      dot = characters.indexOf(character);
      if(dot>=0) {
        if (debug) Serial.print(" -> "); 
        if (debug) Serial.println(dot);
        // strip.setPixelColor(dot, strip.Color(128,128,  0)); else strip.setPixelColor(dot, led_color[dot]);
        strip.setPixelColor(dot, led_color[dot]);
        strip.show();         
        delay(200);
      }
    }
    delay(500); 
  }
  delay(500);
  xmas();
}

void setupxmascolors() {
  for(int i=0;i<=NUM_OF_LEDS;i++){
    led_color[i++]=strip.Color(128,  0,  0); // green
    led_color[i++]=strip.Color(128,  0,128); // cyan
    led_color[i++]=strip.Color(128,128,  0); // yellow
    led_color[i++]=strip.Color(  0,128,128); // pink
    led_color[i++]=strip.Color(  0,  0,128); // blue
    led_color[i++]=strip.Color(  0,128,  0); // red
    led_color[i  ]=strip.Color(128,128,128); // white
  }
}

void xmas() {
  for(int i=0;i<=NUM_OF_LEDS;i++){ strip.setPixelColor(i, led_color[i]); }
  strip.show();
}

void blink() {
  for(int i=10;i>1;i-=2) {
    xmas();
    delay(i*10);
    darkness();
    delay(i*10);
  }
}

void brightlight() {
  for(int i=0; i<strip.numPixels(); i++) { strip.setPixelColor(i, strip.Color( 255,255,255)); }
  strip.show();
}

void darkness() {
  for(int i=0; i<strip.numPixels(); i++) { strip.setPixelColor(i, 0); }
  strip.show();
}

/****************************************** Fancy extra's  ******************************************/


void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void chase(uint8_t wait) {
  darkness();
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(strip.numPixels()-i,  led_color[i]);
    strip.show();
    delay(wait);
    strip.setPixelColor(strip.numPixels()-i,  0);
    strip.show();
  }
}

void lightning() {
  for(int i=0; i<random(3,8); i++) { 
    brightlight();
    delay(random(4,10));
    darkness();
    delay(50+random(100));
  }
}


void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) { strip.setPixelColor(i+q, Wheel( (i+j) % 255)); }
      strip.show();
      delay(wait);
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) { strip.setPixelColor(i+q, 0); }
    }
  }
}

void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) { strip.setPixelColor(i+q, c); }
      strip.show();
      delay(wait);
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) { strip.setPixelColor(i+q, 0); }
    }
  }
}


void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) { strip.setPixelColor(i, Wheel((i+j) & 255)); }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) { return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3); }
  if(WheelPos < 170) { WheelPos -= 85;    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3); }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void flag( String country ) {                 //  green red blue
  darkness();
  
  if( country == "Gay" ) {
    for(int i=48;i>34;i--){
      if( i>=41 && i<=48 ) { strip.setPixelColor(i, strip.Color(   0,255,  0)); }
      if( i>=34 && i<=40 ) { strip.setPixelColor(i, strip.Color( 140,255,  0)); }
    }
    for(int i=17;i<32;i++){
      if( i>=17 && i<=23 ) { strip.setPixelColor(i, strip.Color(  215,155,  0));   }
      if( i>=24 && i<=32 ) { strip.setPixelColor(i, strip.Color(  255,  0,  0)); }
    }
    for(int i=12;i>=0;i--){
      if( i>=7 && i<=12 ) { strip.setPixelColor(i, strip.Color(  0,0, 255));  }
      if( i>=0 && i<=6 ) { strip.setPixelColor(i, strip.Color(  0,148,211)); }
    }
  }
  
  if( country == "BE" ) {
    for(int i=48;i>34;i--){
      if( i>=43 && i<=48 ) { strip.setPixelColor(i, strip.Color(  0,0,  0));   }
      if( i>=39 && i<=42 ) { strip.setPixelColor(i, strip.Color(128,128,  0)); }
      if( i>=34 && i<=38 ) { strip.setPixelColor(i, strip.Color(  0,128,  0)); }
    }
    for(int i=17;i<32;i++){
      if( i>=17 && i<=21 ) { strip.setPixelColor(i, strip.Color(  0,0,  0));   }
      if( i>=22 && i<=25 ) { strip.setPixelColor(i, strip.Color(128,128,  0));  }
      if( i>=26 && i<=32 ) { strip.setPixelColor(i, strip.Color(  0,128,  0)); }
    }
    for(int i=12;i>=0;i--){
      if( i>=9 && i<=12 ) { strip.setPixelColor(i, strip.Color(  0,0,  0));  }
      if( i>=5 && i<=8 ) { strip.setPixelColor(i, strip.Color(128,128,  0));  }
      if( i>=0 && i<=4 ) { strip.setPixelColor(i, strip.Color(  0,128,  0)); }
    }
  }
  
  if( country == "FR" ) {
    for(int i=48;i>34;i--){
      if( i>=43 && i<=48 ) { strip.setPixelColor(i, strip.Color(  0,0,  128)); }
      if( i>=39 && i<=42 ) { strip.setPixelColor(i, strip.Color(128,128,128)); }
      if( i>=34 && i<=38 ) { strip.setPixelColor(i, strip.Color(  0,128,  0)); }
    }
    for(int i=17;i<32;i++){
      if( i>=17 && i<=21 ) { strip.setPixelColor(i, strip.Color(  0,0,  128));  }
      if( i>=22 && i<=25 ) { strip.setPixelColor(i, strip.Color(128,128,128));  }
      if( i>=26 && i<=32 ) { strip.setPixelColor(i, strip.Color(  0,128,  0)); }
    }
    for(int i=12;i>=0;i--){
      if( i>=9 && i<=12 ) { strip.setPixelColor(i, strip.Color(  0,0,  128));  }
      if( i>=5 && i<=8 ) { strip.setPixelColor(i, strip.Color(128,128, 128));  }
      if( i>=0 && i<=4 ) { strip.setPixelColor(i, strip.Color(  0,128,  0));   }
    }
  }
  
  if( country == "NL" ) {
    for(int i=48;i>34;i--){
      strip.setPixelColor(i, strip.Color(  0,128,  0)); // red
    }
    for(int i=17;i<32;i++){
      strip.setPixelColor(i, strip.Color(128,128,128)); // white
    }
    for(int i=12;i>=0;i--){
      strip.setPixelColor(i, strip.Color(  0,  0,128)); // blue
    }
  }
  
  if( country == "DE" ) {
    for(int i=17;i<32;i++){
      strip.setPixelColor(i, strip.Color(  0,128,  0)); // red
    }
    for(int i=12;i>=0;i--){
      strip.setPixelColor(i, strip.Color(128,128,  0)); // yellow
    }
  }
  
  if( country == "Bi" ) {
    for(int i=48;i>34;i--){
      strip.setPixelColor(i, strip.Color(  0,128,128)); // pink
    }
    for(int i=17;i<32;i++){
      strip.setPixelColor(i, strip.Color(0, 32, 32)); // purple
    }
    for(int i=12;i>=0;i--){
      strip.setPixelColor(i, strip.Color(  0,  0,128)); // blue
    }
  }
  
  strip.show();
  
}
