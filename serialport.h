/*****************
作者：王瑞龙
描述：串口通信实验声明
参考文档：微软官方文档: https://docs.microsoft.com/zh-cn/
*****************/

#ifndef SERIALPORT
#define SERIALPORT

#include <string>
#include <WinSock2.h>
#include <windows.h>

class SerialPort
{
private:
    int pHandle[16];
    char synchronizeFlag; //(同步、异步,仅适用与windows): 0为异步，1为同步

public:
    /*
        打开串口,成功返回true，失败返回false
        portName(串口名): 在Windows下是"COM1""COM2"等
        baudRate(波特率): 9600、19200、38400、43000、56000、57600、115200 
        parity(校验位): 0为无校验，1为奇校验，2为偶校验，3为标记校验（仅适用于windows)
        dataBit(数据位): 4-8(windows)通常为8位
        stopBit(停止位): 1为1位停止位，2为2位停止位,3为1.5位停止位
        synchronizeFlag(同步、异步,仅适用与windows): 0为异步，1为同步
    */
    bool Open(const char *portName, int bandRate, char parity, char dataBit, char stopBit, char synsynchronizeflag = 1);

    bool Close();

    int Send(const void *buf, int length);

    int Receive(void *buf, int maxLength);
};

#endif