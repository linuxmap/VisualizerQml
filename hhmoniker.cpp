#include "hhmoniker.h"
#include <atlcomcli.h>
#include <comutil.h>
#include <comdef.h>
#include <QDebug>

HHMoniker* HHMoniker::m_instance = NULL;
HHMoniker::HHMoniker()
{
    m_monikerNum = 0;
    memset(m_monikers, NULL, MaxCameraNum*sizeof(int));
}

HHMoniker::~HHMoniker()
{
    releaseMonikers();
}

HHMoniker *HHMoniker::getInstance()
{
    if(NULL == m_instance)
        m_instance = new HHMoniker;

    return m_instance;
}

int HHMoniker::enumIMoniker()
{
    ICreateDevEnum *pCreateDevEnum = NULL;
    HRESULT hr = S_OK;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (NULL == pCreateDevEnum)
    {
        return 0;
    }

    IEnumMoniker *pEnumMoniker = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	if (NULL == pEnumMoniker)
    {
        pCreateDevEnum->Release();
        return 0;
    }
	
    releaseMonikers();

    ULONG fetched;
    while(S_OK == pEnumMoniker->Next(1, m_monikers+m_monikerNum, &fetched))
    {
        m_monikerNum++;
        if(m_monikerNum == MaxCameraNum)
            break;
    }

    pEnumMoniker->Release();
    pCreateDevEnum->Release();
    return m_monikerNum;
}

int HHMoniker::getIMonikerNum()
{
    return m_monikerNum;
}

bool HHMoniker::getDevInfo(int nIndex, QString &friendlyName, QString &displayName)
{
//    qDebug()<<"HHMoniker::getDevInfo";
    if(nIndex >= m_monikerNum)
        return false;

    IMoniker *pMoniker = m_monikers[nIndex];
    if(NULL == pMoniker)
        return false;

    IPropertyBag *pBag = NULL;
    HRESULT hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pBag);
    if(SUCCEEDED(hr))
    {
        CComVariant va;
        hr = pBag->Read(L"FriendlyName", &va, NULL);
        if( NOERROR == hr){
            std::string str = (_bstr_t)va.bstrVal;
            friendlyName = QString::fromStdString(str);
            SysFreeString(va.bstrVal);
        }
        pBag->Release();
    }
    else
        return false;

    WCHAR *wcharStr;
    hr = pMoniker->GetDisplayName(NULL,NULL,&wcharStr);
    if(FAILED(hr))
        return false;
    displayName = QString::fromWCharArray(wcharStr);

    qDebug()<<"displayName:"<<displayName;
    qDebug()<<"friendlyName:"<<friendlyName;
    return true;
}

IMoniker *HHMoniker::getMoniker(int nIndex)
{
    IMoniker *pMoniker = NULL;
    if(nIndex < m_monikerNum)
        pMoniker = m_monikers[nIndex];

    return pMoniker;
}

void HHMoniker::releaseMonikers()
{
    for(int i=0; i<MaxCameraNum; i++)
    {
        if(NULL != m_monikers[i])
        {
            m_monikers[i]->Release();
            m_monikers[i] = NULL;
        }
    }
    m_monikerNum = 0;
}



