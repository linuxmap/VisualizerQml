#include "Stdafx.h"
#include "interface.h"
#include <ks.h>
#include <ksproxy.h>
#include <vidcap.h>
#include <ksmedia.h>


// {3520FA18-3588-4051-B9EE-6E323768E26E}
DEFINE_GUID(CLSID_ExtensionUnit, 
0x3520fa18, 0x3588, 0x4051, 0xb9, 0xee, 0x6e, 0x32, 0x37, 0x68, 0xe2, 0x6e);

class CNodeControl :
    public IKsNodeControl
{
public:
    STDMETHOD(put_NodeId) (DWORD dwNodeId);
    STDMETHOD(put_KsControl) (PVOID pKsControl);

    DWORD m_dwNodeId;
    CComPtr<IKsControl> m_pKsControl;
};

class CExtension :
   public IExtensionUnit,
   public CComObjectRootEx<CComObjectThreadModel>,
   public CComCoClass<CExtension, &CLSID_ExtensionUnit>,
   public CNodeControl
{
   public:

   CExtension();
   STDMETHOD(FinalConstruct)();

   BEGIN_COM_MAP(CExtension)
      COM_INTERFACE_ENTRY(IKsNodeControl)
      COM_INTERFACE_ENTRY(IExtensionUnit)
   END_COM_MAP()

   DECLARE_PROTECT_FINAL_CONSTRUCT()
   DECLARE_NO_REGISTRY()
   DECLARE_ONLY_AGGREGATABLE(CExtension)

   // IExtensionUnit
   public:
   STDMETHOD (get_Info)(
      ULONG ulSize,
      BYTE pInfo[]);
   STDMETHOD (get_InfoSize)(
      ULONG *pulSize);
   STDMETHOD (get_PropertySize)(
      ULONG PropertyId, 
      ULONG *pulSize);
   STDMETHOD (get_Property)(
      ULONG PropertyId, 
      ULONG ulSize, 
      BYTE pValue[]);
   STDMETHOD (put_Property)(
      ULONG PropertyId, 
      ULONG ulSize, 
      BYTE pValue[]);
   STDMETHOD (get_PropertyRange)(
      ULONG PropertyId, 
      ULONG ulSize,
      BYTE pMin[], 
      BYTE pMax[], 
      BYTE pSteppingDelta[], 
      BYTE pDefault[]);
};

#define STATIC_PROPSETID_VIDCAP_EXTENSION_UNIT \
    0x9A1E7291, 0x6843, 0x4683, 0x6D, 0x92, 0x39, 0xBC, 0x79, 0x06, 0xEE, 0x49

DEFINE_GUIDSTRUCT("9A1E7291-6843-4683-6D92-39BC7906EE49",\
   PROPSETID_VIDCAP_EXTENSION_UNIT);
#define PROPSETID_VIDCAP_EXTENSION_UNIT   DEFINE_GUIDNAMED(PROPSETID_VIDCAP_EXTENSION_UNIT)