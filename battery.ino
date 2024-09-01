#include <Wire.h>
#include <String.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);


int delay_min=15;//穩壓時間
double static_volt=13.5;//穩壓電壓(以下)
double error_volt=0.2;//穩壓允許誤差值
int cn=0;
unsigned long lastUpdate = 0; // 用來記錄上次更新顯示的時間
int displayIndex = 0; // 用來記錄當前顯示的Control物件
const unsigned long displayInterval = 3000; // 顯示每個Control物件的時間間隔 (以毫秒為單位)

void printMillisecondsAsTime(unsigned  long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    milliseconds = milliseconds % 1000;

    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    
    unsigned long hours = minutes / 60;
    minutes = minutes % 60;
    Serial.print(hours);
    Serial.print(" hours,");
    Serial.print(minutes);
    Serial.print(" minutes,");
    Serial.print(seconds);
    Serial.print(" seconds,");
    Serial.print(milliseconds);
    Serial.println(" milliseconds");

}
class Control {  
  public:  
    int switchPin; // 定义微动开关连接的引脚
    int switchState=1; // 用于存储微动开关状态的变量
    String L1,L2;
    int vpin;//電壓檢測角位
    int rpin;//繼電器角位
    double volt;//現在電壓
    String state;//充電座狀態 No Battery/Charging/complete
    double temp;//暫存電壓
    double temp13;//首次13V以上電壓
    bool charge_state=false;//是否首次進入13V穩壓
    int static_time=0;//穩壓次數 每次為1分鐘
    unsigned long time_now = 0;//更新穩壓時間
    int signame=0;//充電座編號
    unsigned long current_time;
    bool onece=true;
    int LH;
    Control(int vpin,int rpin,int switchPin,int LH){//INIT
      this->vpin=vpin;
      this->rpin=rpin;
      this->switchPin=switchPin;
      this->LH=LH;
      pinMode(rpin, OUTPUT);
      pinMode(switchPin, INPUT_PULLUP);
      cn++;
      signame=cn;
    }

    void run(){
      switchState = digitalRead(switchPin); 
      volt=analogRead(vpin)/4.092/10;
      Serial.print("b");
      Serial.print(signame);
      Serial.println(":");
      Serial.print("Volt:");
      Serial.println(volt);
      Serial.print("state:");
      Serial.println(state);
      L1 = "b" + String(signame) + "   " + state;
      L2 = "  V:" + String(volt) + " ST:" + String(static_time);
      if(LH==0){
      if(switchState == 0&&onece){
        if(volt>=13&&volt<static_volt){
          Serial.print("static_times:");
          Serial.println(static_time);
          Serial.print("First 13V volt:");
          Serial.println(temp13);
          Serial.print("error volt:");
          Serial.println(temp-volt);
          Serial.print("current_time:");
          printMillisecondsAsTime(millis()-current_time);
          if(!charge_state){
            time_now=millis();
            current_time=time_now;
            temp13=volt;
            temp=volt;
            charge_state=true;
          }
          if((abs(volt-temp13)<=error_volt/*誤差電壓*/)&&(millis()-time_now>=60000/*每隔1分鐘檢測*/)){
            temp=volt;
            static_time++;
            time_now=millis();
          }else if((abs(temp13-temp)<=error_volt)){
            temp=volt;
          }else{
            static_time=0;
            charge_state=false;
          }
          if(static_time>=delay_min/*5分鐘穩壓停止充電*/){
            state="Complete";
            digitalWrite(rpin, HIGH);  
            onece=false;
          }else{
            state="Charging";
            digitalWrite(rpin, LOW); 
          }
        }else{
          state="Charging";
          digitalWrite(rpin, LOW); 
          static_time=0;
        }
      }else if(switchState == 0&&!onece){
          Serial.println("--------------------");
          digitalWrite(rpin, HIGH); 
      }else{
        Serial.println("--------------------");
        state="NoBattery";
        digitalWrite(rpin, LOW); 
        charge_state=false;
        static_time=0;
        onece=true;
      }
      }
      else{
      if(switchState == 0&&onece){
        if(volt>=13&&volt<static_volt){
          Serial.print("static_times:");
          Serial.println(static_time);
          Serial.print("First 13V volt:");
          Serial.println(temp13);
          Serial.print("error volt:");
          Serial.println(temp-volt);
          Serial.print("current_time:");
          printMillisecondsAsTime(millis()-current_time);
          if(!charge_state){
            time_now=millis();
            current_time=time_now;
            temp13=volt;
            temp=volt;
            charge_state=true;
          }
          if((abs(volt-temp13)<=error_volt/*誤差電壓*/)&&(millis()-time_now>=60000/*每隔1分鐘檢測*/)){
            temp=volt;
            static_time++;
            time_now=millis();
          }else if((abs(temp13-temp)<=error_volt)){
            temp=volt;
          }else{
            static_time=0;
            charge_state=false;
          }
          if(static_time>=delay_min/*5分鐘穩壓停止充電*/){
            state="Complete";
            digitalWrite(rpin, LOW);  
            onece=false;
          }else{
            state="Charging";
            digitalWrite(rpin, HIGH); 
          }
        }else{
          state="Charging";
          digitalWrite(rpin, HIGH); 
        }
      }else if(switchState == 0&&!onece){
          Serial.println("--------------------");
          digitalWrite(rpin, LOW); 
      }else{
        Serial.println("--------------------");
        state="NoBattery";
        digitalWrite(rpin, HIGH); 
        charge_state=false;
        static_time=0;
        onece=true;
      }
      }
      Serial.println("--------------------");
    }
    
};  

Control b1(A0,7,3,0);
Control b2(A1,6,2,0);
Control b3(A2,5,A4,1);
Control b4(A3,4,A5,1);
Control* controls[4] = { &b1,&b2,&b3,&b4 }; // 將所有 Control 物件加入陣列


void setup() {  
  Serial.begin(115200);
  lcd.begin(16, 2);  
  Serial.print("Enable Charge Control:");
  Serial.println(cn);
}

void loop() {
  unsigned long currentMillis = millis();
//  b1.run();   
//  b2.run(); 
  for (int i = 0; i < cn; i++) {
    controls[i]->run();
  }
    if (currentMillis - lastUpdate >= displayInterval) {
    lastUpdate = currentMillis;
    displayIndex = (displayIndex + 1) % cn; // 循環切換顯示
  }

  // 顯示當前選擇的Control物件
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(controls[displayIndex]->L1);
  lcd.setCursor(0, 1);
  lcd.print(controls[displayIndex]->L2);
  Serial.println("Overview:");
  Serial.print("Times:");
  printMillisecondsAsTime(millis());
  Serial.println("--------------------");

  delay(1000);
}
