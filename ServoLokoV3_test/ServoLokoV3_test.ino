byte esqVazio[] = {
  B00011,
  B00100,
  B01000,
  B10011,
  B10100,
  B10100,
  B01010,
  B00110
};

byte esqMeio[] = {
  B00011,
  B00111,
  B01111,
  B11111,
  B10100,
  B10100,
  B01010,
  B00110
};

byte esqCheio[] = {
  B00011,
  B00111,
  B01111,
  B11111,
  B11100,
  B11100,
  B01110,
  B00110
};

byte esqMeio2[] = {
  B00011,
  B00100,
  B01100,
  B11111,
  B11100,
  B11100,
  B01110,
  B00110
};

byte dirVazio[] = {
  B11000,
  B00100,
  B00010,
  B11001,
  B00101,
  B00101,
  B01010,
  B01100
};

byte dirMeio[] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B00101,
  B00101,
  B01010,
  B01100
};

byte dirCheio[] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B00111,
  B00111,
  B01110,
  B01100
};

byte dirMeio2[] = {
  B11000,
  B00100,
  B00110,
  B11111,
  B00111,
  B00111,
  B01110,
  B01100
};

#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo1;
Servo servo2;

LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // RS, E, D4, D5, D6, D7

byte page = 0;   //0 = Principal | 1 = Menu | 2 = Configs | 3 = Programar | 4 = Calibração | 
byte scroll = 0; //Depende da pagina :/
byte zona = 0;   //0 = Completa (apenas manual) | 1 = Frente Esquerda (padrão) | 2 = Frente Direita | 3 = 50% Esquerda | 4 = 50% Direita
volatile bool press1 = LOW;
volatile bool press2 = LOW;
byte ang1 = 180;
byte ang2 = 180;
byte a1;
byte a2;
bool mode = 0; //0 = Manual | 1 = Auto
bool calc = 0; //0 = Angulo | 1 = Inverse Kinematics
float distX = 20.0;
float distY = 20.0;
unsigned long actime = 0;
unsigned long lastpress1 = 0;
unsigned long lastpress2 = 0;
unsigned long lastLCD = 0;
const byte but1UpTime = 700;
const byte but2UpTime = 700;
const byte lcdUpTime  = 400;
const float easing = 0.05;


int P1 = 0;
int P2 = 0;
int minP1 = 0;
int minP2 = 0;
int maxP1 = 1023;
int maxP2 = 1023;

void setup(){
  analogReference(EXTERNAL);
  lcd.begin(16, 2);
  lcd.createChar(0, esqVazio);
  lcd.createChar(1, esqMeio);
  lcd.createChar(2, esqCheio);
  lcd.createChar(3, esqMeio2);
  lcd.createChar(4, dirVazio);
  lcd.createChar(5, dirMeio);
  lcd.createChar(6, dirCheio);
  lcd.createChar(7, dirMeio2);
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2),but1, FALLING);
  attachInterrupt(digitalPinToInterrupt(3),but2, FALLING);
  servo1.attach(5);
  servo2.attach(6);
  Serial.begin(9600);
}

void but1(){
  press1 = HIGH;
}

void but2(){
  press2 = HIGH;
}
void loop(){ 
  actime = millis();
  inputCheck();
  if(actime - lastLCD > lcdUpTime){lcd_update(); lastLCD = actime;}
  //Serial.print(P1);
  //Serial.print(" | ");
  Serial.print(a1);
  Serial.print(" | ");
  Serial.println(ang1);
  
  distX = constrain(mapa(P1, minP1, maxP1, 0.0 , 20.0),0.0, 20.0 );
  distY = constrain(mapa(P2, minP2, maxP2, 0.0 , 20.0),0.0, 20.0 );
  if(!mode && page == 0){ServoMove();} 
  if(mode && page == 0){
    for(int i = 0; i < 15; i++){
      automatico();
      page = 0;
    }
    mode = 0;
    page = 0;
  } 
}

float mapa(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void inputCheck(){ 
  P1 = analogRead(A0);
  P2 = analogRead(A1);
  if(!calc){
    ang1= constrain(map(P1, minP1, maxP1,0 ,180),0,180);
    ang2= constrain(map(P2, minP2, maxP2,0 ,180),0,180);
    //if(abs(a1 - ang1) > 1){ang1 = ang1 + (a1 - ang1) * easing;}
    //if(abs(a2 - ang2) > 1){ang2 = ang2 + (a2 - ang2) * easing;}
  } else {
    calculate();
  }
  
  bool inputA,inputB; //A é o MUDAR e B é o CONFIRMA
  if(actime - lastpress1 > but1UpTime){if(press1) {inputB = HIGH; press1=0; lastpress1 = actime;}} else {press1 = 0; inputB = LOW;}
  if(actime - lastpress2 > but2UpTime){if(press2) {inputA = HIGH; press2=0; lastpress2 = actime;}} else {press2 = 0; inputA = LOW;}
  if(inputA + inputB) lastLCD = actime;
  switch(page){
    case 0: if(inputB)changePage(1); break; //Pag Principal
    case 1:                                //Menu
          if(inputA){scroll++; if(scroll > 3) scroll = 0;}
          if(inputB){
            if(scroll == 0)changePage(2);
            if(scroll == 1)changePage(4);
            if(scroll == 2)changePage(3);
            if(scroll == 3)changePage(0);
            }
          break;
    case 2: // Configs
          if(inputA){scroll++; if(scroll > 3) scroll = 0;}
          if(inputB){
             if(scroll == 0)mode = !mode;
             if(scroll == 1)calc = !calc;
             if(scroll == 2)zona++; if(zona > 4) zona = 0;
             if(scroll == 3)changePage(1);
          }
          break;
    case 3: //Programar
          changePage(1);
          break;
    case 4: //calibração
         if(scroll == 3) {if(P1 < minP1){minP1 = P1;} if(P1 > maxP1){maxP1 = P1;}}
         if(scroll == 4) {if(P2 < minP2){minP2 = P2;} if(P2 > maxP2){maxP2 = P2;}}
         if(inputA && scroll <  3){scroll++; lcd.clear(); if(scroll > 2) scroll = 0;}
         if(inputA && scroll >= 3){scroll = scroll - 3; lcd.clear();}
         if(inputB){
             if(scroll == 0){scroll = 3; minP1 = 99; maxP1 = 900;} 
             if(scroll == 1){scroll = 4; minP2 = 99; maxP2 = 900;};
             if(scroll == 2)changePage(1);           
          }
        break;
  }
}

void ServoMove(){
  if(mode == 0){
  servo1.write(ang1);
  servo2.write(ang2);
  }
}

void changePage(byte pag){
  page = pag;
  lcd.clear();
  scroll = 0;
}

void drawMode(){
  lcd.setCursor(14,0);
  switch(zona){
  case 0: lcd.write(byte(2)); lcd.write(byte(6)); break;//0 = Completa (apenas manual)| 
  case 1: lcd.write(byte(1)); lcd.write(byte(4)); break;//1 = Frente Esquerda (padrão | 
  case 2: lcd.write(byte(0)); lcd.write(byte(5)); break;//2 = Frente Direita          | 
  case 3: lcd.write(byte(3)); lcd.write(byte(4)); break;//3 = 50% Esquerda            | 
  case 4: lcd.write(byte(0)); lcd.write(byte(7)); break;//4 = 50% Direita             |
  }
  lcd.setCursor(15,1); if(!mode){lcd.print("M");} else {lcd.print("A");}
}

void lcd_update(){
  lcd.setCursor(0,0);
  switch(page){ //pag principal
    case 0:     
      //espaço pra codigo com o automatico :)
        if(!calc){lcd.print(ang1); lcd.print(" "); lcd.setCursor(4,0); lcd.write("graus");}
        else{lcd.print(distX)    ; lcd.print(" "); lcd.setCursor(6,0); lcd.write("cm");}
        lcd.setCursor(0,1);
      //espaço pra codigo com o automatico :)
        if(!calc){lcd.print(ang2); lcd.print(" "); lcd.setCursor(4,1); lcd.write("graus");}    
        else{lcd.print(distY)    ; lcd.print(" "); lcd.setCursor(6,1); lcd.write("cm");}
        drawMode();
      break;
    case 1:    //Menu de opções
        if(scroll == 0){lcd.print("CONFIG calibr"); lcd.setCursor(0,1); lcd.print("progrm voltar");}
        if(scroll == 1){lcd.print("config CALIBR"); lcd.setCursor(0,1); lcd.print("progrm voltar");}
        if(scroll == 2){lcd.print("config calibr"); lcd.setCursor(0,1); lcd.print("PROGRM voltar");}
        if(scroll == 3){lcd.print("config calibr"); lcd.setCursor(0,1); lcd.print("progrm VOLTAR");}   
        drawMode(); 
      break;  
    case 2:  //Configurações
        if(scroll == 0){ lcd.print("Modo: ");     if(!mode){lcd.print("manual    ");} else{lcd.print("automatico");}}
        if(scroll == 1){lcd.print("Controle: "); if(!calc){lcd.print("angulo ");} else{lcd.print("calculo");}}
        if(scroll == 2){lcd.print("Zona Auto:  ");
                   if(zona == 0){lcd.write(byte(2)); lcd.write(byte(6)); lcd.print("! ") ;} //0 = Completa (apenas manual)| 
                   if(zona == 1){lcd.write(byte(1)); lcd.write(byte(4)); lcd.print("  ");} //1 = Frente Esquerda (padrão)| 
                   if(zona == 2){lcd.write(byte(0)); lcd.write(byte(5)); lcd.print("  ");} //2 = Frente Direita          | 
                   if(zona == 3){lcd.write(byte(3)); lcd.write(byte(4)); lcd.print("  ");} //3 = 50% Esquerda            | 
                   if(zona == 4){lcd.write(byte(0)); lcd.write(byte(7)); lcd.print("  ");} //4 = 50% Direita             |
                   }
       if(scroll == 3){lcd.print("Retornar ao menu");}
      lcd.setCursor(0,1); lcd.print("A prox | B mudar");
      break;
    case 3:  //Programação
       //aprender a colocar nomes legais e saber a usar as coisas direito :)
      break;
    case 4: // Calibração
      if(scroll == 0 || scroll == 3){lcd.print("P1: "); lcd.print(minP1); lcd.setCursor(7,0); lcd.print("- "); lcd.print(maxP1); lcd.print("   ");}
      if(scroll == 1 || scroll == 4){lcd.print("P2: "); lcd.print(minP2); lcd.setCursor(7,0); lcd.print("- "); lcd.print(maxP2); lcd.print("   ");}
      if(scroll == 2) lcd.print("Retornar ao menu");
      lcd.setCursor(0,1);
      if(scroll < 3){lcd.print("A prox | B mudar");} else {lcd.print("Calibrando...   ");}
      break;
    default: page = 0; break;      
  }
  
}

//famoso ctrl + c e ctrl + V aqui em baixo, vou deletar dps, eu acho...
void automatico() {
  for (int i = 0; i < 29; i++) {
    float angulo1 = -0.12 * i * i + 6.8 * i + 60; //-------
    float angulo2 = -0.14 * i * i + (-5.6 * i) + 70;//--------
    servo1.write(map(angulo1, 0, 180, 180, 0));
    servo2.write(angulo2);
    ang1 = angulo1;
    ang2 = angulo2;
    delay(50);
  }

  for (int i = 29; i >= 0; i--) {
    float angulo1 = -0.12 * i * i + 6.8 * i + 60;//-------
    float angulo2 = -0.14 * i * i + (-5.6 * i) + 70;//------
    servo1.write(map(angulo1, 0, 180, 180, 0));
    servo2.write(angulo2);
    ang1 = angulo1;
    ang2 = angulo2;
    delay(50);
  }
}


void calculate(){
  //posições
    float x_pos = distX;
    float y_pos = distY;
//tamanho dos braços
    float a2 = 10.0;
    float a4 = 10.0;

//variaveis da conta
    bool debug = true;
    float r1 = 0.0;
    float phi_1 = 0.0;
    float phi_2 = 0.0;
    float phi_3 = 0.0;
    float theta_1 = 0.0;
    float theta_2 = 0.0;

    r1 = sqrt((x_pos * x_pos) + (y_pos * y_pos));
    phi_1 = acos(((a4 * a4) - (r1 * r1) - (a2 * a2))/(-2.0 * r1 * a2));
    phi_2 = atan2(y_pos,x_pos);
    theta_1 = phi_2 + phi_1;
    phi_3 = acos(((r1 * r1) - (a2 * a2) - (a4 * a4))/(-2.0 * a2 * a4));
    theta_2 = PI - phi_3;
    theta_1 = theta_1 * RAD_TO_DEG; // Joint 1
    theta_2 = theta_2 * RAD_TO_DEG; // Joint 2
    if(debug){
    Serial.print("Theta 1: ");
    Serial.print(theta_1);
    Serial.println(" degrees");
    Serial.print("Theta 2: ");
    Serial.print(theta_2);
    Serial.println(" degrees");
    Serial.println();
    }
    ang1 = theta_1+90;
    ang2 = map(theta_2, -90, 90, 0, 180);
}
