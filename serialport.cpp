/*****************
作者：王瑞龙
描述：串口通信实验实现
参考文档：微软官方文档: https://docs.microsoft.com/zh-cn/
*****************/

#include "serialport.h"

bool SerialPort::Open(const char *portName,
                      int baudRate,
                      char parity,
                      char dataBit,
                      char stopBit,
                      char synchronizeFlag)
{
    this->synchronizeFlag = synchronizeFlag;
    HANDLE com = NULL;
    if (this->synchronizeFlag)
    {
        //同步方式
        com = CreateFileA(portName,                     //串口名
                          GENERIC_READ | GENERIC_WRITE, //支持读写
                          0,                            //独占方式，串口不支持共享
                          NULL,                         //安全属性指针，默认值为NULL
                          OPEN_EXISTING,                //打开现有串口文件
                          0,                            //同步方式
                          NULL);                        //用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
    }
    else
    {
        //异步方式
        com = CreateFileA(portName,                     //串口名
                          GENERIC_READ | GENERIC_WRITE, //支持读写
                          0,                            //独占方式，串口不支持共享
                          NULL,                         //安全属性指针，默认值为NULL
                          OPEN_EXISTING,                //打开现有串口文件
                          FILE_FLAG_OVERLAPPED,         //同步方式
                          NULL);                        //用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
    }
    if (com == (HANDLE)-1)
    {
        return false; //打开失败
    }
    if (!SetupComm(com, 1024, 1024))
    {                 //配置缓冲区大小
        return false; //配置缓冲区失败
    }

    //config
    DCB p;
    memset(&p, 0, sizeof(p));
    p.DCBlength = sizeof(p);
    p.BaudRate = baudRate;
    p.ByteSize = dataBit;

    switch (parity) //校验
    {
    case 0:
        p.Parity = NOPARITY; //无校验
        break;
    case 1:
        p.Parity = ODDPARITY; //奇校验
        break;
    case 2:
        p.Parity = EVENPARITY; //偶校验
        break;
    case 3:
        p.Parity = MARKPARITY; //标记校验
        break;
    }

    switch (stopBit) //停止位
    {
    case 1:
        p.StopBits = ONESTOPBIT; //1位停止位
        break;
    case 2:
        p.StopBits = TWOSTOPBITS; //2位停止位
        break;
    case 3:
        p.StopBits = ONE5STOPBITS; //1.5位停止位
        break;
    }

    if (!SetCommState(com, &p))
    {
        return false; //设置参数失败
    }

    //超时处理
    //总超时=时间系数*读或写的字符数+时间常量
    COMMTIMEOUTS timeOuts;
    timeOuts.ReadIntervalTimeout = 1000;        //读间隔超时
    timeOuts.ReadTotalTimeoutMultiplier = 500;  //读时间系数
    timeOuts.ReadTotalTimeoutConstant = 5000;   //读时间常量
    timeOuts.WriteTotalTimeoutMultiplier = 500; // 写时间系数
    timeOuts.WriteTotalTimeoutConstant = 2000;  //写时间常量
    SetCommTimeouts(com, &timeOuts);

    PurgeComm(com, PURGE_TXCLEAR | PURGE_RXCLEAR); //清空串口缓冲区
    memcpy(pHandle, &com, sizeof(com));            //保存句柄
    return true;
}

bool SerialPort::Close()
{
    HANDLE com = *(HANDLE *)pHandle;
    if (!CloseHandle(com))
    {
        return false;
    }
    return true;
}

int SerialPort::Send(const void *buf, int length)
{
    HANDLE com = *(HANDLE *)pHandle;

    if (this->synchronizeFlag)
    {
        // 同步方式
        DWORD dwBytesWrite = length;                //成功写入的数据字节数
        BOOL bWriteState = WriteFile(com,           //串口句柄
                                     buf,           //数据首地址
                                     dwBytesWrite,  //要发送的数据长度字节数
                                     &dwBytesWrite, //用来接收返回成功发送的数据字节数
                                     NULL);         //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteState)
        {
            return 0;
        }
        return dwBytesWrite;
    }
    else
    {
        //异步方式
        DWORD dwBytesWrite = length; //成功写入的数据字节数
        DWORD dwErrorFlags;          //错误标志
        COMSTAT comStat;             //通讯状态
        OVERLAPPED m_osWrite;        //异步输入输出结构体
                                     //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osWrite, 0, sizeof(m_osWrite));
        m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, "WriteEvent");
        ClearCommError(com, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        BOOL bWriteStat = WriteFile(com,              //串口句柄
                                    buf,              //数据首地址
                                    dwBytesWrite,     //要发送的数据字节数
                                    &dwBytesWrite,    //DWORD*，用来接收返回成功发送的数据字节数
                                    &m_osWrite);      //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在写入
            {
                WaitForSingleObject(m_osWrite.hEvent, 1000); //等待写入事件1秒钟
            }
            else
            {
                ClearCommError(com, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osWrite.hEvent);                //关闭并释放hEvent内存
                return 0;
            }
        }
        return dwBytesWrite;
    }
}

int SerialPort::Receive(void *buf, int maxlen)
{
    HANDLE com = *(HANDLE *)pHandle;

    if (this->synchronizeFlag)
    {
        //同步方式
        DWORD wCount = maxlen;             //成功读取的数据字节数
        BOOL bReadStat = ReadFile(com,     //串口句柄
                                  buf,     //数据首地址
                                  wCount,  //要读取的数据最大字节数
                                  &wCount, //DWORD*,用来接收返回成功读取的数据字节数
                                  NULL);   //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            return 0;
        }
        return wCount;
    }
    else
    {
        //异步方式
        DWORD wCount = maxlen; //成功读取的数据字节数
        DWORD dwErrorFlags;    //错误标志
        COMSTAT comStat;       //通讯状态
        OVERLAPPED m_osRead;   //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osRead, 0, sizeof(m_osRead));
        m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, "ReadEvent");

        ClearCommError(com, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        if (!comStat.cbInQue)
            return 0; //如果输入缓冲区字节数为0，则返回false

        BOOL bReadStat = ReadFile(com,        //串口句柄
                                  buf,        //数据首地址
                                  wCount,     //要读取的数据最大字节数
                                  &wCount,    //DWORD*,用来接收返回成功读取的数据字节数
                                  &m_osRead); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在读取中
            {
                //GetOverlappedResult函数的最后一个参数设为TRUE
                //函数会一直等待，直到读操作完成或由于错误而返回
                GetOverlappedResult(com, &m_osRead, &wCount, TRUE);
            }
            else
            {
                ClearCommError(com, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osRead.hEvent);                 //关闭并释放hEvent的内存
                return 0;
            }
        }
        return wCount;
    }
}
