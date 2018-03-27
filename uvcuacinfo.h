#ifndef UVCUACENUMERATOR_H
#define UVCUACENUMERATOR_H

#include "DShow.h"
#include <QString>
#include <QtCore>
#include <QObject> 
#include <QMetaType> 

#define MaxCameraNum 5
#define MaxMicNum    10
typedef enum
{
	UsbDeviceTypeUVC = 0,
	UsbDeviceTypeUAC
}UsbDeviceType;

typedef struct
{
	UsbDeviceType type;
	int vid;
	int pid;
	QString friendlyname;
	QString displayname;
	IMoniker    *pMonik;
}UsbDeviceInfo;
Q_DECLARE_METATYPE(UsbDeviceInfo)
class UvcUacEnumerator:public QObject
{
	Q_OBJECT
private:
	UvcUacEnumerator(QObject *parent=nullptr);
public:
	~UvcUacEnumerator();
	static UvcUacEnumerator *getInstance();
	int enumDevices();
	int getCamNum();
	int getMicNum();
	int getCamInfo(int idx, UsbDeviceInfo &info);
	int getCamInfo(int idx, UsbDeviceInfo **ppInfo);
	int getMicInfo(int idx, UsbDeviceInfo &info);
	int getMicInfo(int idx, UsbDeviceInfo **ppInfo);
	IMoniker *getCamMonik(int nIndex);
	IMoniker *getMicMonik(int nIndex);
signals:
	void UVCUACEnumerateSuccessed();
private:
	int fetchDeviceInfo(IMoniker * pImonik, UsbDeviceInfo &info);
	void releaseCams();
	void releaseMics();
private:
	static UvcUacEnumerator *m_instance;
	IMoniker    *_pCamMonikers[MaxCameraNum];
	IMoniker    *_pMicMonikers[MaxMicNum];
	IBaseFilter *_pCamFilters[MaxCameraNum];
	int         _micNum;
	int         _camNum;
	QList<UsbDeviceInfo *> _camsInfo;
	QList<UsbDeviceInfo *> _micsInfo;

};
class UvcUacInfo :public QObject
{
	Q_OBJECT
	Q_PROPERTY(int uvcCount READ uvcCount NOTIFY uvcCountChanged)
	Q_PROPERTY(QStringList uvcFriendlyNames READ uvcFriendlyNames NOTIFY uvcCountChanged)
	Q_PROPERTY(int uacCount READ uacCount NOTIFY uacCountChanged)
	Q_PROPERTY(QStringList uacFriendlyNames READ uacFriendlyNames NOTIFY uacCountChanged)
public:
	UvcUacInfo(QObject*parent=nullptr);
	~UvcUacInfo();
	int uvcCount(void);
	UsbDeviceInfo uvcInfo(int idx);
	QStringList uvcFriendlyNames(void);
	int uacCount(void);
	QStringList uacFriendlyNames(void);

signals:
	void uvcCountChanged();
	void uacCountChanged();
public slots:
	void sltUVCUACEnumerateSuccessed(void);
private:
	Q_DISABLE_COPY(UvcUacInfo)
};
//QML_DECLARE_TYPE(UvcUacInfo)
#endif
