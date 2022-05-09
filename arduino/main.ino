// 光谱仪下位机程序
// 控制DAC模块按顺序输出电压值，并同时读取光电二极管输出
// A0为光电二极管模拟输入端口
// A4为arduinoIIC SDA端口，连接DAC模块SDA口
// A5为arduinoIIC SCL端口，连接DAC模块SCL口

/*宏定义*/
#include <Wire.h>                                //arduinoIIC库
#define MCP4725_ADDR 0x60                        // DAC模块默认IIC地址
#define START_VAL 0                              //每次扫描时控制DAC输出的最小值（对应转动镜子的最小角度）
#define CORRECTING_VAL (START_VAL + END_VAL) / 2 //校准时使用的DAC输出值（对应最小角度与最大角度的一半）
#define END_VAL 4096                             //每次扫描时控制DAC输出的最大值（对应转动镜子的最大角度）

/*全局变量*/
int pdOut = A0; // pd读取模拟信号的io口
int pdRead;     // pd读取数值，最大1024

/*函数声明*/
void writeToDAC(int value); //控制IIC输出数据到DAC，输入参数为12bit数值
void scan();                //控制镜子转动扫描一次
void correcting();          //控制镜子转到中间位置便于调整光路

/*程序主体*/
void setup()
{
    Serial.begin(115200);
    Wire.begin();
}

void loop()
{

}

void scan()
{
    for (int i = START_VAL; i < END_VAL; i++)//扫描
    {
        writeToDAC(i);

        pdRead = analogRead(pdOut);

        Serial.print(i);
        Serial.print(' ');
        Serial.println(pdRead);
    }
    
    writeToDAC(START_VAL);//让镜子恢复到初始值
    delay(50);
}

void correcting()
{
    writeToDAC(CORRECTING_VAL);//让镜子固定在校准位置
}

void writeToDAC(int value)
{
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64);                 //发送更改输出电压指令
    Wire.write((value >> 4) & 255); //发送高八位数据
    Wire.write((value & 15) << 4);  //发送低四位数据
    Wire.endTransmission();
}