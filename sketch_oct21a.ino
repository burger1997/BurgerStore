#define BAUD 9600
#define out 11
#define in 12
#define clk 13
#define SS 10
#define SLECT_ON digitalWrite(10,LOW);
#define SLECT_OFF digitalWrite(10,HIGH);

byte buff[256];
byte ins[1];
byte Address[3]={0x00,0x00,0x00};    //不更動的Address
byte address[3]={0x00,0x00,0x00};    //使用的address
byte chip_is_full='0';
int continue_select_frequence=0;
char previous_select=0;

bool SendByte(int length,byte* arry){
  int i;
  int j;
  bool output;  
  for(i=0;i<length;i++){
    for(j=7;j>=0;j--){
      output=bitRead(*(arry+i),j);
      digitalWrite(clk,0);
      digitalWrite(out,output);
      delay(1);
      digitalWrite(clk,1);
      delay(1);
    }
  }
  return 0;
}
byte ReadByte(){
  byte vaule=0;
  int j=0;
  for(j=7;j>=0;j--){
      digitalWrite(clk,0);
      delay(1);
      
      digitalWrite(clk,1);
      vaule=ldexp(digitalRead(in),j)+vaule;
      delay(1);
    }
   return vaule;
}
bool ReadSerialData(){
  int counter=0;
  while(counter<256){
    if(Serial.available()>0){
      buff[counter]=Serial.read();
      counter++;
    }
  }
}
void WriteEnable(){
  SLECT_ON
  ins[0]=0x06;        //write enable
  SendByte(1,ins);
  SLECT_OFF
}
void NextPage(){
  int i;
  if(continue_select_frequence>0){  //若連續選擇同個功能,則翻頁
    if(address[1]==0xFF){
      address[1]=0;
      address[0]++;
      chip_is_full='0';
    }
    else{
      address[1]++;
      chip_is_full='0';
    }
    if(address[0]>0x0F)
      chip_is_full='1';
  }
  else{
    for(i=0;i<3;i++){
       address[i] = Address[i];
    }
  }
}
void setup() {
  Serial.begin(BAUD);
  pinMode(out,OUTPUT);
  pinMode(in ,INPUT);
  pinMode(SS,OUTPUT);
  pinMode(clk,OUTPUT);
  digitalWrite(clk,HIGH);
  digitalWrite(SS,HIGH);
  Serial.setTimeout(10);
}

void loop() {
  int i=0;
  int j=0;
  
  if(Serial.available()>0){
    char select=Serial.read();
    
    if(select==previous_select){
      continue_select_frequence++;
    }
    else{
      continue_select_frequence=0;
    }
    previous_select=select;
    NextPage();

    
//change address
    if(select=='0'){
     
     for(i=0;i<3;i++){
      Serial.write(Address[i]);
     }
     j=0;
     while(j<3){
      if(Serial.available()>0){
        Address[j]=Serial.read();
        j++;
       }
      }
      
      for(i=0;i<3;i++){
       address[i]=Address[i];
      }
    }
//read
   else if(select=='1'){    
      SLECT_ON
      ins[0]=0x03;
      SendByte(1,ins);
      SendByte(3,address);
      for(i=0;i<256;i++)
         Serial.write(ReadByte());
      SLECT_OFF
    }    
//read ID
    else if(select=='2'){    
      SLECT_ON
      ins[0]=0x9F;
      SendByte(1,ins);
      for(i=0;i<3;i++){
        Serial.write(ReadByte());
      }
      SLECT_OFF
    }
//write
    else if(select=='3'){  
      i=0;
      ReadSerialData();
      WriteEnable();
      delay(1);
      
      SLECT_ON
      ins[0]=0x02;            //page program
      SendByte(1,ins);      
      SendByte(3,address);    //write address
      SendByte(256,buff);     //write serial data in to chip
      SLECT_OFF

      Serial.write(chip_is_full);
    }
//erase
    else if(select=='4'){
      WriteEnable();
      delay(1);
      SLECT_ON
      ins[0]=0xD8;        //block erase
      SendByte(1,ins);      
      SendByte(3,address);    //write address
      SLECT_OFF
    }
//verify
    else if(select=='5'){
      bool safe=true;
      ReadSerialData();
      
      SLECT_ON
      ins[0]=0x03;
      SendByte(1,ins);
      SendByte(3,address);
      for(i=0;i<256;i++){
        if(ReadByte()!=buff[i]){
          safe=false;
        }
      }
      Serial.write("0");
      Serial.write(safe); 
      SLECT_OFF
    }
    else{
      while(Serial.read() >= 0){}   //clean up buffer
    }
  }
}
