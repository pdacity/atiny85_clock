/* 
 *  https://robocraft.ru/blog/3654
 *  
 *  Oled часы Attiny 85
 *  
 *  Подключение следующим образов
 *  P0 — SDA OLED
 *  P1 — Кнопка SET
 *  P2 — CSL OLED
 *  P3 — Кнопка +
 *  P4 — DS18B20
 *  P5 — Не подключен (RESET контролера)
 *  
 *  https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoISP
*/



#include <TinyOzOLED.h>

#include <OneWire.h>

OneWire  ds(4);  // нужна подтяжка к VCC через 4.7 кОм, первый датчик работает при 3.3 кОм

int16_t raw;
byte k;
boolean izm = true;
boolean vremya = true;
byte present = 0;
byte type_s = 0;
byte data[12];
byte addr[8]={0x28, 0xFF, 0x75, 0x4E, 0x87, 0x16, 0x5, 0x63};//адрес второго моего датчика
float celsius;
char prncelsius[10];

int i=0;
long prevmicros = 0;   //переменная для хранения значений таймера
boolean counter=false; // счетчик для полусекунд 

int sek=0;             //значение секунд
int minu=0;            //значение минут
int chas=0;            //значение часов 

int knset = 3;
int knplus = 1;
boolean set;
boolean prset = true ;
boolean plus;
boolean prplus = true ;

char valchas[10];
char valminu[10];

void setup(){
  delay(500);
  digitalWrite(knset, HIGH);
  digitalWrite(knplus, HIGH);  
   
  OzOled.init();  //инициализация дисплей
  OzOled.clearDisplay();          //очистка дисплея
  OzOled.setNormalDisplay();      //нормальный режим
  OzOled.setPageMode();           //адресация страничная
  OzOled.sendCommand(0xA1);       //выбор ориентации сверху - вниз
  OzOled.sendCommand(0xC8);       //слева - направо
   
}

void loop(){
  set = digitalRead(knset);
  plus = digitalRead(knplus);  
  
   if (micros() - prevmicros >499900) { // поменять на другое для корректировки было 500000
      //значение между 499500(спешат) и 500000(отстают)      
     prevmicros = micros();  //принимает значение каждые полсекунды
     counter=!counter;
     if (counter==false)     { 
      sek++;              //переменная секунда + 1          
              
        if(vremya == true){
          OzOled.printString("Time            ", 0, 0);          
          //Вывод часов на дисплей
          if (chas <= 9) {           //часы
       dtostrf(chas, 1, 0, valchas);
       OzOled.printBigNumber(" ", 0, 2);
       OzOled.printBigNumber(valchas, 3, 2);
       }
       else{
       dtostrf(chas, 2, 0, valchas);
       OzOled.printBigNumber(valchas, 0, 2); 
       }      
       OzOled.printBigNumber(".", 6, 2);
       
        if (minu <= 9) {         //минуты
       dtostrf(minu, 1, 0, valminu);
       OzOled.printBigNumber("0", 9, 2);//Первая цифра должна быть ноль
       OzOled.printBigNumber(valminu, 12, 2); 
       }
       else{
       dtostrf(minu, 2, 0, valminu);
       OzOled.printBigNumber(valminu, 9, 2); 
       }                          
     }
    else{
      OzOled.printString("Room temperature", 0, 0);      
      OzOled.printBigNumber(" ", 0, 2);                     
      OzOled.printBigNumber(prncelsius, 3, 2);       
    }    
   }
        
     
     else     { //в полусекунде 
      switch (i){        
        case 1:        
        OzOled.printBigNumber("..", 0, 2);
        break;   
        case 2:       
        OzOled.printBigNumber("..", 9, 2);
        break;
        case 3: 
        OzOled.printBigNumber("..", 0, 2); 
        OzOled.printBigNumber("..", 9, 2);      
        sek = 0;
        break;        
      }
     }   
   }
    
    // Начало получения температуры
    if(sek==0&&izm==true || sek==20&&izm==true || sek==40&&izm==true){      
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 0);        // провести измерение температуры и записать данные в оперативную память
                             //1 - паразитное, 0 - внешнее
    izm=false;          
    }

    if(sek==1&&izm==false || sek==21&&izm==false || sek==41&&izm==false){    
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // считать последовательно 9-ти байт оперативной памяти
  
    for ( k = 0; k < 9; k++) {           // чтение 9-ти байт
     data[k] = ds.read();    
    }  
    // операции преобразования  
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
     raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
    } else {
    byte cfg = (data[4] & 0x60);    
    if (cfg == 0x00) raw = raw & ~7;  
    else if (cfg == 0x20) raw = raw & ~3; 
    else if (cfg == 0x40) raw = raw & ~1;   
    }
  
    celsius = (float)raw / 16.0; 
    dtostrf(celsius, 2, 1, prncelsius);     
    izm=true;            
    }
    //Конец полцчения температуры
     
     if(sek>=0 && sek<=10 || sek>=20 && sek<=30 || sek>=40 && sek<=50){
         vremya = true; 
        }
        else{
         vremya = false;
        }
     
     if(sek>59) {//если переменная секунда больше 59 ...          
     sek=0;//сбрасываем ее на 0
     minu++;//пишем +1 в переменную минута              
        }          
      
     
     if(minu>59) { //если переменная минута больше 59 ..
     minu=0;//сбрасываем ее на 0
     chas++;//пишем +1 в переменную час       
     }  

     if(chas>23) { //если переменная час больше 23 
     chas=0;//сбрасываем ее на 0 
     } //конец часов

   // установка времени кнопками
   if (set != prset)  { //проверка нажатия кнопки установки
   prset = set;
   if (set == false) {
    i++;    
   }  
  delay (10); 
   } 
   
     switch (i) {
       case 1:
       vremya = true;
        if (plus != prplus)  {
          prplus = plus;  
           if (plus == false) {
             chas++;
             } 
           delay (10);              
          }           
        break;
        
        case 2:
        vremya = true;
        if (plus != prplus)  {
          prplus = plus;  
           if (plus == false) {
             minu++;
             } 
             delay (10);            
          }           
        break; 
        
        case 3:
        vremya = true;          
        break;
        
        case 4:
        vremya = true;
        i=0;
        break;        
      }
    // конец установки кнопками 

       
} //end loop
