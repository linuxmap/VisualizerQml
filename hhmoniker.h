#ifndef HHMONIKER_H
#define HHMONIKER_H
#include <Windows.h>
#include "DShow.h"
#include <QString>
#define MaxCameraNum 5
class HHMoniker
{
private:
    HHMoniker();

public:
    ~HHMoniker();
    static HHMoniker *getInstance();
    /*
     * 遍历连接的摄像头，并返回遍历到的数目
     */
    int enumIMoniker();
    int getIMonikerNum();

    /*
     * 获取设备信息
     * nIndex: 设备在m_monikers中的索引
     * friendlyName: 人性化描述名
     * displayName: 设备信息名
     */
    bool getDevInfo(int nIndex, QString &friendlyName, QString &displayName);
    IMoniker *getMoniker(int nIndex);

private:
    void releaseMonikers();


private:
    static HHMoniker *m_instance;

    IMoniker    *m_monikers[MaxCameraNum];
    int         m_monikerNum;
};

#endif // HHMONIKER_H
