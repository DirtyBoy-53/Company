#ifndef SHAREDMEMBUFFER_H
#define SHAREDMEMBUFFER_H
#include <stdint.h>
#define MEMCOUNT 150  //3s
#define PANE_NUM        9   //窗格数量

#include <mutex>

typedef struct _mem_data_info {
    int uIdx;
    uint64_t  uCurPts;//ms
    int uLen;
}mem_data_info;

typedef struct _mem_share_info {
    int uPreHeader = 0xefefefef; //0xefefefef
    int uFlag;
    int totalSize;
    int uCurWriteIdx;
    char * pStartPtr;
    int uoffsetWrite;
    int uDataLen;
    int maxCount;
    mem_data_info mem[MEMCOUNT];
    int uPreTail;
}mem_share_info;

typedef enum _ESHARECHANNEL{
    SHARECHANNEL0 = 0,
    SHARECHANNEL1 = 1,
    SHARECHANNEL2 = 2,
    SHARECHANNEL3 = 3,
    SHARECHANNEL4 = 4,
    SHARECHANNEL5 = 5,
    SHARECHANNEL6 = 6,
    SHARECHANNEL7 = 7,
    SHARECHANNEL8 = 8,
    SHAREMAX
}ESHARECHANNEL;

class SharedMemBuffer
{
public:
    SharedMemBuffer();
    void init(ESHARECHANNEL channel,int iWrite);

    int writeFrame(char * pData, int iLen, uint64_t pts = 0);

    int readFrame(char* pBuffer, int iSize, int &iNumTrunk, uint64_t &iPts);

    int readFrame(char* pBuffer, int iSize);

    int readOneFrame(char **pBuffer);

    int readLastFrame(char **pBuffer);

    int readLastFrame(char* pBuffer, int iSize);

private:
    mem_share_info *m_pHeader;
    mem_data_info m_rTrunk;
    char * m_rDataPtr;
    uint64_t m_uCurPts;
    int m_cacheMaxFrame;

    ESHARECHANNEL m_channel;
    int m_rIdx;
    int m_totalSize;

};

#endif // SHAREDMEMBUFFER_H
