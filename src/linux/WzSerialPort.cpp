
#include "WzSerialPort.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#ifndef BOTHER
#define BOTHER 0010000
#endif


WzSerialPort::WzSerialPort()
{
}

WzSerialPort::~WzSerialPort()
{
}

bool WzSerialPort::open(const char* portname, int baudrate, char parity, char databit, char stopbit, char synchronizeflag)
{
    // 打开串口
    pHandle[0] = -1;
    // 以 读写、不阻塞 方式打开
    pHandle[0] = ::open(portname,O_RDWR|O_NOCTTY|O_NDELAY);
    
    // 打开失败，则打印失败信息，返回false
    if(pHandle[0] == -1)
    {
        std::cout << portname << " open failed , may be you need 'sudo' permission." << std::endl;
        return false;
    }
    
    // if(fcntl(pHandle[0],F_SETFL,FNDELAY) >= 0)//非阻塞，覆盖前面open的属性
    if (fcntl(pHandle[0], F_SETFL, 0) < 0) // 阻塞，即使前面在open串口设备时设置的是非阻塞的，这里设为阻塞后，以此为准
    {
        std::cout << "System Error." << std::endl;
        return false;
    }

    // 设置串口参数
    // 创建串口参数对象
    struct termios options;
    // 先获得串口的当前参数
    if(tcgetattr(pHandle[0],&options) < 0)
    {
        std::cout << portname << " open failed , get serial port attributes failed." << std::endl;
        return false;
    }

    // 设置波特率
    switch(baudrate)
    {
        case 4800:
            cfsetispeed(&options,B4800);
            cfsetospeed(&options,B4800);
            break;
        case 9600:
            cfsetispeed(&options,B9600);
            cfsetospeed(&options,B9600);
            break;
        case 19200:
            cfsetispeed(&options,B19200);
            cfsetospeed(&options,B19200);
            break;
        case 38400:
            cfsetispeed(&options,B38400);
            cfsetospeed(&options,B38400);
            break;
        case 57600:
            cfsetispeed(&options,B57600);
            cfsetospeed(&options,B57600);
            break;
        case 115200:
            cfsetispeed(&options,B115200);
            cfsetospeed(&options,B115200);
            break;
       case 460800:
           cfsetispeed(&options,B460800);
           cfsetospeed(&options,B460800);
           break;
       case 921600:
           cfsetispeed(&options,B921600);
           cfsetospeed(&options,B921600);
           break;
        default:
            std::cout << portname << " open failed , unkown baudrate , only support 4800,9600,19200,38400,57600,115200." << std::endl;
            return false;
    }

    // 设置校验位
    switch(parity)
    {
        // 无校验
        case 0:
            options.c_cflag &= ~PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~INPCK;//INPCK：使奇偶校验起作用
            break;
        // 设置奇校验
        case 1:
            options.c_cflag |= PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag |= PARODD;//PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;//INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP;//ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        // 设置偶校验
        case 2:
            options.c_cflag |= PARENB;//PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~PARODD;//PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;//INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP;//ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        default:
            std::cout << portname << " open failed , unkown parity ." << std::endl;
            return false;
    }

    // 设置数据位
    switch(databit)
    {
        case 5:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag &= ~CSIZE;//屏蔽其它标志位
            options.c_cflag |= CS8;
            break;
        default:
            std::cout << portname << " open failed , unkown databit ." << std::endl;
            return false;
    }

    // 设置停止位
    switch(stopbit)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;//CSTOPB：使用1位停止位
            break;
        case 2:
            options.c_cflag |= CSTOPB;//CSTOPB：使用2位停止位
            break;
        default:
            std::cout << portname << " open failed , unkown stopbit ." << std::endl;
            return false;
    }

    // 控制模式
    options.c_cflag |= CLOCAL; // 保证程序不占用串口
    options.c_cflag |= CREAD;  // 保证程序可以从串口中读取数据

    // 设置输出模式为原始输出
    options.c_oflag &= ~OPOST; // OPOST：若设置则按定义的输出处理，否则所有c_oflag失效

    // 设置本地模式为原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /*
    *ICANON：允许规范模式进行输入处理
    *ECHO：允许输入字符的本地回显
    *ECHOE：在接收EPASE时执行Backspace,Space,Backspace组合
    *ISIG：允许信号
    */

    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /*
    *BRKINT：如果设置了IGNBRK，BREAK键输入将被忽略
    *ICRNL：将输入的回车转化成换行（如果IGNCR未设置的情况下）(0x0d => 0x0a)
    *INPCK：允许输入奇偶校验
    *ISTRIP：去除字符的第8个比特
    *IXON：允许输出时对XON/XOFF流进行控制 (0x11 0x13)
    */

    // 设置等待时间和最小接受字符
    options.c_cc[VTIME] = 1; // 可以在select中设置
    options.c_cc[VMIN] = 1; // 最少读取一个字符

    // 如果发生数据溢出，只接受数据，但是不进行读操作
    tcflush(pHandle[0], TCIFLUSH);

    // 激活新配置
    if((tcsetattr(pHandle[0],TCSANOW,&options))!=0)
    {
        std::cout << portname << " open failed , can not complete set attributes ." << std::endl;
        return false;
    }

    return true;
}

void WzSerialPort::close()
{
    if(pHandle[0] != -1)
    {
        ::close(pHandle[0]);
    }
}

int WzSerialPort::send(const uint8_t *buf,int len)
{
    int sendCount = 0;
    if(pHandle[0] != -1)
    {
        // 将 buf 和 len 转换成api要求的格式
        const uint8_t *buffer = (uint8_t*)buf;
        size_t length = len;
        // 已写入的数据个数
        ssize_t tmp;

        while(length > 0)
        {
            if((tmp = write(pHandle[0], buffer, length)) <= 0)
            {
                if(tmp < 0&&errno == EINTR)
                {
                    tmp = 0;
                }
                else
                {
                    break;
                }
            }
            length -= tmp;
            buffer += tmp;
        }

        sendCount = len - length;
    }

    return sendCount;
//    int lengthSent;
//    lengthSent = write(pHandle[0], buf, len);
//    return lengthSent;
}

int WzSerialPort::receive(uint8_t *buf,int maxlen)
{
    int receiveCount = read(pHandle[0],buf,maxlen);
    if(receiveCount < 0)
    {
        receiveCount = 0;
    }
    return receiveCount;
}

int WzSerialPort::receive(uint8_t *buf, int maxlen, int timeoutms)
{
    int nfds;
//    int nread = 0 ;
//    char read_temp[256];
    fd_set readfds;
    ssize_t	nread;
    struct timeval tv{};

    tv.tv_sec = timeoutms / 1000;
    tv.tv_usec = timeoutms % 1000;
    FD_ZERO(&readfds);
    FD_SET(pHandle[0],&readfds);
//    bzero(read_temp,sizeof(read_temp));

    nfds = select(pHandle[0]+1,&readfds,nullptr,nullptr,&tv);

    if(nfds <= 0) {
        return -1;
//        printf("timeout!\r\n");
    }
    if (FD_ISSET(pHandle[0], &readfds)) {
        if ((nread = read(pHandle[0],buf,maxlen))<0){
            return -1;
        } //即使不满desire_get_len，也会返回实际读取到的数据量
        return (int)nread;
    }
    return -1;
}


