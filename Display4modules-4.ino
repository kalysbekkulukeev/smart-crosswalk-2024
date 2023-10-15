#include <SPI.h>
#include <DMD2.h>
#include "ChangedRus5x7.h"
//#include "Asyl32x16Font.h"
#include "Arial_Black_16.h"  

SoftDMD dmd(1,3);  // DMD controls the entire display
DMD_TextBox box1(dmd, 0, 0, 32, 16);  
DMD_TextBox box2(dmd, 0, 16, 32, 16); 
DMD_TextBox box3(dmd, 0, 33, 32, 16); 

int go_countdown   = 15;
int stand_still_countdown = 3;

unsigned long timer = 0;
unsigned long timer2 = 0;
int countdown = 0;
bool sys_active = false;
bool should_update = false;
bool should_update_walking_man = false;
bool stand_still = false;
unsigned long walking_man_frame = 0;


char text[10];
byte chPos = 0;
byte ch = 0;

ISR(TIMER2_COMPA_vect){  //This is the interrupt request

  if(sys_active){
    timer++;
    if (timer == 100) {    //for 100hz to make 1 sec
      should_update = true;
      timer = 0;             // RESETTING THE TIMER VALUE
    }
    timer2++;
    if ((!stand_still) && (timer2 == 12)) {    //for 100hz to make 120ms
      should_update_walking_man = true;
      timer2 = 0;             // RESETTING THE TIMER VALUE
    }    
  }
}

void setup() {
  Serial.begin(9600);

  // TIMER 0 for interrupt frequency 125 Hz:
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  TCCR2A = 1<<WGM21;    //Set the CTC mode 
  OCR2A = 0x9C; // 100hz
  TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); // Set CS02, CS01 and CS00 bits for 1024 prescaler
  //  TIMSK2 |= (1 << OCIE2A);// enable timer compare interrupt
  TIMSK2 &= ~(1 << OCIE2A);
  sei(); // allow interrupts
  
  dmd.setBrightness(170);
  dmd.selectFont(ChangedRus5x7);
  dmd.begin();
  clearAllScreen();

//  dmd.drawString(0, 0, String("QWERT"));
//  dmd.drawString(0, 8, String("YUIOP"));
//  dmd.drawString(0, 16, String("[]ASD"));
//  dmd.drawString(0, 32, String("FGHJK"));
//  dmd.drawString(0, 40, String("L;'ZX"));
//  dmd.drawString(0, 48, String("CVBNM"));
//  dmd.drawString(0, 56, String(",."));

  delay(100);
  
  dmd.drawString(4, 4, "CNJG");  //стоп
  box2.print(";LBNT YFXFKJ");    //ждите начало
  box3.print("GHJ[J-LBNT");      //прохо-дите
  Serial.println("Started!");

  sys_start_go();
}



void loop() {

  if (Serial.available()){
    while(Serial.available()) {
       //Serial.println("there is new data");
      //read incoming char by char:
       ch = Serial.read();
       text[chPos] = ch;
       chPos++;
       delay(10);  
    }
    text[chPos] = 0; //terminate cString

    Serial.println(text);
    if (chPos == 5) {
      //Serial.println(text);
      int x = text[0] - '0';
      if (x == 0) {
        sys_stand_still();
      } else if (x == 1) {
        int sfirst  = text[1] - '0';
        int ssecond = text[2] - '0';
        
        int gfirst  = text[3] - '0';
        int gsecond = text[4] - '0';

        stand_still_countdown = (sfirst * 10) + ssecond;
        go_countdown          = (gfirst * 10) + gsecond;

        sys_start_go();
        Serial.println(stand_still_countdown);
        Serial.println(go_countdown);
      }
    }
    
    chPos = 0;   // restarting the character position

  } 


  if (should_update) {
    update_screen();
    should_update = false;
  }
  if (should_update_walking_man) {
    update_screen_walking_man();
    should_update_walking_man = false;
  }
   
}

void sys_stand_still(){  // ждите старт, duration controlled by Mega
  
    Serial.println("sys stand still!");
    dmd.selectFont(ChangedRus5x7);
    dmd.drawString(1, 20, ";LBNT");  // ждите
    dmd.drawString(1, 36, "CNFHN");  // старт
    
}

void sys_start_go(){

    Serial.println("sys start go!"); 
    clearAllScreen();
    countdown = stand_still_countdown;
    stand_still = true; // show red countdown
    
    sys_active = true;
    timer = 97;          //value to start immediately
    TCNT2 = 0;            //reset timer
    TIMSK2 = (1<<OCIE0A); //start timer
  
}

void sys_stop(){
  TIMSK2 = 0;

  go_countdown          = 0;
  stand_still_countdown = 0;
  
  sys_active = false;
  clearAllScreen();
}

void update_screen(){

  dmd.selectFont(ChangedRus5x7);
  
  if (stand_still) {
    
    if (countdown == stand_still_countdown)
      Serial.println("updating the stop text!");
      dmd.drawString(4, 4, "CNJG");   // стоп

    if(countdown==0) {
      box1.clear(); walking_man_frame = 0; stand_still = false; countdown = go_countdown; }
      
  }
  
  if (!stand_still) {
    //if (countdown == go_countdown)
    //  Serial.println("updating the go text!");
      //box3.print("GHJ[J-LBNT");     //проходите

    if(countdown==0){
      sys_stop(); box3.clear(); return;}
    
  }

  box2.clear();
  //dmd.selectFont(Asyl32x16Font);
  dmd.selectFont(Arial_Black_16);

  if (countdown<10)
      dmd.drawString(12, 17, String(countdown));
    else
      dmd.drawString(7, 17, String(countdown));

  countdown--;
  
}

void update_screen_walking_man(){
  box1.clear();
  switch (walking_man_frame) {
  case 0:
    dmd.setPixel(12,4);dmd.setPixel(12,5);dmd.setPixel(12,6);dmd.setPixel(12,7);dmd.setPixel(12,8);	
    dmd.setPixel(13,4);dmd.setPixel(13,5);	
    dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,4);dmd.setPixel(14,5);dmd.setPixel(14,6);dmd.setPixel(14,7);dmd.setPixel(14,8);dmd.setPixel(14,9);dmd.setPixel(14,10);dmd.setPixel(14,11);dmd.setPixel(14,12);dmd.setPixel(14,13);dmd.setPixel(14,14);dmd.setPixel(14,15);	
    dmd.setPixel(15,0);dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,3);dmd.setPixel(15,4);dmd.setPixel(15,5);dmd.setPixel(15,6);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,9);dmd.setPixel(15,10);	
    dmd.setPixel(16,0);dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,3);dmd.setPixel(16,4);dmd.setPixel(16,5);dmd.setPixel(16,6);dmd.setPixel(16,7);dmd.setPixel(16,8);dmd.setPixel(16,9);dmd.setPixel(16,10);	
    dmd.setPixel(17,1);dmd.setPixel(17,2);dmd.setPixel(17,4);dmd.setPixel(17,5);dmd.setPixel(17,6);dmd.setPixel(17,7);dmd.setPixel(17,8);dmd.setPixel(17,9);dmd.setPixel(17,10);dmd.setPixel(17,11);dmd.setPixel(17,12);dmd.setPixel(17,13);dmd.setPixel(17,14);dmd.setPixel(17,15);	
    dmd.setPixel(18,4);dmd.setPixel(18,5);	
    dmd.setPixel(19,4);dmd.setPixel(19,5);dmd.setPixel(19,6);dmd.setPixel(19,7);dmd.setPixel(19,8);	

    dmd.setPixel(12,5);dmd.setPixel(12,6);dmd.setPixel(12,7);dmd.setPixel(12,8);	
    dmd.setPixel(13,4);dmd.setPixel(13,5);	
    dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,4);dmd.setPixel(14,5);dmd.setPixel(14,6);dmd.setPixel(14,7);dmd.setPixel(14,8);dmd.setPixel(14,9);dmd.setPixel(14,10);dmd.setPixel(14,11);dmd.setPixel(14,12);dmd.setPixel(14,13);dmd.setPixel(14,14);dmd.setPixel(14,15);	
    dmd.setPixel(15,0);dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,3);dmd.setPixel(15,4);dmd.setPixel(15,5);dmd.setPixel(15,6);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,9);dmd.setPixel(15,10);	
    dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,4);dmd.setPixel(16,5);dmd.setPixel(16,6);dmd.setPixel(16,7);dmd.setPixel(16,8);dmd.setPixel(16,9);dmd.setPixel(16,10);dmd.setPixel(16,11);dmd.setPixel(16,12);dmd.setPixel(16,13);dmd.setPixel(16,14);dmd.setPixel(16,15);	
    dmd.setPixel(17,4);dmd.setPixel(17,5);
    dmd.setPixel(18,5);dmd.setPixel(18,6);dmd.setPixel(18,7);dmd.setPixel(18,8);	

    dmd.setPixel(12,55); dmd.setPixel(12,56); dmd.setPixel(12,60);    dmd.setPixel(12,61);    dmd.setPixel(12,62);    dmd.setPixel(12,63);  
    dmd.setPixel(13,54); dmd.setPixel(13,55); dmd.setPixel(13,59);    dmd.setPixel(13,60);    dmd.setPixel(13,13+48);    dmd.setPixel(13,14+48);  dmd.setPixel(13,15+48);
    dmd.setPixel(14,1+48); dmd.setPixel(14,2+48); dmd.setPixel(14,5+48);   dmd.setPixel(14,6+48);   dmd.setPixel(14,10+48);    dmd.setPixel(14,11+48);  
    dmd.setPixel(15,48); dmd.setPixel(15,1+48); dmd.setPixel(15,2+48);   dmd.setPixel(15,3+48);   dmd.setPixel(15,4+48);   dmd.setPixel(15,5+48); dmd.setPixel(15,6+48);dmd.setPixel(15,7+48);dmd.setPixel(15,8+48);dmd.setPixel(15,9+48);dmd.setPixel(15,10+48);
    dmd.setPixel(16,0+48); dmd.setPixel(16,1+48); dmd.setPixel(16,2+48);   dmd.setPixel(16,3+48);   dmd.setPixel(16,4+48);   dmd.setPixel(16,5+48); dmd.setPixel(16,6+48);dmd.setPixel(16,7+48);dmd.setPixel(16,8+48);dmd.setPixel(16,9+48);dmd.setPixel(16,10+48);dmd.setPixel(16,11+48);
    dmd.setPixel(17,1+48); dmd.setPixel(17,2+48); dmd.setPixel(17,4+48);   dmd.setPixel(17,5+48);   dmd.setPixel(17,10+48);    dmd.setPixel(17,11+48);  dmd.setPixel(17,12+48);
    dmd.setPixel(18,5+48); dmd.setPixel(18,6+48); dmd.setPixel(18,12+48);    dmd.setPixel(18,13+48);    dmd.setPixel(18,14+48);    
    dmd.setPixel(19,6+48);  dmd.setPixel(19,7+48); dmd.setPixel(19,8+48);   dmd.setPixel(19,14+48);    dmd.setPixel(19,15+48);    
    break;
  case 1:
    dmd.setPixel(24,7);dmd.setPixel(24,8);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);dmd.setPixel(24,15);
    dmd.setPixel(25,1);dmd.setPixel(25,2);dmd.setPixel(25,6);dmd.setPixel(25,7);dmd.setPixel(25,11);dmd.setPixel(25,12);dmd.setPixel(25,13);dmd.setPixel(25,14);dmd.setPixel(25,15);
    dmd.setPixel(26,0);dmd.setPixel(26,1);dmd.setPixel(26,2);dmd.setPixel(26,3);dmd.setPixel(26,4);dmd.setPixel(26,5);dmd.setPixel(26,6);dmd.setPixel(26,7);dmd.setPixel(26,8);dmd.setPixel(26,9);dmd.setPixel(26,10);dmd.setPixel(26,11);
    dmd.setPixel(27,0);dmd.setPixel(27,1);dmd.setPixel(27,2);dmd.setPixel(27,3);dmd.setPixel(27,4);dmd.setPixel(27,5);dmd.setPixel(27,6);dmd.setPixel(27,7);dmd.setPixel(27,8);dmd.setPixel(27,9);dmd.setPixel(27,10);dmd.setPixel(27,11);
    dmd.setPixel(28,1);dmd.setPixel(28,2);dmd.setPixel(28,4);dmd.setPixel(28,5);dmd.setPixel(28,6);dmd.setPixel(28,10);dmd.setPixel(28,11);dmd.setPixel(28,12);dmd.setPixel(28,13);
    dmd.setPixel(29,6);dmd.setPixel(29,7);dmd.setPixel(29,8);dmd.setPixel(29,12);dmd.setPixel(29,13);dmd.setPixel(29,14);
    dmd.setPixel(30,13);dmd.setPixel(30,14);dmd.setPixel(30,15);
    break;
  case 2:
    dmd.setPixel(24,1);dmd.setPixel(24,2);dmd.setPixel(24,7);dmd.setPixel(24,8);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);dmd.setPixel(24,15);
    dmd.setPixel(25,0);dmd.setPixel(25,1);dmd.setPixel(25,2);dmd.setPixel(25,3);dmd.setPixel(25,4);dmd.setPixel(25,5);dmd.setPixel(25,6);dmd.setPixel(25,7);dmd.setPixel(25,8);dmd.setPixel(25,9);dmd.setPixel(25,10);dmd.setPixel(25,11);dmd.setPixel(25,12);dmd.setPixel(25,13);dmd.setPixel(25,14);dmd.setPixel(25,15);
    dmd.setPixel(26,0);dmd.setPixel(26,1);dmd.setPixel(26,2);dmd.setPixel(26,3);dmd.setPixel(26,4);dmd.setPixel(26,5);dmd.setPixel(26,6);dmd.setPixel(26,7);dmd.setPixel(26,8);dmd.setPixel(26,9);dmd.setPixel(26,10);dmd.setPixel(26,11);dmd.setPixel(26,12);
    dmd.setPixel(27,1);dmd.setPixel(27,2);dmd.setPixel(27,7);dmd.setPixel(27,8);dmd.setPixel(27,10);dmd.setPixel(27,11);dmd.setPixel(27,12);dmd.setPixel(27,13);
    dmd.setPixel(28,13);dmd.setPixel(28,14);
    dmd.setPixel(29,14);dmd.setPixel(29,15);
    break;
  case 3:   
		dmd.setPixel(23,1);dmd.setPixel(23,2);
    dmd.setPixel(24,0);dmd.setPixel(24,1);dmd.setPixel(24,2);dmd.setPixel(24,3);dmd.setPixel(24,4);dmd.setPixel(24,5);dmd.setPixel(24,6);dmd.setPixel(24,7);dmd.setPixel(24,8);dmd.setPixel(24,9);dmd.setPixel(24,10);dmd.setPixel(24,11);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);dmd.setPixel(24,15);
    dmd.setPixel(25,0);dmd.setPixel(25,1);dmd.setPixel(25,2);dmd.setPixel(25,3);dmd.setPixel(25,4);dmd.setPixel(25,5);dmd.setPixel(25,6);dmd.setPixel(25,7);dmd.setPixel(25,8);dmd.setPixel(25,9);dmd.setPixel(25,10);dmd.setPixel(25,11);dmd.setPixel(25,12);dmd.setPixel(25,13);dmd.setPixel(25,14);dmd.setPixel(25,15);
    dmd.setPixel(26,1);dmd.setPixel(26,2);dmd.setPixel(26,13);dmd.setPixel(26,14);
    break;
  case 4:
    dmd.setPixel(21,14);//dmd.setPixel(21,15);					
    dmd.setPixel(22,1);dmd.setPixel(22,2);dmd.setPixel(22,8);dmd.setPixel(22,9);dmd.setPixel(22,12);dmd.setPixel(22,13);dmd.setPixel(22,14);//dmd.setPixel(22,15);					
    dmd.setPixel(23,0);dmd.setPixel(23,1);dmd.setPixel(23,2);dmd.setPixel(23,3);dmd.setPixel(23,4);dmd.setPixel(23,5);dmd.setPixel(23,6);dmd.setPixel(23,7);dmd.setPixel(23,8);dmd.setPixel(23,9);dmd.setPixel(23,10);dmd.setPixel(23,11);dmd.setPixel(23,12);					
    dmd.setPixel(24,0);dmd.setPixel(24,1);dmd.setPixel(24,2);dmd.setPixel(24,3);dmd.setPixel(24,4);dmd.setPixel(24,5);dmd.setPixel(24,6);dmd.setPixel(24,7);dmd.setPixel(24,8);dmd.setPixel(24,9);dmd.setPixel(24,10);dmd.setPixel(24,11);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);dmd.setPixel(24,15);					
    dmd.setPixel(25,1);dmd.setPixel(25,2);dmd.setPixel(25,8);dmd.setPixel(25,9);dmd.setPixel(25,14);dmd.setPixel(25,15);					
    break;
  case 5:
    dmd.setPixel(19,7);dmd.setPixel(19,8);dmd.setPixel(19,12);dmd.setPixel(19,13);dmd.setPixel(19,14);//dmd.setPixel(19,15);
    dmd.setPixel(20,1);dmd.setPixel(20,2);dmd.setPixel(20,6);dmd.setPixel(20,7);dmd.setPixel(20,11);dmd.setPixel(20,12);dmd.setPixel(20,13);dmd.setPixel(20,14);//dmd.setPixel(20,15);
    dmd.setPixel(21,0);dmd.setPixel(21,1);dmd.setPixel(21,2);dmd.setPixel(21,3);dmd.setPixel(21,4);dmd.setPixel(21,5);dmd.setPixel(21,6);dmd.setPixel(21,7);dmd.setPixel(21,8);dmd.setPixel(21,9);dmd.setPixel(21,10);dmd.setPixel(21,11);
    dmd.setPixel(22,0);dmd.setPixel(22,1);dmd.setPixel(22,2);dmd.setPixel(22,3);dmd.setPixel(22,4);dmd.setPixel(22,5);dmd.setPixel(22,6);dmd.setPixel(22,7);dmd.setPixel(22,8);dmd.setPixel(22,9);dmd.setPixel(22,10);dmd.setPixel(22,11);
    dmd.setPixel(23,1);dmd.setPixel(23,2);dmd.setPixel(23,5);dmd.setPixel(23,6);dmd.setPixel(23,10);dmd.setPixel(23,11);dmd.setPixel(23,12);dmd.setPixel(23,13);
    dmd.setPixel(24,6);dmd.setPixel(24,7);dmd.setPixel(24,8);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);
    dmd.setPixel(25,13);dmd.setPixel(25,14);dmd.setPixel(25,15);
    break;
  case 6:
    //2-nd step
    dmd.setPixel(18,6);dmd.setPixel(18,7);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);
    dmd.setPixel(19,5);dmd.setPixel(19,6);dmd.setPixel(19,11);dmd.setPixel(19,12);dmd.setPixel(19,13);dmd.setPixel(19,14);dmd.setPixel(19,15);
    dmd.setPixel(20,1);dmd.setPixel(20,2);dmd.setPixel(20,4);dmd.setPixel(20,5);dmd.setPixel(20,10);dmd.setPixel(20,11);
    dmd.setPixel(21,0);dmd.setPixel(21,1);dmd.setPixel(21,2);dmd.setPixel(21,3);dmd.setPixel(21,4);dmd.setPixel(21,5);dmd.setPixel(21,6);dmd.setPixel(21,7);dmd.setPixel(21,8);dmd.setPixel(21,9);dmd.setPixel(21,10);
    dmd.setPixel(22,0);dmd.setPixel(22,1);dmd.setPixel(22,2);dmd.setPixel(22,3);dmd.setPixel(22,4);dmd.setPixel(22,5);dmd.setPixel(22,6);dmd.setPixel(22,7);dmd.setPixel(22,8);dmd.setPixel(22,9);dmd.setPixel(22,10);dmd.setPixel(22,11);
    dmd.setPixel(23,1);dmd.setPixel(23,2);dmd.setPixel(23,4);dmd.setPixel(23,5);dmd.setPixel(23,10);dmd.setPixel(23,11);dmd.setPixel(23,12);
 	  dmd.setPixel(24,5);dmd.setPixel(24,6);dmd.setPixel(24,12);dmd.setPixel(24,13);dmd.setPixel(24,14);
	  dmd.setPixel(25,6);dmd.setPixel(25,7);dmd.setPixel(25,8);dmd.setPixel(25,14);dmd.setPixel(25,15);
    break;
  case 7:
    dmd.setPixel(18,7);dmd.setPixel(18,8);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);
    dmd.setPixel(19,1);dmd.setPixel(19,2);dmd.setPixel(19,6);dmd.setPixel(19,7);dmd.setPixel(19,11);dmd.setPixel(19,12);dmd.setPixel(19,13);dmd.setPixel(19,14);dmd.setPixel(19,15);
    dmd.setPixel(20,0);dmd.setPixel(20,1);dmd.setPixel(20,2);dmd.setPixel(20,3);dmd.setPixel(20,4);dmd.setPixel(20,5);dmd.setPixel(20,6);dmd.setPixel(20,7);dmd.setPixel(20,8);dmd.setPixel(20,9);dmd.setPixel(20,10);dmd.setPixel(20,11);
    dmd.setPixel(21,0);dmd.setPixel(21,1);dmd.setPixel(21,2);dmd.setPixel(21,3);dmd.setPixel(21,4);dmd.setPixel(21,5);dmd.setPixel(21,6);dmd.setPixel(21,7);dmd.setPixel(21,8);dmd.setPixel(21,9);dmd.setPixel(21,10);dmd.setPixel(21,11);
    dmd.setPixel(22,1);dmd.setPixel(22,2);dmd.setPixel(22,4);dmd.setPixel(22,5);dmd.setPixel(22,6);dmd.setPixel(22,10);dmd.setPixel(22,11);dmd.setPixel(22,12);dmd.setPixel(22,13);
    dmd.setPixel(23,6);dmd.setPixel(23,7);dmd.setPixel(23,8);dmd.setPixel(23,12);dmd.setPixel(23,13);dmd.setPixel(23,14);
    dmd.setPixel(24,13);dmd.setPixel(24,14);dmd.setPixel(24,15);
    break;
  case 8:
    dmd.setPixel(18,1);dmd.setPixel(18,2);dmd.setPixel(18,7);dmd.setPixel(18,8);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);
    dmd.setPixel(19,0);dmd.setPixel(19,1);dmd.setPixel(19,2);dmd.setPixel(19,3);dmd.setPixel(19,4);dmd.setPixel(19,5);dmd.setPixel(19,6);dmd.setPixel(19,7);dmd.setPixel(19,8);dmd.setPixel(19,9);dmd.setPixel(19,10);dmd.setPixel(19,11);dmd.setPixel(19,12);dmd.setPixel(19,13);dmd.setPixel(19,14);dmd.setPixel(19,15);
    dmd.setPixel(20,0);dmd.setPixel(20,1);dmd.setPixel(20,2);dmd.setPixel(20,3);dmd.setPixel(20,4);dmd.setPixel(20,5);dmd.setPixel(20,6);dmd.setPixel(20,7);dmd.setPixel(20,8);dmd.setPixel(20,9);dmd.setPixel(20,10);dmd.setPixel(20,11);dmd.setPixel(20,12);
    dmd.setPixel(21,1);dmd.setPixel(21,2);dmd.setPixel(21,7);dmd.setPixel(21,8);dmd.setPixel(21,10);dmd.setPixel(21,11);dmd.setPixel(21,12);dmd.setPixel(21,13);
    dmd.setPixel(22,13);dmd.setPixel(22,14);
    dmd.setPixel(23,14);dmd.setPixel(23,15);
    break;
  case 9:
		dmd.setPixel(17,1);dmd.setPixel(17,2);
    dmd.setPixel(18,0);dmd.setPixel(18,1);dmd.setPixel(18,2);dmd.setPixel(18,3);dmd.setPixel(18,4);dmd.setPixel(18,5);dmd.setPixel(18,6);dmd.setPixel(18,7);dmd.setPixel(18,8);dmd.setPixel(18,9);dmd.setPixel(18,10);dmd.setPixel(18,11);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);
    dmd.setPixel(19,0);dmd.setPixel(19,1);dmd.setPixel(19,2);dmd.setPixel(19,3);dmd.setPixel(19,4);dmd.setPixel(19,5);dmd.setPixel(19,6);dmd.setPixel(19,7);dmd.setPixel(19,8);dmd.setPixel(19,9);dmd.setPixel(19,10);dmd.setPixel(19,11);dmd.setPixel(19,12);dmd.setPixel(19,13);dmd.setPixel(19,14);dmd.setPixel(19,15);
    dmd.setPixel(20,1);dmd.setPixel(20,2);dmd.setPixel(20,13);dmd.setPixel(20,14);
    break;
  case 10:
    dmd.setPixel(15,14);//dmd.setPixel(15,15);					
    dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,8);dmd.setPixel(16,9);dmd.setPixel(16,12);dmd.setPixel(16,13);dmd.setPixel(16,14);//dmd.setPixel(16,15);					
    dmd.setPixel(17,0);dmd.setPixel(17,1);dmd.setPixel(17,2);dmd.setPixel(17,3);dmd.setPixel(17,4);dmd.setPixel(17,5);dmd.setPixel(17,6);dmd.setPixel(17,7);dmd.setPixel(17,8);dmd.setPixel(17,9);dmd.setPixel(17,10);dmd.setPixel(17,11);dmd.setPixel(17,12);					
    dmd.setPixel(18,0);dmd.setPixel(18,1);dmd.setPixel(18,2);dmd.setPixel(18,3);dmd.setPixel(18,4);dmd.setPixel(18,5);dmd.setPixel(18,6);dmd.setPixel(18,7);dmd.setPixel(18,8);dmd.setPixel(18,9);dmd.setPixel(18,10);dmd.setPixel(18,11);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);					
    dmd.setPixel(19,1);dmd.setPixel(19,2);dmd.setPixel(19,8);dmd.setPixel(19,9);dmd.setPixel(19,14);dmd.setPixel(19,15);
    break;
  case 11:
    dmd.setPixel(13,7);dmd.setPixel(13,8);dmd.setPixel(13,12);dmd.setPixel(13,13);dmd.setPixel(13,14);//dmd.setPixel(13,15);
    dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,6);dmd.setPixel(14,7);dmd.setPixel(14,11);dmd.setPixel(14,12);dmd.setPixel(14,13);dmd.setPixel(14,14);//dmd.setPixel(14,15);
    dmd.setPixel(15,0);dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,3);dmd.setPixel(15,4);dmd.setPixel(15,5);dmd.setPixel(15,6);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,9);dmd.setPixel(15,10);dmd.setPixel(15,11);
    dmd.setPixel(16,0);dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,3);dmd.setPixel(16,4);dmd.setPixel(16,5);dmd.setPixel(16,6);dmd.setPixel(16,7);dmd.setPixel(16,8);dmd.setPixel(16,9);dmd.setPixel(16,10);dmd.setPixel(16,11);
    dmd.setPixel(17,1);dmd.setPixel(17,2);dmd.setPixel(17,5);dmd.setPixel(17,6);dmd.setPixel(17,10);dmd.setPixel(17,11);dmd.setPixel(17,12);dmd.setPixel(17,13);
    dmd.setPixel(18,6);dmd.setPixel(18,7);dmd.setPixel(18,8);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);
    dmd.setPixel(19,13);dmd.setPixel(19,14);dmd.setPixel(19,15);
    break;
  case 12:
    //3 STEP
    dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);
    dmd.setPixel(13,6);dmd.setPixel(13,7);dmd.setPixel(13,11);dmd.setPixel(13,12);dmd.setPixel(13,13);dmd.setPixel(13,14);dmd.setPixel(13,15);
    dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,5);dmd.setPixel(14,6);dmd.setPixel(14,10);dmd.setPixel(14,11);
    dmd.setPixel(15,0);dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,3);dmd.setPixel(15,4);dmd.setPixel(15,5);dmd.setPixel(15,6);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,9);dmd.setPixel(15,10);
    dmd.setPixel(16,0);dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,3);dmd.setPixel(16,4);dmd.setPixel(16,5);dmd.setPixel(16,6);dmd.setPixel(16,7);dmd.setPixel(16,8);dmd.setPixel(16,9);dmd.setPixel(16,10);dmd.setPixel(16,11);
    dmd.setPixel(17,1);dmd.setPixel(17,2);dmd.setPixel(17,4);dmd.setPixel(17,5);dmd.setPixel(17,10);dmd.setPixel(17,11);dmd.setPixel(17,12);
	  dmd.setPixel(18,5);dmd.setPixel(18,6);dmd.setPixel(18,12);dmd.setPixel(18,13);dmd.setPixel(18,14);
	  dmd.setPixel(19,6);dmd.setPixel(19,7);dmd.setPixel(19,8);dmd.setPixel(19,14);dmd.setPixel(19,15);
    break;
  case 13:
    dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);
    dmd.setPixel(13,1);dmd.setPixel(13,2);dmd.setPixel(13,6);dmd.setPixel(13,7);dmd.setPixel(13,11);dmd.setPixel(13,12);dmd.setPixel(13,13);dmd.setPixel(13,14);dmd.setPixel(13,15);
    dmd.setPixel(14,0);dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,3);dmd.setPixel(14,4);dmd.setPixel(14,5);dmd.setPixel(14,6);dmd.setPixel(14,7);dmd.setPixel(14,8);dmd.setPixel(14,9);dmd.setPixel(14,10);dmd.setPixel(14,11);
    dmd.setPixel(15,0);dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,3);dmd.setPixel(15,4);dmd.setPixel(15,5);dmd.setPixel(15,6);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,9);dmd.setPixel(15,10);dmd.setPixel(15,11);
    dmd.setPixel(16,1);dmd.setPixel(16,2);dmd.setPixel(16,4);dmd.setPixel(16,5);dmd.setPixel(16,6);dmd.setPixel(16,10);dmd.setPixel(16,11);dmd.setPixel(16,12);dmd.setPixel(16,13);
    dmd.setPixel(17,6);dmd.setPixel(17,7);dmd.setPixel(17,8);dmd.setPixel(17,12);dmd.setPixel(17,13);dmd.setPixel(17,14);
    dmd.setPixel(18,13);dmd.setPixel(18,14);dmd.setPixel(18,15);
    break;
  case 14:
    dmd.setPixel(12,1);dmd.setPixel(12,2);dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);
    dmd.setPixel(13,0);dmd.setPixel(13,1);dmd.setPixel(13,2);dmd.setPixel(13,3);dmd.setPixel(13,4);dmd.setPixel(13,5);dmd.setPixel(13,6);dmd.setPixel(13,7);dmd.setPixel(13,8);dmd.setPixel(13,9);dmd.setPixel(13,10);dmd.setPixel(13,11);dmd.setPixel(13,12);dmd.setPixel(13,13);dmd.setPixel(13,14);dmd.setPixel(13,15);
    dmd.setPixel(14,0);dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,3);dmd.setPixel(14,4);dmd.setPixel(14,5);dmd.setPixel(14,6);dmd.setPixel(14,7);dmd.setPixel(14,8);dmd.setPixel(14,9);dmd.setPixel(14,10);dmd.setPixel(14,11);dmd.setPixel(14,12);
    dmd.setPixel(15,1);dmd.setPixel(15,2);dmd.setPixel(15,7);dmd.setPixel(15,8);dmd.setPixel(15,10);dmd.setPixel(15,11);dmd.setPixel(15,12);dmd.setPixel(15,13);
    dmd.setPixel(16,13);dmd.setPixel(16,14);
    dmd.setPixel(17,14);dmd.setPixel(17,15);
    break;
  case 15:
		dmd.setPixel(11,1);dmd.setPixel(11,2);
    dmd.setPixel(12,0);dmd.setPixel(12,1);dmd.setPixel(12,2);dmd.setPixel(12,3);dmd.setPixel(12,4);dmd.setPixel(12,5);dmd.setPixel(12,6);dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,9);dmd.setPixel(12,10);dmd.setPixel(12,11);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);
    dmd.setPixel(13,0);dmd.setPixel(13,1);dmd.setPixel(13,2);dmd.setPixel(13,3);dmd.setPixel(13,4);dmd.setPixel(13,5);dmd.setPixel(13,6);dmd.setPixel(13,7);dmd.setPixel(13,8);dmd.setPixel(13,9);dmd.setPixel(13,10);dmd.setPixel(13,11);dmd.setPixel(13,12);dmd.setPixel(13,13);dmd.setPixel(13,14);dmd.setPixel(13,15);
    dmd.setPixel(14,1);dmd.setPixel(14,2);dmd.setPixel(14,13);dmd.setPixel(14,14);
    break;
  case 16:
    dmd.setPixel(9,14);//dmd.setPixel(9,15);					
    dmd.setPixel(10,1);dmd.setPixel(10,2);dmd.setPixel(10,8);dmd.setPixel(10,9);dmd.setPixel(10,12);dmd.setPixel(10,13);dmd.setPixel(10,14);//dmd.setPixel(10,15);					
    dmd.setPixel(11,0);dmd.setPixel(11,1);dmd.setPixel(11,2);dmd.setPixel(11,3);dmd.setPixel(11,4);dmd.setPixel(11,5);dmd.setPixel(11,6);dmd.setPixel(11,7);dmd.setPixel(11,8);dmd.setPixel(11,9);dmd.setPixel(11,10);dmd.setPixel(11,11);dmd.setPixel(11,12);					
    dmd.setPixel(12,0);dmd.setPixel(12,1);dmd.setPixel(12,2);dmd.setPixel(12,3);dmd.setPixel(12,4);dmd.setPixel(12,5);dmd.setPixel(12,6);dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,9);dmd.setPixel(12,10);dmd.setPixel(12,11);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);					
    dmd.setPixel(13,1);dmd.setPixel(13,2);dmd.setPixel(13,8);dmd.setPixel(13,9);dmd.setPixel(13,14);dmd.setPixel(13,15);
    break;
  case 17:
    dmd.setPixel(7,7);dmd.setPixel(7,8);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);//dmd.setPixel(7,15);
    dmd.setPixel(8,1);dmd.setPixel(8,2);dmd.setPixel(8,6);dmd.setPixel(8,7);dmd.setPixel(8,11);dmd.setPixel(8,12);dmd.setPixel(8,13);dmd.setPixel(8,14);//dmd.setPixel(8,15);
    dmd.setPixel(9,0);dmd.setPixel(9,1);dmd.setPixel(9,2);dmd.setPixel(9,3);dmd.setPixel(9,4);dmd.setPixel(9,5);dmd.setPixel(9,6);dmd.setPixel(9,7);dmd.setPixel(9,8);dmd.setPixel(9,9);dmd.setPixel(9,10);dmd.setPixel(9,11);
    dmd.setPixel(10,0);dmd.setPixel(10,1);dmd.setPixel(10,2);dmd.setPixel(10,3);dmd.setPixel(10,4);dmd.setPixel(10,5);dmd.setPixel(10,6);dmd.setPixel(10,7);dmd.setPixel(10,8);dmd.setPixel(10,9);dmd.setPixel(10,10);dmd.setPixel(10,11);
    dmd.setPixel(11,1);dmd.setPixel(11,2);dmd.setPixel(11,5);dmd.setPixel(11,6);dmd.setPixel(11,10);dmd.setPixel(11,11);dmd.setPixel(11,12);dmd.setPixel(11,13);
    dmd.setPixel(12,6);dmd.setPixel(12,7);dmd.setPixel(12,8);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);
    dmd.setPixel(13,13);dmd.setPixel(13,14);dmd.setPixel(13,15);
    break;
  case 18:
    //4 STEP
    dmd.setPixel(6,7);dmd.setPixel(6,8);dmd.setPixel(6,12);dmd.setPixel(6,13);dmd.setPixel(6,14);dmd.setPixel(6,15);
    dmd.setPixel(7,6);dmd.setPixel(7,7);dmd.setPixel(7,11);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);dmd.setPixel(7,15);
    dmd.setPixel(8,1);dmd.setPixel(8,2);dmd.setPixel(8,5);dmd.setPixel(8,6);dmd.setPixel(8,10);dmd.setPixel(8,11);
    dmd.setPixel(9,0);dmd.setPixel(9,1);dmd.setPixel(9,2);dmd.setPixel(9,3);dmd.setPixel(9,4);dmd.setPixel(9,5);dmd.setPixel(9,6);dmd.setPixel(9,7);dmd.setPixel(9,8);dmd.setPixel(9,9);dmd.setPixel(9,10);
    dmd.setPixel(10,0);dmd.setPixel(10,1);dmd.setPixel(10,2);dmd.setPixel(10,3);dmd.setPixel(10,4);dmd.setPixel(10,5);dmd.setPixel(10,6);dmd.setPixel(10,7);dmd.setPixel(10,8);dmd.setPixel(10,9);dmd.setPixel(10,10);dmd.setPixel(10,11);
    dmd.setPixel(11,1);dmd.setPixel(11,2);dmd.setPixel(11,4);dmd.setPixel(11,5);dmd.setPixel(11,10);dmd.setPixel(11,11);dmd.setPixel(11,12);
    dmd.setPixel(12,5);dmd.setPixel(12,6);dmd.setPixel(12,12);dmd.setPixel(12,13);dmd.setPixel(12,14);
    dmd.setPixel(13,6);dmd.setPixel(13,7);dmd.setPixel(13,8);dmd.setPixel(13,14);dmd.setPixel(13,15);
    break;
  case 19:
    dmd.setPixel(6,7);dmd.setPixel(6,8);dmd.setPixel(6,12);dmd.setPixel(6,13);dmd.setPixel(6,14);dmd.setPixel(6,15);
    dmd.setPixel(7,1);dmd.setPixel(7,2);dmd.setPixel(7,6);dmd.setPixel(7,7);dmd.setPixel(7,11);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);dmd.setPixel(7,15);
    dmd.setPixel(8,0);dmd.setPixel(8,1);dmd.setPixel(8,2);dmd.setPixel(8,3);dmd.setPixel(8,4);dmd.setPixel(8,5);dmd.setPixel(8,6);dmd.setPixel(8,7);dmd.setPixel(8,8);dmd.setPixel(8,9);dmd.setPixel(8,10);dmd.setPixel(8,11);
    dmd.setPixel(9,0);dmd.setPixel(9,1);dmd.setPixel(9,2);dmd.setPixel(9,3);dmd.setPixel(9,4);dmd.setPixel(9,5);dmd.setPixel(9,6);dmd.setPixel(9,7);dmd.setPixel(9,8);dmd.setPixel(9,9);dmd.setPixel(9,10);dmd.setPixel(9,11);
    dmd.setPixel(10,1);dmd.setPixel(10,2);dmd.setPixel(10,4);dmd.setPixel(10,5);dmd.setPixel(10,6);dmd.setPixel(10,10);dmd.setPixel(10,11);dmd.setPixel(10,12);dmd.setPixel(10,13);
    dmd.setPixel(11,6);dmd.setPixel(11,7);dmd.setPixel(11,8);dmd.setPixel(11,12);dmd.setPixel(11,13);dmd.setPixel(11,14);
    dmd.setPixel(12,13);dmd.setPixel(12,14);dmd.setPixel(12,15);
    break;
  case 20:
    dmd.setPixel(6,1);dmd.setPixel(6,2);dmd.setPixel(6,7);dmd.setPixel(6,8);dmd.setPixel(6,12);dmd.setPixel(6,13);dmd.setPixel(6,14);dmd.setPixel(6,15);
    dmd.setPixel(7,0);dmd.setPixel(7,1);dmd.setPixel(7,2);dmd.setPixel(7,3);dmd.setPixel(7,4);dmd.setPixel(7,5);dmd.setPixel(7,6);dmd.setPixel(7,7);dmd.setPixel(7,8);dmd.setPixel(7,9);dmd.setPixel(7,10);dmd.setPixel(7,11);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);dmd.setPixel(7,15);
    dmd.setPixel(8,0);dmd.setPixel(8,1);dmd.setPixel(8,2);dmd.setPixel(8,3);dmd.setPixel(8,4);dmd.setPixel(8,5);dmd.setPixel(8,6);dmd.setPixel(8,7);dmd.setPixel(8,8);dmd.setPixel(8,9);dmd.setPixel(8,10);dmd.setPixel(8,11);dmd.setPixel(8,12);
    dmd.setPixel(9,1);dmd.setPixel(9,2);dmd.setPixel(9,7);dmd.setPixel(9,8);dmd.setPixel(9,10);dmd.setPixel(9,11);dmd.setPixel(9,12);dmd.setPixel(9,13);
    dmd.setPixel(10,13);dmd.setPixel(10,14);
    dmd.setPixel(11,14);dmd.setPixel(11,15);
    break;
  case 21:
		dmd.setPixel(5,1);dmd.setPixel(5,2);
    dmd.setPixel(6,0);dmd.setPixel(6,1);dmd.setPixel(6,2);dmd.setPixel(6,3);dmd.setPixel(6,4);dmd.setPixel(6,5);dmd.setPixel(6,6);dmd.setPixel(6,7);dmd.setPixel(6,8);dmd.setPixel(6,9);dmd.setPixel(6,10);dmd.setPixel(6,11);dmd.setPixel(6,12);dmd.setPixel(6,13);dmd.setPixel(6,14);dmd.setPixel(6,15);
    dmd.setPixel(7,0);dmd.setPixel(7,1);dmd.setPixel(7,2);dmd.setPixel(7,3);dmd.setPixel(7,4);dmd.setPixel(7,5);dmd.setPixel(7,6);dmd.setPixel(7,7);dmd.setPixel(7,8);dmd.setPixel(7,9);dmd.setPixel(7,10);dmd.setPixel(7,11);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);dmd.setPixel(7,15);
    dmd.setPixel(8,1);dmd.setPixel(8,2);dmd.setPixel(8,13);dmd.setPixel(8,14);
    break;
  case 22:
    dmd.setPixel(3,14);//dmd.setPixel(3,15);					
    dmd.setPixel(4,1);dmd.setPixel(4,2);dmd.setPixel(4,8);dmd.setPixel(4,9);dmd.setPixel(4,12);dmd.setPixel(4,13);dmd.setPixel(4,14);//dmd.setPixel(4,15);					
    dmd.setPixel(5,0);dmd.setPixel(5,1);dmd.setPixel(5,2);dmd.setPixel(5,3);dmd.setPixel(5,4);dmd.setPixel(5,5);dmd.setPixel(5,6);dmd.setPixel(5,7);dmd.setPixel(5,8);dmd.setPixel(5,9);dmd.setPixel(5,10);dmd.setPixel(5,11);dmd.setPixel(5,12);					
    dmd.setPixel(6,0);dmd.setPixel(6,1);dmd.setPixel(6,2);dmd.setPixel(6,3);dmd.setPixel(6,4);dmd.setPixel(6,5);dmd.setPixel(6,6);dmd.setPixel(6,7);dmd.setPixel(6,8);dmd.setPixel(6,9);dmd.setPixel(6,10);dmd.setPixel(6,11);dmd.setPixel(6,12);dmd.setPixel(6,13);dmd.setPixel(6,14);dmd.setPixel(6,15);					
    dmd.setPixel(7,1);dmd.setPixel(7,2);dmd.setPixel(7,8);dmd.setPixel(7,9);dmd.setPixel(7,14);dmd.setPixel(7,15);					
    break;
  case 23:
    dmd.setPixel(2,7);dmd.setPixel(2,8);dmd.setPixel(2,12);dmd.setPixel(2,13);dmd.setPixel(2,14);//dmd.setPixel(2,15);
    dmd.setPixel(3,1);dmd.setPixel(3,2);dmd.setPixel(3,6);dmd.setPixel(3,7);dmd.setPixel(3,11);dmd.setPixel(3,12);dmd.setPixel(3,13);dmd.setPixel(3,14);//dmd.setPixel(3,15);
    dmd.setPixel(4,0);dmd.setPixel(4,1);dmd.setPixel(4,2);dmd.setPixel(4,3);dmd.setPixel(4,4);dmd.setPixel(4,5);dmd.setPixel(4,6);dmd.setPixel(4,7);dmd.setPixel(4,8);dmd.setPixel(4,9);dmd.setPixel(4,10);dmd.setPixel(4,11);
    dmd.setPixel(5,0);dmd.setPixel(5,1);dmd.setPixel(5,2);dmd.setPixel(5,3);dmd.setPixel(5,4);dmd.setPixel(5,5);dmd.setPixel(5,6);dmd.setPixel(5,7);dmd.setPixel(5,8);dmd.setPixel(5,9);dmd.setPixel(5,10);dmd.setPixel(5,11);
    dmd.setPixel(6,1);dmd.setPixel(6,2);dmd.setPixel(6,5);dmd.setPixel(6,6);dmd.setPixel(6,10);dmd.setPixel(6,11);dmd.setPixel(6,12);dmd.setPixel(6,13);
    dmd.setPixel(7,6);dmd.setPixel(7,7);dmd.setPixel(7,8);dmd.setPixel(7,12);dmd.setPixel(7,13);dmd.setPixel(7,14);
    dmd.setPixel(8,13);dmd.setPixel(8,14);dmd.setPixel(8,15);
    break;
  }
  walking_man_frame++;
  if (walking_man_frame >= 24)
    walking_man_frame = 0;
}

void clearAllScreen(){
  box1.clear();
  box2.clear();
  box3.clear();
}
