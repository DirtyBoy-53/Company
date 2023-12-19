#ifndef CPINGTEST_H
#define CPINGTEST_H
#include <WinSock2.h>

typedef unsigned char u_char;
typedef unsigned short u_short;

typedef struct tagIPHDR
{
    u_char VIHL; // Version and IHL
    u_char TOS; // Type Of Service
    short TotLen; // Total Length
    short ID; // Identification
    short FlagOff; // Flags and Fragment Offset
    u_char TTL; // Time To Live
    u_char Protocol; // Protocol
    u_short Checksum; // Checksum
    struct in_addr iaSrc; // Internet Address - Source
    struct in_addr iaDst; // Internet Address - Destination
}IPHDR, *PIPHDR;

// ICMP Header - RFC 792
typedef struct tagICMPHDR
{
    u_char Type; // Type
    u_char Code; // Code
    u_short Checksum; // Checksum
    u_short ID; // Identification
    u_short Seq; // Sequence
    char Data; // Data
}ICMPHDR, *PICMPHDR;

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
    ICMPHDR icmpHdr;
    struct timeval echoTime;
    char cData[32];
}ECHOREQUEST, *PECHOREQUEST;

// ICMP Echo Reply
typedef struct tagECHOREPLY
{
    IPHDR ipHdr;
    ECHOREQUEST echoRequest;
    char cFiller[256];
}ECHOREPLY, *PECHOREPLY;

class CPingTest
{
public:
    CPingTest();
    static bool pingTest(const char* localip, const char* hostip);
};

#endif // CPINGTEST_H
