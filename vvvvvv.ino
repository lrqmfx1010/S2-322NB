#include <S2_322NB.h>
#include <SoftwareSerial.h>

#define OK_DIST 35        // 设置避障距离
#define MAN 0             // 手动模式
#define AUTO 1            // 自动模式
#define GEST 2            // 体感模式

#define TURN_LEFT_90   900        // 左转90度延迟参数
#define TURN_RIGHT_90   1000      // 右转90度延迟参数
#define TURN_BACK   1600          // 掉头延迟参数


S2_322_nb S2_322NB(1, 0, 0, 1, 200, A0, A1, 10); 
                                                                                               

SoftwareSerial softSerial(9, 2);    

char cmdChar = '5';             
char cmdCharSave = cmdChar;     
byte systemMode = MAN;          
                           
void setup() {

  softSerial.begin(9600);     
  delay(100);  

  Serial.begin(9600);       
  delay(100);  
  
  S2_322NB.headServoIni();    //头部舵机初始化
  S2_322NB.setHeadPos(90);    
    
  
}

void loop() {

  if(softSerial.available()){           
    cmdChar = softSerial.read();        
    Serial.print("cmdChar = ");        
    Serial.println(cmdChar);           
  }   

  if(Serial.available()){             
    cmdChar = Serial.read();          
    Serial.print("cmdChar = ");       
    Serial.println(cmdChar);          
  } 

  runMode();                           // 执行运行模式并实施相应控制
}

// 执行运行模式并实施相应控制
void runMode(){
  switch(cmdChar){
    case 'A':                       // 微信端输入控制指令字符为自动运行字符'A'
      systemMode = AUTO;            // 将当前运行模式控制变量设置为AUTO
      break;
      
    case 'M':                       // 微信端输入控制指令字符为自动运行字符'M'          
      systemMode = MAN;             // 将当前运行模式控制变量设置为手动  
      S2_322NB.stop();               // 切换为手动模式后自动停车
      cmdChar =  '5';               // 并且将控制指令字符设置为停车字符
      break;      

    case 'G':                       // 微信端输入控制指令字符为自动运行字符'G'          
      systemMode = GEST;             // 将当前运行模式控制变量设置为体感模式
      S2_322NB.stop();               // 切换为体感模式后自动停车
      cmdChar =  '5';               // 并且将控制指令字符设置为停车字符
      break;        
  }
  
  if (systemMode == MAN || systemMode == GEST ){  // 如果当前运行模式为手动或体感
    manMode();                      // 则使用手动控制函数控制小车
  } else if(systemMode == AUTO){    // 如果当前运行模式为自动
    autoMode();                     // 则使用自动控制函数控制小车
  }     
  
  modeReport();       // 输出运行模式信息
}

// 小车手动控制函数
void manMode(){

  switch(cmdChar){ 

    case '8': // 前进
      S2_322NB.forward();
      break;             

    case '2': // 后退
      S2_322NB.backward();
      break;   

    case '4': //左转
      S2_322NB.turnL();
      break;        

    case '6': //右转
      S2_322NB.turnR();
      break;    
      
    case '5': //停止
      S2_322NB.stop();
      break;       

    case '7': //左前
      S2_322NB.forwardL();
      break;         

    case '9': //右前
      S2_322NB.forwardR();
      break;      

    case '1': //右后
      S2_322NB.backwardR();
      break; 

    case '3': //左后
      S2_322NB.backwardL();
      break;                 
  }

}

// 小车自动控制函数
void autoMode(){ 
  if(cmdChar == '5'){       // 如果控制字符为'5'停止
    S2_322NB.stop();         // 小车停止
    S2_322NB.setHeadPos(90); // 头部舵机恢复向前
  } else {                  // 如果控制字符不是'5'停止
    delay(50);              // 等待50ms
    int frontDist = S2_322NB.getDistance(); // 检查前方距离
    if(frontDist >= OK_DIST){          // 如果检测前方距离读数大于等于允许距离
      S2_322NB.forward();               // 小车向前
    } else {                           // 如果检测前方距离小于允许距离
      S2_322NB.stop();                  // 小车停止   
      autoTurn();                      // 检测左右侧距离并做出自动转向 
    }
  }
}

// 检查左右方向距离并返回方向
void autoTurn(){
  
  // 检查右侧距离     
  for (int pos = 90; pos >= 0; pos -= 1) {
    S2_322NB.setHeadPos(pos);               
    delay(3);                     
  }
  delay(300);
  int rightDist =  S2_322NB.getDistance();
  Serial.print("rightDist =  ");Serial.println(rightDist);

  // 检查左侧距离     
  for (int pos = 0; pos <= 180; pos += 1) {
    S2_322NB.setHeadPos(pos);                
    delay(3);                     
  }
  delay(300);
  int leftDist =  S2_322NB.getDistance();  
  Serial.print("leftDist =  ");Serial.println(leftDist);

  //将头部调整到正前方
  for (int pos = 180; pos >= 90; pos -= 1) {
    S2_322NB.setHeadPos(pos);               
    delay(3);                     
  }
  delay(500);
  
  //检查左右距离并做出转向决定
  if ( rightDist < OK_DIST && leftDist < OK_DIST){  // 如果左右方向距离均小于允许距离
    Serial.println("Turn 180");                    
    turnBack();                                     // 掉头
    return;
  } else if ( rightDist >= leftDist){               // 如果右边距离大于左边距离
    Serial.println("Turn Right");                  
    turnR90();                                      // 右转90度
    return;
  } else {                                         // 如果左边距离大于右边距离
    Serial.println("Turn Left");                  
    turnL90();                                     // 左转90度
    return;
  }
}

// 左转90度
void turnL90(){
  S2_322NB.turnL();
  delay(TURN_LEFT_90);
}

// 右转90度
void turnR90(){
  S2_322NB.turnR();
  delay(TURN_RIGHT_90);  
}

// 掉头
void turnBack(){
  S2_322NB.turnL();
  delay(TURN_BACK);  
}



void modeReport(){
  if(cmdCharSave != cmdChar){
    
    cmdCharSave = cmdChar;
    
    if (systemMode == MAN){
      Serial.println("S2_322_nb: MANUAL Mode");
      opReport();           
    } else if (systemMode == AUTO){
      Serial.println("S2_322_nb: AUTO Mode");
    } else if (systemMode == GEST){
      Serial.println("S2_322_nb: GEST Mode");
      opReport();           
    } 
     
  }
}


void opReport(){
  switch(cmdChar){   

    case '8': 

        Serial.println("S2_322_nb: FORWARD");
 
      break;             

    case '2':

        Serial.println("S2_322_nb: BACKWARD");
 
      break;   

    case '4':

        Serial.println("S2_322_nb: TURN LEFT");
 
      break;        

    case '6':

        Serial.println("S2_322_nb: TURN RIGHT");
 
      break;    
      
    case '5':

        Serial.println("S2_322_nb: STOP");
   
      break;       

    case '7':

        Serial.println("S2_322_nb: FORWARD LEFT");
   
      break;         

    case '9':

        Serial.println("S2_322_nb: FORWARD RIGHT");
   
      break;      

    case '1':

        Serial.println("S2_322_nb: BACKWARD LEFT");
   
      break; 

    case '3':

        Serial.println("S2_322_nb: BACKWARD RIGHT");
   
      break;   
  }
}

/*
控制指令字符

模式控制
AUTO  ---  自动避障模式
MAN   ---  人工控制模式
GEST  ---  体感控制模式

运行控制
8  ---  前进
2  ---  后退
4  ---  左转
6  ---  右转
5  ---  停车
7  ---  左前
9  ---  右前
1  ---  左后
3  ---  右后
A  ---  自动模式
M  ---  手动模式
G  ---  体感模式
*/
