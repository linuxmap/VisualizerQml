#include "uvccontroler.h"
#include <ksproxy.h>
#include <ksmedia.h>

#define CamPropertyCnt   10
#define CamControlCnt   7

UvcControlerThread::UvcControlerThread(UvcControler *controler, QObject*parent)
	:QThread(parent)
	, _controler(controler)
{
	_cmdList.clear();
}
UvcControlerThread::~UvcControlerThread()
{
	if (isRunning()) {
		_run = false;
		wait(1000);
	}

}
void UvcControlerThread::addCmd(UvcControlCmd cmd)
{
	_cmdListLocker.lock();
	_cmdList.append(cmd);
	_cmdListLocker.unlock();
	if (!isRunning())start();
}

UvcControlerThread::UvcControlCmd UvcControlerThread::takeCmd(void)
{
	UvcControlCmd cmd;
	_cmdListLocker.lock();
	if(!_cmdList.isEmpty())
		cmd = _cmdList.takeFirst();
	_cmdListLocker.unlock();
	return cmd;
}

void UvcControlerThread::run()
{
	qint32 delay = 1000;
	_run = true;
	_repeatCmdList.clear();
	do
	{
		while (!_cmdList.isEmpty()|| !_repeatCmdList.isEmpty())
		{
			if (!_cmdList.isEmpty())
			{
				UvcControlCmd cmd;
				cmd = takeCmd();
				switch (cmd)
				{
				case UvcControlZoomStop:
					if (_repeatCmdList.contains(UvcControlZoomIn))
						_repeatCmdList.removeAt(_repeatCmdList.indexOf(UvcControlZoomIn));
					if (_repeatCmdList.contains(UvcControlZoomOut))
						_repeatCmdList.removeAt(_repeatCmdList.indexOf(UvcControlZoomOut));

					break;
				case UvcControlZoomIn:
					if (_repeatCmdList.contains(UvcControlZoomIn))break;
					else 
					{
						if (_repeatCmdList.contains(UvcControlZoomOut))
							_repeatCmdList.removeAt(_repeatCmdList.indexOf(UvcControlZoomOut));
						_repeatCmdList.append(UvcControlZoomIn);
					}
					break;
				case UvcControlZoomOut:
					if (_repeatCmdList.contains(UvcControlZoomOut))break;
					else {
						if (_repeatCmdList.contains(UvcControlZoomIn))
							_repeatCmdList.removeAt(_repeatCmdList.indexOf(UvcControlZoomIn));
						_repeatCmdList.append(UvcControlZoomOut);
					}
					break;
				default:
					break;
				}
			}
			if (!_repeatCmdList.isEmpty())
			{
				foreach(UvcControlCmd repeatCmd, _repeatCmdList)
				{
					switch (repeatCmd)
					{
					case UvcControlZoomIn:
						_controler->doZoomIn();
						break;
					case UvcControlZoomOut:
						_controler->doZoomOut();
						break;
					default:
						break;
					}
				}
			}
			msleep(33);
		}
		msleep(100);
	} while (delay--&&_run);
	_run = false;
}

UvcControler::UvcControler(UvcCapture *camera, QObject *parent)
	:QObject(parent)
	, _camera(camera)
	, _propReady(false)
{
	memset(&_propInfo, 0, sizeof(_propInfo));
	_controlThread = new UvcControlerThread(this,this);
}

UvcControler::~UvcControler()
{
	
}
int UvcControler::zoomIn()
{
	_controlThread->addCmd(UvcControlerThread::UvcControlZoomIn);
	return 0;
}
int UvcControler::zoomOut()
{
	_controlThread->addCmd(UvcControlerThread::UvcControlZoomOut);
	return 0;
}

int UvcControler::zoomStop()
{
	_controlThread->addCmd(UvcControlerThread::UvcControlZoomStop);
	return 0;
}

void UvcControler::cameraChanged()
{
	_propReady = false;
}

IBaseFilter * UvcControler::camFilter()
{
	return _camera->m_srcFilter;
}
int UvcControler::getCamPropInfo(CamProp prop, struct CamPropInfo &info)
{
	HRESULT hr;
	memset(&info, 0, sizeof(CamPropInfo));
	bool isVideoProcInf = false; //use IAMVideoProcAmp interface or 
	int property;
	dshowType(prop, property, isVideoProcInf);

	IAMVideoProcAmp *pProcAmp = NULL;
	IAMCameraControl *pCamCtrl = NULL;
	m_uvcCtrlMutex.lock();

	if (isVideoProcInf)
	{
		if (camFilter())
			hr = camFilter()->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	}
	else
	{
		if (camFilter())
			hr = camFilter()->QueryInterface(IID_IAMCameraControl, (void**)&pCamCtrl);
	}

	if (FAILED(hr))
	{
		qDebug() << "Query camera setting interface failed";
	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;
		// Get the range and default value. 
		if (isVideoProcInf)
			hr = pProcAmp->GetRange(property, &Min, &Max, &Step,
				&Default, &Flags);
		else
			hr = pCamCtrl->GetRange(property, &Min, &Max, &Step,
				&Default, &Flags);

		if (SUCCEEDED(hr))
		{
			info.bSupport = true;
			info.min = Min;
			info.max = Max;
			info.steppingDelta = Step;
			info.default = Default;
			if (Flags&CameraControl_Flags_Auto)
				info.bSuppAuto = true;
			else
				info.bSuppAuto = false;

			// Get the current value.
			if (isVideoProcInf)
				hr = pProcAmp->Get(property, &Val, &Flags);
			else
				hr = pCamCtrl->Get(property, &Val, &Flags);
			if (SUCCEEDED(hr))
			{
				info.curValue = Val;
				if (CameraControl_Flags_Auto == Flags)
				{
					info.bAuto = true;
				}
				else if (CameraControl_Flags_Manual == Flags)
				{
					info.bAuto = false;
				}
			}
			else
			{
				qDebug() << "Get cur value failed";
			}
		}
		else
		{
			qDebug() << "Get property "<<prop<<" range failed";
		}
	}
	m_uvcCtrlMutex.unlock();
	return 0;
}
int UvcControler::setCamProp(CamProp prop, long value, bool bAuto)
{
	HRESULT hr;
	bool isVideoProcInf = false; //use IAMVideoProcAmp interface or 
	int property;
	dshowType(prop,property,isVideoProcInf);
	IAMVideoProcAmp *pProcAmp = NULL;
	IAMCameraControl *pCamCtrl = NULL;
	m_uvcCtrlMutex.lock();

	if (isVideoProcInf)
	{
		if (camFilter())
			hr = camFilter()->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	}
	else
	{
		if (camFilter())
			hr = camFilter()->QueryInterface(IID_IAMCameraControl, (void**)&pCamCtrl);
	}
	if (FAILED(hr))
	{
		return false;
	}
	else
	{
		// Get the current value.
		long flags;
		if (bAuto)flags = CameraControl_Flags_Auto;
		else flags = CameraControl_Flags_Manual;
		if (isVideoProcInf)
			hr = pProcAmp->Set(property, value, flags);
		else
			hr = pCamCtrl->Set(property, value, flags);
	}
	m_uvcCtrlMutex.unlock();
	return true;
}

void UvcControler::dshowType(CamProp prop, int &dshowType, bool &isVideoProcInf)
{
	if (prop >= CamPropertyBrightness && prop <= CamPropertyGain)
	{
		dshowType = prop;
		isVideoProcInf = true;
	}
	else if (prop >= CamPropertyPan && prop <= CamPropertyFocus)
	{
		dshowType = prop - CamPropertyCnt;
		isVideoProcInf = false;
	}
}
HRESULT UvcControler::FindExtensionNode(IKsTopologyInfo *pKsTopologyInfo, GUID guid, DWORD *node)
{
	HRESULT hr = E_FAIL;
	DWORD dwNumNodes = 0;
	GUID guidNodeType;
	IKsControl *pKsControl = NULL;
	ULONG ulBytesReturned = 0;
	KSP_NODE ExtensionProp;
	if (!pKsTopologyInfo || !node)
		return E_POINTER;

	// Retrieve the number of nodes in the filter
	hr = pKsTopologyInfo->get_NumNodes(&dwNumNodes);
	if (!SUCCEEDED(hr))
		return hr;
	if (dwNumNodes == 0)
		return E_FAIL;

	// Find the extension unit node that corresponds to the given GUID
	for (unsigned int i = 0; i < dwNumNodes; i++)
	{
		hr = E_FAIL;
		hr = pKsTopologyInfo->get_NodeType(i, &guidNodeType);
		GUID nodeGuid;

		if (SUCCEEDED(hr)&&IsEqualGUID(guidNodeType, guid))
		{
			//IKsTopologyInfo *pKsTopologyInfo = NULL;
#if 0
			DWORD dwExtensionNode=i;
			IExtensionUnit *pExtensionUnit = NULL;
			ULONG ulSize;
			BYTE *	pbPropertyValue = NULL;
			hr = pKsTopologyInfo->CreateNodeInstance(
				dwExtensionNode,
				__uuidof(IExtensionUnit),
				(void **)&pExtensionUnit);
			if (FAILED(hr))
			{
				printf("Unable to create extension node instance : %x\n", hr);
				if(i== dwNumNodes-1)goto errExit;
				else continue;
			}
			hr = pExtensionUnit->get_PropertySize(0, &ulSize);
			if (FAILED(hr))
			{
				printf("Unable to find property size : %x\n", hr);
				goto errExit;
			}
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
				printf("Unable to allocate memory for property value\n");
				goto errExit;
			}
			hr = pExtensionUnit->get_Property(0, ulSize, pbPropertyValue);
			if (FAILED(hr))
			{
				printf("Unable to get property value\n");
				goto errExit;
			}
		errExit:
			//if (pKsTopologyInfo)
				//pKsTopologyInfo->Release();
			if (pExtensionUnit)
				pExtensionUnit->Release();
			if (pbPropertyValue)
				delete[] pbPropertyValue;
#endif
#if 0
			//pKsTopologyInfo->get_NodeName();
			KSTOPOLOGY_CONNECTION connectionInfo;
			DWORD connectNum;
			pKsTopologyInfo->get_NumConnections(&connectNum);
			if (SUCCEEDED(hr) && connectNum >= 0) {
				for (int k = 0; k < connectNum; k++) {
					hr=pKsTopologyInfo->get_ConnectionInfo(i, &connectionInfo);
				}
			}
			DWORD catNum=0;
			hr= pKsTopologyInfo->get_NumCategories(&catNum);
			if (SUCCEEDED(hr) && catNum >= 0) {
				for (int j = 0; j < catNum; j++)
				{
					hr = pKsTopologyInfo->get_Category(j, &nodeGuid);
					if (SUCCEEDED(hr) && IsEqualGUID(nodeGuid, UVCEXTENUNIT_HICAMERA_CONTROL))
					{
						*node = i;
						break;
					}
				}
			}
#endif

#if 0
			printf("found one xu node\n");
			IExtensionUnit*   pExtensionUnit = NULL;
			hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IExtensionUnit), (void **)&pExtensionUnit);
			if (SUCCEEDED(hr))
			{
				ExtensionProp.Property.Set = guid;
				ExtensionProp.Property.Id = 0;
				ExtensionProp.Property.Flags = KSPROPERTY_TYPE_SETSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
				ExtensionProp.NodeId = i;
				ExtensionProp.Reserved = 0;
				*node = i;
				return hr;
				/*
				hr = pKsControl->KsProperty((PKSPROPERTY)&ExtensionProp, sizeof(ExtensionProp), NULL, 0, &ulBytesReturned);
				if(SUCCEEDED(hr))
				{
				*node = i;
				break;
				}
				*/
			}
			else
			{
				printf("CreateNodeInstance failed - 0x%x\n", hr);
			}
#endif
		}
	}
	return hr;
}

int UvcControler::getAllPropInfo(void)
{
	HRESULT hr=S_OK;
	IKsTopologyInfo *pKsTopologyInfo=NULL;
	DWORD dwExtensionNode;
	IExtensionUnit *pExtensionUnit=NULL;
	ULONG ulSize;
	BYTE *	pbPropertyValue=NULL;


	for (int i = 0; i < CamPropertyCount; i++)
	{
		getCamPropInfo(CamProp(i), _propInfo[i]);
	}
	_propReady = true;
#if 0
	// pUnkOuter is the unknown associated with the base filter
	if (camFilter())
	{
		hr = camFilter()->QueryInterface(__uuidof(IKsTopologyInfo),
			(void **)&pKsTopologyInfo);
		if (!SUCCEEDED(hr))
		{
			printf("Unable to obtain IKsTopologyInfo %x\n", hr);
			goto errExit;
		}
	}
	hr = FindExtensionNode(pKsTopologyInfo,
		KSNODETYPE_DEV_SPECIFIC,
		&dwExtensionNode);
	if (FAILED(hr))
	{
		printf("Unable to find extension node : %x\n", hr);
		goto errExit;
	}
	hr = pKsTopologyInfo->CreateNodeInstance(
		dwExtensionNode,
		__uuidof(IExtensionUnit),
		(void **)&pExtensionUnit);
	if (FAILED(hr))
	{
		printf("Unable to create extension node instance : %x\n", hr);
		goto errExit;
	}
	hr = pExtensionUnit->get_PropertySize(1, &ulSize);
	if (FAILED(hr))
	{
		printf("Unable to find property size : %x\n", hr);
		goto errExit;
	}
	pbPropertyValue = new BYTE[ulSize];
	if (!pbPropertyValue)
	{
		printf("Unable to allocate memory for property value\n");
		goto errExit;
	}
	hr = pExtensionUnit->get_Property(1, ulSize, pbPropertyValue);
	if (FAILED(hr))
	{
		printf("Unable to get property value\n");
		goto errExit;
	}
	// assume the property value is an integer
	if(ulSize == 4)
		printf("The value of property 1 = %d\n", *((int *)pbPropertyValue));


	if (pKsTopologyInfo)
		pKsTopologyInfo->Release();
	if (pExtensionUnit)
		pExtensionUnit->Release();
	if (pbPropertyValue)
		delete[] pbPropertyValue;
#endif

	_propReady = true;
	return 0;
errExit:
#if 0
	if (pKsTopologyInfo)
		pKsTopologyInfo->Release();
	if (pExtensionUnit)
		pExtensionUnit->Release();
	if (pbPropertyValue)
		delete[] pbPropertyValue;
#endif
	return -1;
}

int UvcControler::propertyBeReady(void)
{
	if (_propReady)return 0;
	return getAllPropInfo();
}

int UvcControler::doZoomIn()
{
	if (!_camera->isRunning())return -1;
	propertyBeReady();
	if (_propInfo[CamPropertyZoom].bSupport
		&&_propInfo[CamPropertyZoom].curValue != _propInfo[CamPropertyZoom].max)
	{

		long value = _propInfo[CamPropertyZoom].curValue + _propInfo[CamPropertyZoom].steppingDelta;
		if (value > _propInfo[CamPropertyZoom].max)value = _propInfo[CamPropertyZoom].max;
		setCamProp(CamPropertyZoom, value, false);
		_propInfo[CamPropertyZoom].curValue = value;
		return 0;
	}
	return -1;
}

int UvcControler::doZoomOut()
{
	if (!_camera->isRunning())return -1;
	propertyBeReady();
	if (_propInfo[CamPropertyZoom].bSupport
		&&_propInfo[CamPropertyZoom].curValue != _propInfo[CamPropertyZoom].min)
	{
		long value = _propInfo[CamPropertyZoom].curValue - _propInfo[CamPropertyZoom].steppingDelta;
		if (value < _propInfo[CamPropertyZoom].min)value = _propInfo[CamPropertyZoom].min;
		setCamProp(CamPropertyZoom, value, false);
		_propInfo[CamPropertyZoom].curValue = value;
		return 0;
	}
	return -1;
}

