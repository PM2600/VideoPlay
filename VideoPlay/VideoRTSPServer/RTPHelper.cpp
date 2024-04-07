#include "RTPHelper.h"
#include <Windows.h>
#define RTP_MAX_SIZE 1300

int RTPHelper::SendMediaFrame(EBuffer& frame)
{
    RTPFrame rtpframe;
    size_t frame_size = frame.size();
    int sepsize = GetFrameSepSize(frame);
    frame_size -= sepsize;
    BYTE* pFrame = sepsize + (BYTE*)frame;


    if (frame_size > RTP_MAX_SIZE) { //分片
        BYTE nalu = pFrame[0] & 0x1F;
        size_t restsize = frame_size % RTP_MAX_SIZE;
        size_t count = frame_size / RTP_MAX_SIZE;
        for (size_t i = 0; i < count; i++) {
            rtpframe.m_pyload.resize(RTP_MAX_SIZE);
            ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28; //0110 0000 | 0001 1100
            ((BYTE*)rtpframe.m_pyload)[1] = nalu;      //0000 0000 中间

            if(i == 0)
                ((BYTE*)rtpframe.m_pyload)[1] = 0x80 | ((BYTE*)rtpframe.m_pyload)[1]; //开始1000 0000 & 0001 1111        
            else if ((restsize == 0) && (i == count - 1))
                ((BYTE*)rtpframe.m_pyload)[1] = 0x40 | ((BYTE*)rtpframe.m_pyload)[1]; //结束0100 0000

            memcpy(2 + (BYTE*)rtpframe.m_pyload, pFrame + RTP_MAX_SIZE * i, RTP_MAX_SIZE);
            //TODO 发送数据包
        }
    
        if (frame_size % RTP_MAX_SIZE != 0) {
            //处理尾巴
            ((BYTE*)rtpframe.m_pyload)[1] = 0x40 | ((BYTE*)rtpframe.m_pyload)[1]; //结束0100 0000
            //TODO 发送数据包
        }
    }
    else {  //小包发送
        rtpframe.m_pyload.resize(frame.size() - sepsize);
        memcpy(rtpframe.m_pyload, frame, frame.size() - sepsize);
        //TODO:处理rtp的header，序号累加，时间戳计算出来
        //发送后，加入休眠，等待发送完成，控制发送速度
        Sleep(1000 / 30);
    }

    return 0;
}

int RTPHelper::GetFrameSepSize(EBuffer& frame)
{
    BYTE buf[] = { 0,0,0,1 };
    if (memcmp(frame, buf, 4) == 0)
        return 4;
    return 3;
}