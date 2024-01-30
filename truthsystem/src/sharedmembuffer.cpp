#include "sharedmembuffer.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
//#include <sys/shm.h>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QCoreApplication>

#include <queue>

#define MEMKEY 354233

#define SYNCDATA 0xef234543

typedef struct _SHARE_CHANNEL_INFO {
    int key;
    int size;
    char * ptr;
}SHARE_CHANNEL_INFO;

std::mutex g_mutex;

std::queue<SHARE_CHANNEL_INFO> g_channel_info[SHAREMAX + 1];

// static SHARE_CHANNEL_INFO g_channel_info[SHAREMAX + 1] =
//     {
//         {354233, 100*1024*1024,NULL}, //main vl
//         {354234, 100*1024*1024,NULL},  //sub vl
//         {354235, 100*1024*1024,NULL},  //main ir
//         {354236, 100*1024*1024,NULL},   //sub ir
//         {354237, 100*1024*1024,NULL},   //web h264
//         {354238, 100*1024*1024,NULL},    //y16
//         {354239, 100*1024*1024,NULL},    //x16
//         {354240, 100*1024*1024,NULL},    //y16
//         {354241, 100*1024*1024,NULL},    //y16
//     };


SharedMemBuffer::SharedMemBuffer()
{
    m_pHeader = nullptr;
    m_rTrunk.uLen = 0;
    m_rTrunk.uIdx = 0;
    m_rIdx = -1;
    m_uCurPts = 0;
    m_cacheMaxFrame = 3;
}

void SharedMemBuffer::init(ESHARECHANNEL channel,int iWrite)
{
    if(channel < 0 || channel >= SHAREMAX) {
        return;
    }
    m_channel = channel;

    std::unique_lock<std::mutex> locker(g_mutex);
    if (iWrite == 1) {
        while (!g_channel_info[m_channel].empty()) {
            SHARE_CHANNEL_INFO info = g_channel_info[m_channel].front();
            if (info.ptr) {
                delete []info.ptr;
                info.ptr = nullptr;
            }
            g_channel_info[m_channel].pop();
        }
        m_totalSize = 0;
    }

    // if(!iWrite && g_channel_info[channel].ptr) {

    //     m_rDataPtr = (char *)g_channel_info[channel].ptr + sizeof(mem_share_info);
    //     m_pHeader = (mem_share_info *) g_channel_info[channel].ptr;
    //     return;
    // }
    // if(!iWrite) {
    //     return;
    // }

    // int totalSize = g_channel_info[channel].size;
    // void * shm = nullptr;

    // shm = (void *) malloc(totalSize);

    // m_pHeader = (mem_share_info *) shm;
    // if(iWrite) {
    //     memset(shm, 0, totalSize);
    //     m_pHeader->totalSize = totalSize;
    //     m_pHeader->pStartPtr = (char *)(shm + sizeof(mem_share_info));
    //     m_pHeader->uPreHeader = SYNCDATA;
    //     m_pHeader->uPreTail = SYNCDATA;
    //     m_pHeader->uFlag = 0;
    //     m_pHeader->uCurWriteIdx = 0;
    //     m_pHeader->uoffsetWrite = 0;
    //     m_pHeader->maxCount = 0;
    //     m_pHeader->uDataLen = totalSize - sizeof(mem_share_info);
    //     g_channel_info[channel].ptr = shm;
    // }
    // m_totalSize = totalSize;
    // m_channel = channel;
    // m_rDataPtr = (char *)(shm + sizeof(mem_share_info));
    qDebug("init succ");
}

int SharedMemBuffer::writeFrame(char * pData, int iLen, uint64_t pts)
{
    std::unique_lock<std::mutex> locker(g_mutex);
    if (g_channel_info[m_channel].size() > 100 || m_totalSize > 100) {
        SHARE_CHANNEL_INFO infoX = g_channel_info[m_channel].front();
        g_channel_info[m_channel].pop();
        if (infoX.ptr) {
            delete []infoX.ptr;
            infoX.ptr = nullptr;
        }
    }

    SHARE_CHANNEL_INFO info;
    info.ptr = new char[iLen + 1];
    memcpy(info.ptr, pData, iLen);
    info.size = iLen;
    g_channel_info[m_channel].push(info);

    return 0;

    // if(!m_pHeader
    //         || m_pHeader->uPreHeader != SYNCDATA
    //         || m_pHeader->uPreTail != SYNCDATA) {
    //     qInfo("Init err or mem error %x %x", m_pHeader->uPreHeader,m_pHeader->uPreTail);
    //     return 0;
    // }
    // int iCurIdx = m_pHeader->uCurWriteIdx;
    // int iCurPos = m_pHeader->uoffsetWrite;
    // if(iCurPos + iLen >= m_pHeader->uDataLen) {
    //     iCurPos = 0;
    // }
    // m_pHeader->mem[iCurIdx].uIdx = iCurPos;
    // m_pHeader->mem[iCurIdx].uLen = iLen;
    // if(m_pHeader->maxCount <= 0) {
    //     m_pHeader->maxCount = (m_pHeader->totalSize/iLen) > MEMCOUNT? MEMCOUNT: (m_pHeader->totalSize/iLen) ;
    // }
    // if(pts > 0) {
    //     m_pHeader->mem[iCurIdx].uCurPts = pts;
    // }else {
    //     m_pHeader->mem[iCurIdx].uCurPts = QDateTime::currentMSecsSinceEpoch();
    // }
    // memcpy(m_pHeader->pStartPtr + iCurPos, pData, iLen);
    // m_pHeader->uFlag = 1;
    // m_pHeader->uoffsetWrite = iCurPos + iLen;
    // iCurIdx ++;
    // iCurIdx = iCurIdx % m_pHeader->maxCount;
    // m_pHeader->uCurWriteIdx = iCurIdx;
    // m_pHeader->uFlag = 0;
    // return iLen;
}

int SharedMemBuffer::readFrame(char* pBuffer, int iSize, int &iNumTrunk, uint64_t &iPts)
{
//     int rLen;
//     if(!m_pHeader
//             || m_pHeader->uPreHeader != SYNCDATA
//             || m_pHeader->uPreTail != SYNCDATA ) {
//         qInfo("Init err or mem error ");
//         return 0;
//     }
//     if(m_pHeader->uFlag == 1) {
//         return 0;
//     }
//     if(m_pHeader->maxCount <= 0) {
//         return 0;
//     }
//     qInfo("2. Start read Frame");
//     if(m_rTrunk.uLen > 0) {
        
//         m_pHeader->uFlag = 1;
//         rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//         iNumTrunk = m_rTrunk.uLen - rLen;
//         qInfo("2.1 Start read Frame");
//         memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//         m_rTrunk.uLen = iNumTrunk;
//         m_rTrunk.uIdx += rLen;
//         iPts = m_rTrunk.uCurPts;
//         m_uCurPts = m_rTrunk.uCurPts;
        
//         m_pHeader->uFlag = 0;
//         qInfo("2.2 Start read Frame");
// //        qInfo("read trunk %d uCurPts %u iNumTrunk %d iSize %d ",rLen, m_rTrunk.uCurPts, iNumTrunk, iSize);
//         return rLen;
//     }
//     int iCurW = m_pHeader->uCurWriteIdx;

//     int iNextW = (iCurW - 1 + m_pHeader->maxCount) % m_pHeader->maxCount;
//     mem_data_info mLst = m_pHeader->mem[iNextW];
//     uint64_t iNextPts = QDateTime::currentMSecsSinceEpoch();
//     if(mLst.uLen > 0 ) {
//         iNextPts = mLst.uCurPts;
//     }
//     if(m_uCurPts + 2000 < iNextPts) {
//         qInfo("recv too slow  ,limit 500ms,reset");
//         m_rIdx = -1;
//     }

//     qInfo("2.3 Start read Frame");
//     if(m_rIdx < 0) {
//         if(iCurW == 0) {
//             m_rIdx = m_pHeader->maxCount - 1;
//         }else {
//             m_rIdx = iCurW - 1;
//         }
//     }else if(m_rIdx < iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = iCurW - m_cacheMaxFrame;
//         }
//     }else if(m_rIdx >iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < m_pHeader->maxCount + iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = (iCurW - m_cacheMaxFrame + m_pHeader->maxCount)%m_pHeader->maxCount;
//         }
//     }

//     qInfo("2.3.1 r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//     if(m_rIdx == iCurW) {
//         m_uCurPts = iNextPts;
//         return 0;
//     }

//     qInfo("2.3.2 r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//     if(m_pHeader->mem[m_rIdx].uLen <= 0) {
//         m_uCurPts = iNextPts;
//         return 0;
//     }
//     m_pHeader->uFlag = 1;
//     m_rTrunk = m_pHeader->mem[m_rIdx];
//     rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//     iNumTrunk = m_rTrunk.uLen - rLen;
//     qInfo("2.4 Start read Frame");
//     memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//     qInfo("2.5 Start read Frame");
//     m_rTrunk.uLen = iNumTrunk;
//     m_rTrunk.uIdx += rLen;
//     m_rIdx ++;
//     iPts = m_rTrunk.uCurPts;
//     m_rIdx = m_rIdx % m_pHeader->maxCount;
//     m_uCurPts = m_rTrunk.uCurPts;
//     m_pHeader->uFlag = 0;
// 	qInfo("2. End read %d uCurPts %u pos %u iNumTrunk %d iSize %d",rLen,m_rTrunk.uCurPts, m_rTrunk.uIdx, iNumTrunk, iSize);
//     return rLen;
}

int SharedMemBuffer::readFrame(char* pBuffer, int iSize)
{
    std::unique_lock<std::mutex> locker(g_mutex);
    if (g_channel_info[m_channel].empty()) {
        return 0;
    }

    SHARE_CHANNEL_INFO info = g_channel_info[m_channel].front();

    memcpy(pBuffer, info.ptr, info.size);
    int size = info.size;

    g_channel_info[m_channel].pop();
    if (info.ptr) {
        delete []info.ptr;
        info.ptr = nullptr;
    }

    return size;
    // int iTrunk;
    // uint64_t iPts;
    // return readFrame(pBuffer, iSize, iTrunk, iPts);
}

int SharedMemBuffer::readOneFrame(char **pBuffer)
{
//     int rLen;
//     int iSize = 1024 * 1024;
//     int iNumTrunk = 0;
//     uint64_t iPts = 0;
//     if(!m_pHeader
//             || m_pHeader->uPreHeader != SYNCDATA
//             || m_pHeader->uPreTail != SYNCDATA ) {
//         qInfo("read Init err or mem error ");
//         return 0;
//     }
//     if(m_pHeader->uFlag == 1) {
//         return 0;
//     }
//     if(m_rTrunk.uLen > 0) {
//         rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//         iNumTrunk = m_rTrunk.uLen - rLen;
//         memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//         m_rTrunk.uLen = iNumTrunk;
//         m_rTrunk.uIdx += rLen;
//         iPts = m_rTrunk.uCurPts;
//         m_uCurPts = m_rTrunk.uCurPts;
//         return rLen;
//     }
//     int iCurW = m_pHeader->uCurWriteIdx;
//     if(m_rIdx < 0) {
//         if(iCurW == 0) {
//             m_rIdx = m_pHeader->maxCount - 1;
//         }else {
//             m_rIdx = iCurW - 1;
//         }
//     }else if(m_rIdx < iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = iCurW - m_cacheMaxFrame;
//         }
//     }else if(m_rIdx >iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < m_pHeader->maxCount + iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = (iCurW - m_cacheMaxFrame + m_pHeader->maxCount)%m_pHeader->maxCount;
//         }
//     }
//     if(m_rIdx == iCurW) {
//         return 0;
//     }

//     if(m_pHeader->mem[m_rIdx].uLen <= 0) {
//         return 0;
//     }
//     m_rTrunk = m_pHeader->mem[m_rIdx];
//     rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//     iNumTrunk = m_rTrunk.uLen - rLen;
//     *pBuffer = m_rDataPtr + m_rTrunk.uIdx;
// //    memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//     m_rTrunk.uLen = iNumTrunk;
//     m_rTrunk.uIdx += rLen;
//     m_rIdx ++;
//     iPts = m_rTrunk.uCurPts;
//     m_rIdx = m_rIdx % m_pHeader->maxCount;
//     m_uCurPts = m_rTrunk.uCurPts;
//     return rLen;
}

int SharedMemBuffer::readLastFrame(char **pBuffer)
{
    // if (g_channel_info[m_channel].empty()) {
    //     return 0;
    // }

    // int size = 0;
    // {
    //     std::unique_lock<std::mutex> locker(g_mutex);
    //     SHARE_CHANNEL_INFO info = g_channel_info[m_channel].front();
    //     memcpy(pBuffer, info.ptr.get(), info.size);
    //     size = info.size;
    // }

    // return size;
    // int rLen;

    // if(!m_pHeader
    //         || m_pHeader->uPreHeader != SYNCDATA
    //         || m_pHeader->uPreTail != SYNCDATA ) {
    //     qInfo("read Init err or mem error ");
    //     init(m_channel, 0);
    //     return 0;
    // }
    // while(m_pHeader->uFlag == 1) {
    //     QCoreApplication::processEvents();
    //     usleep(1 * 50);
    // }

    // int iCurW = m_pHeader->uCurWriteIdx;
    // if(m_rIdx < 0) {
    //     if(iCurW == 0) {
    //         m_rIdx = m_pHeader->maxCount - 1;
    //     }else {
    //         m_rIdx = iCurW - 1;
    //     }
    // }
    // if(m_rIdx == iCurW) {
    //     return 0;
    // }

    // if(m_pHeader->mem[m_rIdx].uLen <= 0) {
    //     return 0;
    // }
    // m_rTrunk = m_pHeader->mem[m_rIdx];
    // rLen = m_rTrunk.uLen ;
    // *pBuffer = m_rDataPtr + m_rTrunk.uIdx;

    // m_rIdx = -1;
    // m_uCurPts = m_rTrunk.uCurPts;
    // return rLen;
}


int SharedMemBuffer::readLastFrame(char* pBuffer, int iSize)
{
    std::unique_lock<std::mutex> locker(g_mutex);
    if (g_channel_info[m_channel].empty()) {
        return 0;
    }

    SHARE_CHANNEL_INFO info = g_channel_info[m_channel].back();
    memcpy(pBuffer, info.ptr, info.size);
    int size = info.size;
    return size;

//     int rLen;
//     int iNumTrunk = 0;
//     int iPts = 0;
//     if(!m_pHeader
//             || m_pHeader->uPreHeader != SYNCDATA
//             || m_pHeader->uPreTail != SYNCDATA ) {
//         qInfo("read Init err or mem error ");
//         return 0;
//     }

//     if(m_pHeader->uFlag == 1) {
//         return 0;
//     }

//     // while(m_pHeader->uFlag == 1) {
//     //     QCoreApplication::processEvents();
//     //     usleep(1 * 50);
//     // }
//     qInfo("1. Start read last frame...");

//     if(m_rTrunk.uLen > 0) {
//         rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//         iNumTrunk = m_rTrunk.uLen - rLen;
//         memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//         m_rTrunk.uLen = iNumTrunk;
//         m_rTrunk.uIdx += rLen;
//         iPts = m_rTrunk.uCurPts;
//         m_uCurPts = m_rTrunk.uCurPts;
//         return rLen;
//     }
//     qInfo("1.1 Start read last frame...");
//     int iCurW = m_pHeader->uCurWriteIdx;
//     if(m_rIdx < 0) {
//         if(iCurW == 0) {
//             m_rIdx = MEMCOUNT - 1;
//         }else {
//             m_rIdx = iCurW - 1;
//         }
//     }else if(m_rIdx < iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = iCurW - m_cacheMaxFrame;
//         }
//     }else if(m_rIdx >iCurW) {
//         if(m_rIdx + m_cacheMaxFrame < MEMCOUNT + iCurW) {
//             qInfo("r idx reset %d  to %d", m_rIdx, iCurW - m_cacheMaxFrame);
//             m_rIdx = (iCurW - m_cacheMaxFrame + MEMCOUNT)%MEMCOUNT;
//         }
//     }
//     if(m_rIdx == iCurW) {
//         return 0;
//     }
//     qInfo("1.2 Start read last frame...");

//     if(m_pHeader->mem[m_rIdx].uLen <= 0) {
//         return 0;
//     }
//     m_rTrunk = m_pHeader->mem[m_rIdx];
//     rLen = iSize > m_rTrunk.uLen ? m_rTrunk.uLen : iSize;
//     iNumTrunk = m_rTrunk.uLen - rLen;
//     qInfo("1.3 Start read last frame...");
//     memcpy(pBuffer, m_rDataPtr + m_rTrunk.uIdx, rLen);
//     m_rTrunk.uLen = iNumTrunk;
//     m_rTrunk.uIdx += rLen;
//     m_rIdx = -1;
//     iPts = m_rTrunk.uCurPts;
// //    m_rIdx = m_rIdx % MEMCOUNT;
//     m_uCurPts = m_rTrunk.uCurPts;
//     qInfo("1. End read last frame over...");
//     return rLen;
}
