#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "**";  //  your network SSID (name)
char pass[] = "**";       // your network password

int get_NTPtime(){
																		// send NTP request, wait until NTPtmout, extract time
	#define NTPtmout 5000												//#include <WiFiUdp.h>
	#define timezone 9
	#define timezone_min 30
	#define NTP_PACKET_SIZE 48
	#define localPort 2390												// local port to listen for UDP packets

	if(WiFi.status() != WL_CONNECTED){
		Serial.println("Wifi is not connected!");
		return -1;
	}

	WiFiUDP udp;				
	IPAddress timeServerIP; 											// time.nist.gov NTP server address
	const char* ntpServerName = "time.nist.gov";
	byte packetBuffer[ NTP_PACKET_SIZE]; 								//buffer to hold incoming and outgoing packets

  	udp.begin(localPort);												//begin UDP
	WiFi.hostByName(ntpServerName, timeServerIP); 						//get a random server from the pool
 
	memset(packetBuffer, 0, NTP_PACKET_SIZE);							// create ntp request
	packetBuffer[0] = 0b11100011;										// LI, Version, Mode
	packetBuffer[1] = 0;												// Stratum, or type of clock
	packetBuffer[2] = 6;												// Polling Interval
	packetBuffer[3] = 0xEC;												// Peer Clock Precision
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	udp.beginPacket(timeServerIP, 123);									//NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);							// send NTP request
	udp.endPacket();
	delay(NTPtmout);													// wait for resp.

	int resp = udp.parsePacket();
	if (!resp) {
		Serial.println("NTP response was not recived!");
		return -1;
	}
	udp.read(packetBuffer, NTP_PACKET_SIZE); 							// read the packet into the buffer

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long epoch = (highWord << 16 | lowWord) - 2208988800UL;	// - 70 years

    unsigned int hours =(unsigned int)((epoch  % 86400L) / 3600);
    unsigned int minutes = (unsigned int)((epoch  % 3600) / 60); 		// print the minute (3600 equals secs per minute)
	unsigned int secs = (unsigned int)(epoch  % 60);

	hours += timezone;
	minutes = minutes + timezone_min;
	if(minutes > 60){
		minutes-=60;
		hours += 1;
	}
	if(hours >= 24){
		hours = hours % 24;
	}

	Serial.print("Time is "); 
	Serial.print(hours); 
	Serial.print(":"); 
	Serial.print(minutes); 
	Serial.print(":"); 
	Serial.println(secs); 

	return 0;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop()
{
	get_NTPtime();
}
