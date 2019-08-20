#include <nRF24L01.h>
#include <printf.h>
#include "RF24.h"
#include <SPI.h>

RF24 Radio (10,9);     //9,8											//create RF24 object

int Com(bool init=false, bool snd= false, char msg[32] = ""){
	#define _com_verbose
	#define _timeout_val 200000											// timeout in us
	#define _channel 100
    
    byte myaddress [6] = "base1";
    byte otheraddr [6] = "base2";

	char resp[32]  = "OK";
	bool timeout = false;       
    
    if(init){
		Radio.begin();
        if(!(Radio.isChipConnected())){
			Serial.println(F("Module not connected!"));
			return -2;
        }
		Radio.setAutoAck(false);										// disable ack
        Radio.setPALevel(RF24_PA_MIN);     								//set RF power output to minimum // test only!!!
     	Radio.setChannel(_channel);             						//set frequency to channel 
		if(Radio.setDataRate(RF24_250KBPS) == false)					//set datarate to 250kbps
			Serial.println(F("unable to set datarate!"));

        Radio.openWritingPipe(otheraddr);
        Radio.openReadingPipe(1,myaddress);         					// pipe no_1, listening to 'myaddress'
        Radio.startListening();
        #ifdef _com_verbose
        Serial.println(F("Init completed"));
        #endif
        return 0;
    }
	else if(!snd){
		if( Radio.available()){		
			char inmsg[32];								
			Radio.read(&inmsg, sizeof(inmsg));
			Radio.stopListening();                                          
			Radio.write( &resp, sizeof(resp));                
			Radio.startListening();                    
			#ifdef _com_verbose
			Serial.print(F("New msg->"));
			Serial.print(inmsg); 
			Serial.print(F("|Sent->"));
			Serial.println(resp);
			#endif  
        }
        #ifdef _com_verbose
		else
			Serial.println("NO msg");
        #endif
    }
    else if(snd){
		Radio.stopListening();  
		char outmsg[32] = "hello";
		Radio.write(&outmsg,sizeof(outmsg));  							//transmit 'msg'
		Radio.startListening();


		unsigned long start = micros();               					// start timer for timeout                    
		while(!Radio.available()){
			if(micros() - start > _timeout_val){            			
				timeout = true;
				break;
			}      
		}
        if(timeout){
        	Serial.println(F("NO ACK. timeout!"));
			return -1;
    	}
		else{
        	Radio.read( &resp, sizeof(resp));
    	}
		#ifdef _com_verbose
		Serial.print("Sent->");
		Serial.print(outmsg);
		Serial.print("<Resp:");
		Serial.print(resp);
		Serial.print(" -delay: ");
		Serial.print((unsigned long)(micros() - start));
		Serial.println(" us");
        #endif
		return 0;
	}
}

void setup() {
	Serial.begin(115200);
	Com(true);
}

void loop() {
	Com(false,false);
	//Com(false,true,"hello");
	delay(3000);      
}
