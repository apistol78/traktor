#pragma optimize( "", off )

#if !defined(WINCE)

#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/WebBrowser.h"
#include "Ui/Win32/WebBrowserWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

template < typename Class >
class UnknownImpl : public Class
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (std::memcmp(&riid, &IID_IUnknown, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return ++m_refCount;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return --m_refCount;
	}

private:
	AtomicRefCount m_refCount;
};

template < typename Class1, typename Class2 >
class UnknownImpl2 : public Class1, public Class2
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (std::memcmp(&riid, &IID_IUnknown, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return ++m_refCount;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return --m_refCount;
	}

private:
	AtomicRefCount m_refCount;
};


class OleInPlaceSite : public UnknownImpl2< IOleInPlaceSite, IOleInPlaceFrame >
{
public:
	OleInPlaceSite(HWND hWnd)
	:	m_hWnd(hWnd)
	{
	}

	void setOleObject(IOleInPlaceObject* oleInPlaceObject)
	{
		m_oleInPlaceObject = oleInPlaceObject;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (std::memcmp(&riid, &IID_IOleInPlaceSite, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		else if (std::memcmp(&riid, &IID_IOleInPlaceFrame, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		return UnknownImpl2< IOleInPlaceSite, IOleInPlaceFrame >::QueryInterface(riid, ppvObject);
	}

	virtual  HRESULT STDMETHODCALLTYPE GetWindow(HWND *phwnd)
	{
		*phwnd = m_hWnd;
		return S_OK;
	}
    
	virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate(void)
	{
		return S_OK;
	}
    
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate(void)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnUIActivate(void)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetWindowContext( 
		IOleInPlaceFrame **ppFrame,
		IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect,
		LPRECT lprcClipRect,
		LPOLEINPLACEFRAMEINFO lpFrameInfo
	)
	{
		*ppFrame = this;

		// We have no OLEINPLACEUIWINDOW
		*ppDoc = 0;

		// Fill in some other info for the browser
		lpFrameInfo->fMDIApp = FALSE;
		lpFrameInfo->hwndFrame = m_hWnd;
		lpFrameInfo->haccel = 0;
		lpFrameInfo->cAccelEntries = 0;

		// Give the browser the dimensions of where it can draw. We give it our entire window to fill
		GetClientRect(m_hWnd, lprcPosRect);
		GetClientRect(m_hWnd, lprcClipRect);

		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtant)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate(void)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE DiscardUndoState( void)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo(void)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect)
	{
		if (m_oleInPlaceObject)
			m_oleInPlaceObject->SetObjectRects(lprcPosRect, lprcPosRect);
		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE GetBorder(LPRECT lprectBorder)
	{
		return E_NOTIMPL;
	}
        
    virtual HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
	{
		return E_NOTIMPL;
	}
        
    virtual HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
	{
		return E_NOTIMPL;
	}

    virtual HRESULT STDMETHODCALLTYPE SetActiveObject( 
		IOleInPlaceActiveObject *pActiveObject,
		LPCOLESTR pszObjName
	)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE InsertMenus( 
		HMENU hmenuShared,
		LPOLEMENUGROUPWIDTHS lpMenuWidths
	)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetMenu( 
		HMENU hmenuShared,
		HOLEMENU holemenu,
		HWND hwndActiveObject
	)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU hmenuShared)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR pszStatusText)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg, WORD wID)
	{
		return E_NOTIMPL;
	}

private:
	HWND m_hWnd;
	ComRef< IOleInPlaceObject > m_oleInPlaceObject;
};

class OleClientSite : public UnknownImpl< IOleClientSite >
{
public:
	OleClientSite(IOleInPlaceSite* inPlaceSite)
	:	m_inPlaceSite(inPlaceSite)
	{
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (std::memcmp(&riid, &IID_IOleClientSite, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		else if (std::memcmp(&riid, &IID_IOleInPlaceSite, sizeof(GUID)) == 0)
		{
			*ppvObject = m_inPlaceSite;
			return S_OK;
		}
		return UnknownImpl::QueryInterface(riid, ppvObject);
	}

	virtual HRESULT STDMETHODCALLTYPE SaveObject(void)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer **ppContainer)
	{
		return E_NOINTERFACE;
	}
        
	virtual HRESULT STDMETHODCALLTYPE ShowObject(void)
	{
		return NOERROR;
	}

	virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout(void)
	{
		return E_NOTIMPL;
	}

private:
	ComRef< IOleInPlaceSite > m_inPlaceSite;
};

class OleStorage : public UnknownImpl< IStorage >
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateStream( 
		const OLECHAR *pwcsName,
		DWORD grfMode,
		DWORD reserved1,
		DWORD reserved2,
		::IStream **ppstm
	)
	{
		return E_NOTIMPL;
	}	
        
	virtual HRESULT STDMETHODCALLTYPE OpenStream( 
		const OLECHAR *pwcsName,
		void *reserved1,
		DWORD grfMode,
		DWORD reserved2,
		::IStream **ppstm
	)
   	{
		return E_NOTIMPL;
	}
     
	virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
		const OLECHAR *pwcsName,
		DWORD grfMode,
		DWORD reserved1,
		DWORD reserved2,
		IStorage **ppstg
	)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OpenStorage( 
		const OLECHAR *pwcsName,
		IStorage *pstgPriority,
		DWORD grfMode,
		SNB snbExclude,
		DWORD reserved,
		IStorage **ppstg
	)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE CopyTo( 
		DWORD ciidExclude,
		const IID *rgiidExclude,
		SNB snbExclude,
		IStorage *pstgDest
	)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE MoveElementTo( 
		const OLECHAR *pwcsName,
		IStorage *pstgDest,
		const OLECHAR *pwcsNewName,
		DWORD grfFlags
	)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE Revert(void)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE EnumElements( 
		DWORD reserved1,
		void *reserved2,
		DWORD reserved3,
		IEnumSTATSTG **ppenum
	)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE DestroyElement(const OLECHAR *pwcsName)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE RenameElement(const OLECHAR *pwcsOldName, const OLECHAR *pwcsNewName)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE SetElementTimes(const OLECHAR *pwcsName, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE SetClass(REFCLSID clsid)
	{
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE SetStateBits(DWORD grfStateBits, DWORD grfMask)
	{
		return E_NOTIMPL;
	}
        
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag)
	{
		return E_NOTIMPL;
	}
};

		}

WebBrowserWin32::WebBrowserWin32(EventSubject* owner)
:	WidgetWin32Impl< IWebBrowser >(owner)
,	m_doneVerb(false)
{
}

bool WebBrowserWin32::create(IWidget* parent, const std::wstring& url)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP;
	DWORD dwStyleEx = WS_EX_CONTROLPARENT;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("TraktorWin32Class"),
		_T(""),
		dwStyle,
		dwStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		32,
		32
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	RECT rcInner;
	GetClientRect(m_hWnd, &rcInner);

	m_hWnd.registerMessageHandler(WM_SIZE, new MethodMessageHandler< WebBrowserWin32 >(this, &WebBrowserWin32::eventSize));

	ComRef< OleInPlaceSite > inPlaceSite = new OleInPlaceSite(m_hWnd);
	m_clientSite = new OleClientSite(inPlaceSite);
	m_storage = new OleStorage();

	HRESULT hr = OleCreate(
		CLSID_WebBrowser,
		IID_IOleObject,
		OLERENDER_DRAW,
		0,
		m_clientSite,
		m_storage,
		(void**)&m_webBrowserObject.getAssign()
	);
	if (FAILED(hr) || !m_webBrowserObject)
	{
		log::error << L"Unable to create WebBrowser OLE object, hr = " << int32_t(hr) << Endl;
		return false;
	}

	ComRef< IOleInPlaceObject > inPlaceObject;
	m_webBrowserObject->QueryInterface(IID_IOleInPlaceObject, (void**)&inPlaceObject.getAssign());
	if (FAILED(hr) || !inPlaceObject)
	{
		log::error << L"Unable to get IOleInPlaceObject implementation, hr = " << int32_t(hr) << Endl;
		return false;
	}

	inPlaceSite->setOleObject(inPlaceObject);

	hr = m_webBrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&m_webBrowser.getAssign());
	if (FAILED(hr) || !m_webBrowser)
	{
		log::error << L"Unable to get IWebBrowser2 implementation, hr = " << int32_t(hr) << Endl;
		return false;
	}

	OleSetContainedObject(m_webBrowserObject, TRUE);

	if (!url.empty())
		navigate(url);

	return true;
}

void WebBrowserWin32::navigate(const std::wstring& url)
{
	VARIANT vurl;
	HRESULT hr;

	// Ensure path is absolute if using file:// protocol.
	if (startsWith< std::wstring >(url, L"file://"))
	{
		std::wstring qurl = L"file://" + FileSystem::getInstance().getAbsolutePath(url.substr(7)).getPathName();
		VariantInit(&vurl);
		vurl.vt = VT_BSTR;
		vurl.bstrVal = SysAllocString(qurl.c_str());
	}
	else
	{
		VariantInit(&vurl);
		vurl.vt = VT_BSTR;
		vurl.bstrVal = SysAllocString(url.c_str());
	}

	hr = m_webBrowser->Navigate2(&vurl, 0, 0, 0, 0);
	if (FAILED(hr))
		log::error << L"Unable to navigate to URL \"" << url << L"\", hr = " << int32_t(hr) << Endl;

	VariantClear(&vurl);
}

void WebBrowserWin32::forward()
{
	m_webBrowser->GoForward();
}

void WebBrowserWin32::back()
{
	m_webBrowser->GoBack();
}

void WebBrowserWin32::reload(bool forced)
{
	VARIANT v;
	v.vt = VT_I4;
	v.intVal = forced ? REFRESH_COMPLETELY : REFRESH_NORMAL;
	m_webBrowser->Refresh2(&v);
}

bool WebBrowserWin32::ready() const
{
	READYSTATE rs;
	m_webBrowser->get_ReadyState(&rs);
	return rs == READYSTATE_COMPLETE;
}

LRESULT WebBrowserWin32::eventSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass)
{
	if (m_webBrowser)
	{
		T_ASSERT (m_webBrowserObject);

		RECT rect;
		GetClientRect(m_hWnd, &rect);

		if (!m_doneVerb)
		{
			m_webBrowserObject->DoVerb(OLEIVERB_SHOW, NULL, m_clientSite, -1, m_hWnd, &rect);
			m_doneVerb = true;
		}

		m_webBrowser->put_Left(0);
		m_webBrowser->put_Top(0);
		m_webBrowser->put_Width(rect.right - rect.left);
		m_webBrowser->put_Height(rect.bottom - rect.top);
	}
	return TRUE;
}

	}
}

#endif
