#include <iostream>
#include "SerialPort.h"
#include <stdio.h>
#include <string.h>

using namespace std;

#define MAX_DATA_LENGTH 256
char portName[] = "\\\\.\\COM3";
char option[1];
char address[50]="C:\\Users\\User\\Desktop\\example2.bin";
//Arduino SerialPort object
SerialPort* arduino;

void getfile() {
	FILE* stream;
	errno_t err;
	char address[50];
	int c=0;
	char arr[16];
	int i=0,j;
	cin >> address;
	const char*  Address = address;
	err = fopen_s(&stream, Address , "rb");
	if (stream != NULL) {
		do {
			printf(" ");
			for ( i = 0; i < sizeof(arr); i++)
			{
				c = getc(stream);
				arr[i] = c;
			}
			printf("\n");
			for (j = 0; j < i-1; j++)
			{
				printf("%x ",arr[j]);
			}
			Sleep(2000);
		} while (c != EOF);
		fclose(stream);
	}
	else
	{
		puts("error");
	}
}
void SendDataFromFile() {
	FILE* stream;
	errno_t err;
	char serial_return[1], send_data[MAX_DATA_LENGTH];
	int readResult;
	int c;
	int i = 0;
	bool busy;
	bool chip_is_full=false;

	const char* Address = address;
	err = fopen_s(&stream, Address, "rb");

	if (stream != NULL) {
		printf("busy...\n");
		do {
			for (i = 0; i < MAX_DATA_LENGTH; i++)							//將資料放入arry
			{
				c = getc(stream);
				send_data[i] = c;
			}
			arduino->writeSerialPort(option, 1);							//傳送指令與arry
			arduino->writeSerialPort(send_data, MAX_DATA_LENGTH);
			busy = true;
			while (busy == true)											//等待回傳訊息表示資料寫入完畢
			{
				int readResult = arduino->readSerialPort(serial_return, 1);
				Sleep(10);
				if (readResult == 1)
				{
					busy = false;
					if (serial_return[0]=='1')								//若回傳訊息為'1'表示晶片以寫滿
						chip_is_full = true;
				}

			}
		} while (c != EOF && chip_is_full==false);
		fclose(stream);
		
		printf("finish\n");
		if (chip_is_full == true)
		{
			printf("資料寫滿了!!\n");
		}
	}
	else
	{
		puts("file open error");
	}
}
void Read(void)
{
	unsigned char byte;
	char readdata[1];
	int i=0;
	arduino->writeSerialPort(option, 1);
	
	while (i<256)
	{
		int readResult = arduino->readSerialPort(readdata, sizeof(readdata));
		byte = readdata[0];
		Sleep(10);
		
		if (readResult==1)
		{
			if (i % 16 == 0) 
				printf("\n %02x line ", i );
			printf("%02x ", byte);
			i++;
		}
	}
}
void ReadIDorAddress()
{
	unsigned char id_byte;
	int i = 0;
	char readid[1];
	arduino->writeSerialPort(option, 1);
	while (i < 3)
	{
		int readResult = arduino->readSerialPort(readid, 1);
		Sleep(10);
		if (readResult == 1)
		{
			id_byte = readid[0];
			printf("%02x ", id_byte);
			i++;
		}
	}
}

void autoConnect(void)
{
	//wait connection
	while (!arduino->isConnected()) {                       
		Sleep(100);
		arduino = new SerialPort(portName);
	}

	//Checking if arduino is connected or not
	if (arduino->isConnected()) {
		std::cout << "Connection established at port " << portName << endl;
	}
}

int main()
{
	//getfile();
	
	arduino = new SerialPort(portName);
	autoConnect();
	while (true)
	{
		std::cout << "\n0.change address 1.Read 2.ReadID 3.write 4.erase 5.block verify" << "\n";
		std::cin>>option[0];
		//change address
		if (option[0] == '0')                 
		{
			printf("Address= ");
			ReadIDorAddress();
			char address[3];
			std::cout << "輸入想要編輯的位置\n";
			scanf_s("%x %x %x", address,address+1,address+2);
			arduino->writeSerialPort(address, 3);
		}
		//read
		if (option[0]=='1')																	
		{
			Read();
		}
		//read id
		else if (option[0] == '2')
		{
			printf("device ID=");
			ReadIDorAddress();
		}
		//write
		else if (option[0] == '3')
		{
			std::cout << "請輸入檔案位置:\n";   
			std::cin >> address;     
			SendDataFromFile();
		}
		//erase
		else if (option[0] == '4')
		{
			arduino->writeSerialPort(option, 1);
		}
		//verify
		else if (option[0] == '5')
		{
			char returnvaule[1];
			int readResult;
			SendDataFromFile();
			do
			{
				readResult = arduino->readSerialPort(returnvaule, sizeof(returnvaule));
			} while (readResult!=1);
			if (returnvaule[0]==1)
			{
				printf("pass");
			}
			else
			{
				printf("failure");
			}
		}
	setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
	}
	/*SerialPort port(portName);
	char readBuffer[512];
	arduino->readSerialPort(readBuffer, 512);*/

}
//C:\\Users\\User\\Desktop\\example2.bin