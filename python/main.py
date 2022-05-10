import serial;
import tkinter;
import frame;

serialPort = "COM4"  # 串口
baudRate = 115200 # 波特率
ser = serial.Serial(serialPort, baudRate, timeout=0.5)
print("参数设置：串口=%s ，波特率=%d" % (serialPort, baudRate))

