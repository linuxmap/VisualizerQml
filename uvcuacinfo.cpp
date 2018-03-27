#include <atlcomcli.h>
#include <comutil.h>
#include <comdef.h>
#include <QDebug>
#include <wmcodecdsp.h>
#include "uvcuacinfo.h"

#pragma region class UvcUacEnumerator
UvcUacEnumerator* UvcUacEnumerator::m_instance = NULL;
UvcUacEnumerator::UvcUacEnumerator(QObject *parent)
	:QObject(parent)
	,_camNum(0)
	,_micNum(0)
{
	for (int i = 0; i < MaxCameraNum; i++)
	{
		_pCamMonikers[i] = NULL;
	}
	for (int i = 0; i < MaxMicNum; i++)
	{
		_pMicMonikers[i] = NULL;
	}
	_camsInfo.clear();
	_micsInfo.clear();
}

UvcUacEnumerator::~UvcUacEnumerator()
{
	releaseCams();
	releaseMics();
	m_instance = NULL;
}

UvcUacEnumerator *UvcUacEnumerator::getInstance()
{
	if (NULL == m_instance)
		m_instance = new UvcUacEnumerator;
	return m_instance;
}

int UvcUacEnumerator::enumDevices()
{
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = S_OK;
	/* enum  video device */
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (NULL == pCreateDevEnum)
	{
		goto FAILED;
	}

	IEnumMoniker *pEnumMoniker = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	if (NULL == pEnumMoniker)
	{
		goto FAILED;
	}
	releaseCams();
	ULONG fetched;
	while (S_OK == pEnumMoniker->Next(1, _pCamMonikers + _camNum, &fetched))
	{
		UsbDeviceInfo *pInfo=new UsbDeviceInfo;
		pInfo->type = UsbDeviceTypeUVC;
		fetchDeviceInfo(_pCamMonikers[_camNum], *pInfo);
		_camsInfo.append(pInfo);
		_camNum++;
		if (_camNum == MaxCameraNum)
			break;
	}

	pEnumMoniker->Release();
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumMoniker, 0);
	if (NULL == pEnumMoniker)
	{
		goto FAILED;
	}

	releaseMics();
	while (S_OK == pEnumMoniker->Next(1, _pMicMonikers + _micNum, &fetched))
	{
		UsbDeviceInfo *pInfo=new UsbDeviceInfo;
		pInfo->type = UsbDeviceTypeUAC;
		fetchDeviceInfo(_pMicMonikers[_micNum], *pInfo);
		_micsInfo.append(pInfo);
		_micNum++;
		if (_micNum == MaxMicNum)
			break;
	}
	pEnumMoniker->Release();
	pCreateDevEnum->Release();
	emit UVCUACEnumerateSuccessed();
	return _camNum;
FAILED:
	if (pEnumMoniker)
	{
		pEnumMoniker->Release();
		pEnumMoniker = NULL;
	}
	if (pCreateDevEnum)
	{
		pCreateDevEnum->Release();
		pCreateDevEnum = NULL;
	}
	return 0;
}

int UvcUacEnumerator::getCamNum()
{
	return _camNum;
}

int UvcUacEnumerator::getMicNum()
{
	return _micNum;
}

int UvcUacEnumerator::getCamInfo(int idx, UsbDeviceInfo &info)
{
	if (idx > _camNum)return -1;
	memcpy(&info,_camsInfo[idx],sizeof(UsbDeviceInfo));
	return 0;
}

int UvcUacEnumerator::getCamInfo(int idx, UsbDeviceInfo **ppInfo)
{
	if (idx > _camNum)
	{
		*ppInfo = NULL;
		return -1;
	}
	*ppInfo = _camsInfo[idx];
	return 0;
}

int UvcUacEnumerator::fetchDeviceInfo(IMoniker * pImonik, UsbDeviceInfo &info)
{
	if (!pImonik)return -1;
	info.vid = 0;
	info.pid = 0;
	info.pMonik = pImonik;
	IPropertyBag *pBag = NULL;
	HRESULT hr = info.pMonik->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pBag);
	if (SUCCEEDED(hr))
	{
		CComVariant va;
		hr = pBag->Read(L"FriendlyName", &va, NULL);
		if (NOERROR == hr) {
			std::string str = (_bstr_t)va.bstrVal;
			info.friendlyname = QString::fromLocal8Bit(str.data());
			SysFreeString(va.bstrVal);
		}
		pBag->Release();
	}
	else info.friendlyname.clear();

	WCHAR *wcharStr;
	hr = info.pMonik->GetDisplayName(NULL, NULL, &wcharStr);
	if (SUCCEEDED(hr))
		info.displayname = QString::fromWCharArray(wcharStr);
	else info.displayname.clear();

	////IID_IBaseFilter:IMoniker绑定的方式获取视频源Filter
	//IBaseFilter *pfilter;
	//hr = info.pMonik->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pfilter);
	//if (FAILED(hr))
	//{
	//	qDebug() << "Bind Camera moniker failed";
	//}



	return 0;
}

int UvcUacEnumerator::getMicInfo(int idx, UsbDeviceInfo &info)
{
	if (idx > _micNum)return -1;
	memcpy(&info, _micsInfo[idx], sizeof(UsbDeviceInfo));
	return 0;
}

int UvcUacEnumerator::getMicInfo(int idx, UsbDeviceInfo **ppInfo)
{
	if (idx > _micNum)
	{
		*ppInfo = NULL;
		return -1;
	}
	*ppInfo = _micsInfo[idx];
	return 0;
}

IMoniker *UvcUacEnumerator::getCamMonik(int nIndex)
{
	IMoniker *pMoniker = NULL;
	if (nIndex < _camNum)
		pMoniker = _pCamMonikers[nIndex];

	return pMoniker;
}

IMoniker * UvcUacEnumerator::getMicMonik(int nIndex)
{
	IMoniker *pMoniker = NULL;
	if (nIndex < _micNum)
		pMoniker = _pMicMonikers[nIndex];
	return pMoniker;
}


void UvcUacEnumerator::releaseCams()
{
	for (int i = 0; i < _camNum; i++)
	{
		delete _camsInfo[i];
		if (NULL != _pCamMonikers[i])
		{
			_pCamMonikers[i]->Release();
			_pCamMonikers[i] = NULL;
		}
	}
	_camsInfo.clear();
	_camNum = 0;

}

void UvcUacEnumerator::releaseMics()
{
	for (int i = 0; i < _micNum; i++)
	{
		delete _micsInfo[i];
		if (NULL != _pMicMonikers[i])
		{
			_pMicMonikers[i]->Release();
			_pMicMonikers[i] = NULL;
		}
	}
	_micsInfo.clear();
	_micNum = 0;
}

#pragma endregion

UvcUacInfo::UvcUacInfo(QObject*parent)
	:QObject(parent)
{
	connect(UvcUacEnumerator::getInstance(), SIGNAL(UVCUACEnumerateSuccessed()), this, SLOT(sltUVCUACEnumerateSuccessed()));
	UvcUacEnumerator::getInstance()->enumDevices();
}

UvcUacInfo::~UvcUacInfo()
{
}

int UvcUacInfo::uvcCount(void)
{
	return UvcUacEnumerator::getInstance()->getCamNum();
}

UsbDeviceInfo UvcUacInfo::uvcInfo(int idx)
{
	UsbDeviceInfo info;
	memset(&info, 0, sizeof(UsbDeviceInfo));
	UvcUacEnumerator::getInstance()->getCamInfo(idx, info);
	return info;
}

QStringList UvcUacInfo::uvcFriendlyNames(void)
{
	UvcUacEnumerator *e = UvcUacEnumerator::getInstance();
	int uvcCount = e->getCamNum();
	int ret;
	QStringList friendlyNames;
	friendlyNames.clear();

	for (int i = 0; i < uvcCount; i++)
	{
		UsbDeviceInfo *pInfo;
		QString fn;
		if (ret=e->getCamInfo(i, &pInfo)>=0)
		{
			fn = pInfo->friendlyname;
			friendlyNames.append(fn);
		}
	}
	return friendlyNames;
}

int UvcUacInfo::uacCount(void)
{
	return UvcUacEnumerator::getInstance()->getMicNum();
}

QStringList UvcUacInfo::uacFriendlyNames(void)
{
	UvcUacEnumerator *e = UvcUacEnumerator::getInstance();
	int uacCount = e->getMicNum();
	int ret;
	QStringList friendlyNames;
	friendlyNames.clear();
	for (int i = 0; i < uacCount; i++)
	{
		UsbDeviceInfo *pInfo;
		QString fn;
		if (ret = e->getMicInfo(i, &pInfo) >= 0)
		{
			fn = pInfo->friendlyname;
			friendlyNames.append(fn);
		}
	}
	return friendlyNames;
}

void UvcUacInfo::sltUVCUACEnumerateSuccessed(void)
{
	emit uvcCountChanged();
	emit uacCountChanged();
}
