#include "CPingTest.h"
#include <QString>
#include <QDebug>
#include <QThread>
#include <chrono>    //头文件包含

#define ICMP_ECHO	8
#define ICMP_REPLY	0
struct
{
    __int64 tv_sec;
    __int64 tv_usec;
}timeval;


void gettimeofday(struct timeval* tv) {

    auto time_now = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    auto duration_in_s = std::chrono::duration_cast<std::chrono::seconds>(time_now.time_since_epoch()).count();
    auto duration_in_us = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch()).count();

    tv->tv_sec = duration_in_s;
    tv->tv_usec = duration_in_us;
}

//计算网际效验和函数 pBuf指向ICMP数据开始的地方
uint16_t checksum(uint16_t *pBuf, int iSize)
{
    uint32_t cksum=0;
    pBuf[1]=0;//checksum字段本身应该置零
    while(iSize>1)
    {
        cksum+=*pBuf++;
        iSize-=sizeof(uint16_t);
    }
    if(iSize)
    {
        cksum+=*(uint16_t*)pBuf;
    }
    cksum=(cksum>>16)+(cksum&0xffff);
    cksum+=(cksum>>16);
    return(uint16_t)(~cksum);
}

void tv_sub(struct timeval *out,struct timeval *in)
{
    if( (out->tv_usec-=in->tv_usec)<0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec-=in->tv_sec;
}

int WaitForEchoReply(SOCKET s)
{
    struct timeval Timeout;
    fd_set readfds;

    readfds.fd_count = 1;
    readfds.fd_array[0] = s;
    Timeout.tv_sec = 1;
    Timeout.tv_usec = 500000;

    return (select(1, &readfds, NULL, NULL, &Timeout));
}

CPingTest::CPingTest()
{

}
class CNetwork {
public:
    CNetwork() {
        WSAStartup(MAKEWORD(2,2), &mWsaData);
    }
    ~CNetwork() {
        WSACleanup();
    }

private:
    WSADATA mWsaData;
};

bool CPingTest::pingTest(const char* localip, const char* hostip)
{
    qDebug() << __FUNCTION__ << " " << QThread::currentThreadId();
    CNetwork network;
    if (hostip == nullptr) {
        qDebug() << "destination ip address is nullptr";
        return false;
    }

    int sockfd = 0;
    int nRet;
    int nCount;
    int iSuccess = 0;
    struct sockaddr_in addrDest;
    float spenttime;


    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        return false;
    }


    if (localip != nullptr) {
        qDebug() << "local ip address is not nullptr, will do bind operation";
        struct sockaddr_in addrLocal;
        memset(&addrLocal, 0, sizeof(sockaddr_in));
        addrLocal.sin_family = AF_INET;
        addrLocal.sin_port = htons(0);
        addrLocal.sin_addr.s_addr = inet_addr(localip);
        if (0 != bind(sockfd, (sockaddr*)&addrLocal, sizeof(sockaddr_in))) {
            qDebug() << "bind local ip " << localip << " failed";
            closesocket(sockfd);
            return false;
        }
    }

    memset(&addrDest, 0, sizeof(sockaddr_in));
    addrDest.sin_addr.s_addr = inet_addr(hostip);
    addrDest.sin_family = AF_INET;

    ECHOREQUEST echoReq;
    memset(&echoReq, 0, sizeof(echoReq));
    echoReq.icmpHdr.Type = ICMP_ECHO; // request 0; reply 8;
    echoReq.icmpHdr.Code = ICMP_REPLY;
    echoReq.icmpHdr.ID = GetCurrentProcessId();// reply 类型消息才需要。
    int Seq = 0;
    for (nRet = 0; nRet < 32; nRet++)
        echoReq.cData[nRet] = ' '+nRet;

    struct timeval tout;
    tout.tv_sec = 1;
    tout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tout, sizeof(timeval));

    nCount = 0;
    while(nCount < 3)
    {
        echoReq.icmpHdr.Seq = Seq++;
        echoReq.icmpHdr.Checksum = 0;
        gettimeofday(&echoReq.echoTime);
        echoReq.icmpHdr.Checksum = checksum((unsigned short*)&echoReq, sizeof(echoReq));
        if (sendto(sockfd, (char*)&echoReq, sizeof(echoReq), 0, (sockaddr*)&addrDest, sizeof(addrDest)) < 0)
        {
            DWORD dw = GetLastError();
            closesocket(sockfd);
            return false;
        }
        if(WaitForEchoReply(sockfd) == -1)
        {
            closesocket(sockfd);
            return false;
        }
        ECHOREPLY icmpRecv;
        int addr_len;
        addr_len = sizeof(sockaddr_in);
        if (recvfrom(sockfd, (char*)&icmpRecv, sizeof(tagECHOREPLY),
                     0, (sockaddr*)&addrDest, (int*)&addr_len) < 0) {
            closesocket(sockfd);
            return false;
        } else {
            if (icmpRecv.echoRequest.icmpHdr.Type == ICMP_REPLY) { // 8: echo reply , 0: echo request
                gettimeofday(&icmpRecv.echoRequest.echoTime);
                tv_sub(&icmpRecv.echoRequest.echoTime, &echoReq.echoTime);
                spenttime=icmpRecv.echoRequest.echoTime.tv_sec*1000+icmpRecv.echoRequest.echoTime.tv_usec*0.001;
                if (strcmp(hostip, inet_ntoa(icmpRecv.ipHdr.iaSrc))==0) {
                    iSuccess ++;
                    qDebug("Reply from %s: Bytes=%d Id_seq = %d time=%4.3fms TTL=%d",
                           inet_ntoa(icmpRecv.ipHdr.iaSrc), sizeof(icmpRecv.echoRequest),
                           icmpRecv.echoRequest.icmpHdr.Seq, spenttime,icmpRecv.ipHdr.TTL);
                } else {
                    qDebug("From %s Id_seq %d Destination Host Unreachable", inet_ntoa(icmpRecv.ipHdr.iaSrc), icmpRecv.echoRequest.icmpHdr.Seq);
                }
            }
        }
        nCount ++;
    }

    closesocket(sockfd);
    if (iSuccess >0)
        return true;
    else
        return false;
}
