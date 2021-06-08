#include <Neurona.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
bool if_a,if_b,if_c,if_d,if_e=false;                // lcd pins
LiquidCrystal_I2C lcd(0x27, 16, 2);
int redled=11;
int greenled=12;
int blueled=13;

int value = A0;
int red;
int blue;
int green;

int redvalue;
int bluevalue;
int greenvalue;

int redout=A1;
int blueout=A2;
int greenout=A3;


#define LDR_PIN 0
#define THRESHOLD 2000
#define LED_DELAY 500

#define RECOGNIZE 1
#define CALIBRATE 2
#define CALIBRATE_B 3
#define CALIBRATE_W 4

#define NET_INPUTS 3
#define NET_OUTPUTS 10
#define NET_LAYERS 2


const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6,7,8,9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int lastState=0, op=0;
unsigned long pressTime = 0;

int rgb[] = {11,12,13}; //led pins
int input[] = {0, 0, 0}; //RGB values
double netInput[] = {-1.0, 0.0, 0.0, 0.0};
int calib[3][2] = {{329, 778}, {166, 569}, {140, 528}};

char *colors[] = {"BLACK", "RED", "GREEN", "BLUE", "YELLOW", "BROWN", "PURPLE", "ORANGE", "PINK", "WHITE"};

int layerSizes[] = {6, NET_OUTPUTS, -1};
int offset=0, iOffset=0, yOffset=0;
double PROGMEM const initW[] = {2.753086,-11.472257,-3.311738,16.481226,19.507006,20.831778,7.113330,-6.423491,1.907215,6.495393,-27.712126,26.228203,-0.206367,-5.724560,-22.278070,30.065610,6.139262,-10.814282,28.513130,-9.784946,6.467021,0.055005,3.730361,4.145092,2.479019,0.013003,-3.582416,-16.364391,14.133357,-5.089288,1.637492,5.894826,1.415764,-3.315533,14.814289,-20.906571,-1.568656,1.917658,4.910184,4.039419,-10.848469,-5.641680,-4.132432,10.711442,3.759935,19.507702,17.728724,-3.210244,-2.476992,8.988450,5.196827,2.636043,17.357207,2.005429,11.713386,-5.453253,-6.940325,10.752005,0.666605,-7.266082,-3.587120,-9.921817,-12.682059,-15.456143,-13.740927,0.508265,15.179410,-11.143178,-19.085120,1.251235,22.006491,-4.227328,-0.444516,3.589025,0.649661,13.675598,-13.026884,-11.229070,-15.300703,-1.718191,6.737973,-28.176802,-2.505471,5.197970,7.007983,-2.869269,3.650349,18.029204,4.098356,10.481188,-2.566311,9.927770,2.344936,4.524327};

MLP mlp(NET_INPUTS,NET_OUTPUTS,layerSizes,MLP::LOGISTIC,initW,true);

void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(redled,OUTPUT);
  pinMode(greenled,OUTPUT);
  pinMode(blueled,OUTPUT);
  pinMode(value,INPUT);

  pinMode(redout,OUTPUT);
  pinMode(greenout,OUTPUT);
  pinMode(blueout,OUTPUT);

  Serial.begin(9600);

  pinMode(rgb[0], OUTPUT);
  pinMode(rgb[1], OUTPUT);
  pinMode(rgb[2], OUTPUT);
  for(int i=0;i<3;i++){
    
    digitalWrite(rgb[i], HIGH); //desligar
    delay(40);
    digitalWrite(rgb[i],LOW);
    delay(40);
   }  
  

}

void loop() {
  
  char key = keypad.getKey();
  
  if (key=='A'||key == 'B'||key == 'C'||key == '*'||key == '#'){
    if_a=false;
    if_b=false;
    if_c=false;
    if_d=false;
    if_e=false;
  }
   if ((key=='*')||(if_d==true)){
  lcd.setCursor(0, 0);
  lcd.print("    Welcome :)   ") ;
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modes--> Cal-(A)") ;
  lcd.setCursor(0, 1);
  lcd.print(" Sen-(B),RGB-(C)");
  delay(3000);
  lcd.clear();
  }
  
  if ((key=='C')||(if_c==true)){
    if_c=true;
  lcd.setCursor(0, 0);
  lcd.print("   RGB LED Mode   ") ;
  if (key=='1'){
    lcd.setCursor(0, 1);
    lcd.print("Color-->RED");
    digitalWrite(redout,HIGH);
    delay(1500);
    digitalWrite(redout,LOW);
    delay(1000);
    lcd.clear();
  }
  if (key=='2'){
    lcd.setCursor(0, 1);
    lcd.print("Color-->GREEN");
    digitalWrite(greenout,HIGH);
    delay(1500);
    digitalWrite(greenout,LOW);
    delay(1000);
    lcd.clear();
  }
  if (key=='3'){
    lcd.setCursor(0, 1);
    lcd.print("Color-->BLUE");
    digitalWrite(blueout,HIGH);
    delay(1500);
    digitalWrite(blueout,LOW);
    delay(1000);
    lcd.clear();
  }
  }
  if ((key=='B')||(if_b==true)){
  if_b=true;
  digitalWrite(redled,HIGH);
  delay(40);
  red=analogRead(value);
  delay(10);
  Serial.print("R=");
  Serial.print(red);
  Serial.println();
  digitalWrite(redled,LOW);

  digitalWrite(greenled,HIGH);
  delay(40);
  green=analogRead(value);
  delay(10);
  Serial.print("G=");
  Serial.print(green);
  Serial.println();
  digitalWrite(greenled,LOW);

  digitalWrite(blueled,HIGH);
  delay(40);
  blue=analogRead(value);
  delay(10);
  Serial.print("B=");
  Serial.print(blue);
  Serial.println();
  digitalWrite(blueled,LOW);

  
  if (red>100&&blue>100&&green>100){
    redvalue=HIGH;
    greenvalue=HIGH;
    bluevalue=HIGH;
    lcd.setCursor(0, 0);
    lcd.print("Colour Detected");
    lcd.setCursor(0, 1);
    lcd.print("   White Color   ") ;
    }


  else if (red>green&&red>blue&&50<red){
    redvalue=HIGH;
     lcd.setCursor(0, 0);
    lcd.print("Colour Detected");
    lcd.setCursor(0, 1);  
    lcd.print("    RED Color   ");
    }

  else if (green>red&&green>blue&&green>100){
    greenvalue=HIGH;
     lcd.setCursor(0, 0);
    lcd.print("Colour Detected");
    lcd.setCursor(0, 1); 
    lcd.print("   Green Color   ");
    }

  else if (blue>red&&blue>green&&blue>50){
    bluevalue=HIGH;
    lcd.setCursor(0, 0);
    lcd.print("Colour Detected");
    lcd.setCursor(0, 1);
    lcd.print("   Blue Color   ") ;
    }

   
   else{
    redvalue=LOW;
    greenvalue=LOW;
    bluevalue=LOW;
    lcd.setCursor(0, 0);
    lcd.print("Sensoring Mode");
    lcd.setCursor(0, 1);
    lcd.print("   Insert Colour   ") ;
    delay(500);
    lcd.clear();
    }
   
  digitalWrite(redout,redvalue);
  digitalWrite(greenout,greenvalue);
  digitalWrite(blueout,bluevalue);
}
if ((key=='#')||(if_e==true)){
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      OFF      ") ;
  delay(500);
  lcd.clear();
  }

 if ((key=='A')||(if_a==true)){

  if_a=true;
  
 for(int i=0;i<3;i++){
    
    digitalWrite(rgb[i], HIGH); //desligar
    delay(40);
    digitalWrite(rgb[i],LOW);
    delay(40);
   }  
   
   if(key=='1'){
    
    lcd.clear();
    lcd.print("- CALIBRATION  -");
    lcd.setCursor(0, 1);
    lcd.print("Show BLACK color");
    delay(3000);
    for(int i=0;i<3;i++){
      
    digitalWrite(rgb[i], HIGH); //desligar
    delay(40);
    digitalWrite(rgb[i],LOW);
    delay(40);
      calib[i][0] = -1;
      calib[i][1] = -1;
    }
  } else if(key=='2'){
    if(calib[0][0]==-1 || calib[0][1]==-1 ){
      getRGB(1);
      for(int i=0;i<3;i++){
           
      digitalWrite(rgb[i], HIGH); //desligar
      delay(40);
      digitalWrite(rgb[i],LOW);
      delay(40);
        calib[i][1] = input[i];
      lcd.clear();
      lcd.print("- CALIBRATION  -");
      lcd.setCursor(0, 1);
      lcd.print("Show WHITE color");
      delay(2000);
    }}
  }else if(key=='3'){
    lcd.clear();
    lcd.print("Recognizing...  ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(4000);
    getRGB(0);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    for(int i=0;i<3;i++){
      lcd.print(input[i]);
      if(i<2)
        lcd.print(" / ");
      netInput[i+1] = (double)input[i]/255.0;
    }
    lcd.setCursor(0, 1);
    lcd.print(colors[mlp.getActivation(netInput)]);
    delay(5000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
//    lcd.print("PRESS- Recognize");
//    lcd.setCursor(0, 1);
//    lcd.print("HOLD - Calibrate");
     lcd.print("cal-->BLA-(1)") ;
      lcd.setCursor(0, 1);
      lcd.print("WHI-(2),Sen-(3)");
  }
  
}


}
void getRGB(int calibration){
  for(int i=0;i<3;i++){
     digitalWrite(rgb[i], HIGH); //desligar
    delay(40);
   
    
    input[i] = analogRead(LDR_PIN);
    if(!calibration){
      input[i] = map(input[i], calib[i][0], calib[i][1], 0, 255);
      input[i] = input[i]<0?0:input[i]>255?255:input[i];
    }
    
     digitalWrite(rgb[i],LOW);
      delay(40);
  }}
