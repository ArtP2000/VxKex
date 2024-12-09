#include "buildcfg.h"
#include "kxcomp.h"

KXCOMAPI HRESULT WINAPI _CoCreateInstance(REFCLSID rclsid, IUnknown *outer, DWORD cls_context,
        REFIID riid, void **obj)
{
	MULTI_QI OneQI;
    HRESULT hr;

    if (!obj)
        return E_POINTER;
	
	OneQI.pItf = NULL;
	OneQI.pIID = riid;

    hr = CoCreateInstanceEx(rclsid, outer, cls_context, NULL, 1, &OneQI);
    *obj = OneQI.pItf;
    return hr;
}