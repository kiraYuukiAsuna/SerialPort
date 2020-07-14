/*****************
作者：王瑞龙
描述：串口通信实验测试
参考文档：微软官方文档: https://docs.microsoft.com/zh-cn/
*****************/
#include "serialport.h"
#include <iostream>
struct spInfo
{
    char portName[8];
    int bandRate;
    char parity;
    char dataBit;
    char stopBit;
    char synchronizeFlag;
};

int main(int argc, char *argv[])
{
    using namespace std;
    SerialPort sp;
    spInfo spInfo;
    char sendbuf[512];
    char revbuf[512];
    cout << "输入要打开的端口：" << endl;
    cin >> spInfo.portName;
    cout << "输入波特率：" << endl;
    cin >> spInfo.bandRate;
    spInfo.parity = 0;
    spInfo.dataBit = 8;
    spInfo.stopBit = 1;
    spInfo.synchronizeFlag = 1;
    if (!sp.Open(spInfo.portName, spInfo.bandRate, spInfo.parity, spInfo.dataBit, spInfo.stopBit, spInfo.synchronizeFlag))
    {
        cout << "打开端口失败！";
        cin.get();
        cin.get();
        return -1;
    }
    while (true)
    {
        memset(&sendbuf, 0, sizeof(sendbuf));
        memset(&revbuf, 0, sizeof(revbuf));
        cout << "端口打开成功" << endl
             << "输入要发送的数据:";
        cin >> sendbuf;
        sp.Send(sendbuf, strlen(sendbuf));
        cout << "发送成功!" << endl;

        sp.Receive(revbuf, sizeof(revbuf));
        cout << "接收到数据：" << revbuf << endl;
    }
    return 0;
}