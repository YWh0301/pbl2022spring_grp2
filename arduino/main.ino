// 光谱仪下位机程序
// 控制DAC模块按顺序输出电压值，并同时读取光电二极管输出
// A0为光电二极管模拟输入端口
// A4为arduinoIIC SDA端口，连接DAC模块SDA口
// A5为arduinoIIC SCL端口，连接DAC模块SCL口

/*宏定义*/
#include <Wire.h> //arduinoIIC库

#define MCP4725_ADDR 0x60 // DAC模块默认IIC地址

#define START_VAL 0                              //每次扫描时控制DAC输出的最小值（对应转动镜子的最小角度，设置成0）
#define CORRECTING_VAL (START_VAL + END_VAL) / 2 //校准时使用的DAC输出值（对应最小角度与最大角度的一半）
#define END_VAL 4096                             //每次扫描时控制DAC输出的最大值（对应转动镜子的最大角度，可以更改）

#define SCAN_COMMAND 2       //上位机指令为扫描
#define CORRECTION_COMMAND 1 //上位机指令为校准
#define STOP_COMMAND 0       //上位机指令为停止

//表示当前扫描镜子的位置，用于实现scan()函数中的往复扫描；每次程序改变镜子位置都需要更改mirrorstatus变量
#define MAX_ANGLE 2     //最大位置
#define MINIMUM_ANGLE 0 //最小位置
#define OTHER_ANGLE 1   //其他位置

/*全局变量*/
int pdOut = A0;                   // pd读取模拟信号的io口
int pdRead;                       // pd读取数值，最大1024
int command = STOP_COMMAND;       //存储上位机输入的指令代码
int mirrorstatus = MINIMUM_ANGLE; //存储扫描到了什么位置：MINIMUM_ANGLE代表最小位置，MAX_ANGLE代表最大位置，OTHER_ANGLE代表其他位置；每次程序改变镜子位置都需要更改mirrorstatus变量；

/*函数声明*/
void writeToDAC(int value); //控制IIC输出数据到DAC，输入参数为12bit数值
void scan();                //控制镜子转动扫描一次；根据当前镜子位置决定扫描方向，实现往复扫描
void correcting();          //控制镜子转到中间位置便于调整光路

/*程序主体*/
void setup()
{
    Serial.begin(115200);
    Wire.begin();
    writeToDAC(0);
}

void loop()
{
    while (Serial.available()) //检查串口缓存区有没有输入新的指令
    {
        command = Serial.read();
    }
    switch (command) //根据已有的指令运行，执行相应操作
    {
    case SCAN_COMMAND:
        scan();
        break;
    case CORRECTION_COMMAND:
        correcting();
        break;
    case STOP_COMMAND:
    default:
        writeToDAC(0);
        break;
    }
}

void scan() //控制镜子转动扫描一次；根据当前镜子位置决定扫描方向，实现往复扫描
{
    switch (mirrorstatus) //根据mirrorstatus变量中的结果来判断当前镜子所处的位置，并决定扫描方向，实现往复扫描，节约时间
    {
    case MINIMUM_ANGLE:
        for (int i = START_VAL; i < END_VAL; i++) //扫描
        {
            writeToDAC(i);

            pdRead = analogRead(pdOut);

            Serial.print(i); //串口输出结果
            Serial.print(' ');
            Serial.println(pdRead);
        }
        mirrorstatus = MAX_ANGLE;
        break;
    case MAX_ANGLE:
        for (int i = END_VAL - 1; i >= START_VAL; i--) //如果镜子在最大角度就倒着扫描
        {
            writeToDAC(i);

            pdRead = analogRead(pdOut);

            Serial.print(i); //串口输出结果
            Serial.print(' ');
            Serial.println(pdRead);
        }
        mirrorstatus = MINIMUM_ANGLE;
        break;
    case OTHER_ANGLE:
        writeToDAC(START_VAL); //先把镜子转到最小角度，等待到位后开始扫描
        mirrorstatus = MINIMUM_ANGLE;
        delay(50);
        scan();
        break;
    }
}

void correcting() //控制镜子转到中间位置便于调整光路
{
    writeToDAC(CORRECTING_VAL); //让镜子固定在校准位置
    mirrorstatus = OTHER_ANGLE; //更改变量记录镜子位置
}

void writeToDAC(int value) //控制IIC输出数据到DAC，输入参数为12bit数值
{
    Wire.beginTransmission(MCP4725_ADDR);
    Wire.write(64);                 //发送更改输出电压指令
    Wire.write((value >> 4) & 255); //发送高八位数据
    Wire.write((value & 15) << 4);  //发送低四位数据
    Wire.endTransmission();
}