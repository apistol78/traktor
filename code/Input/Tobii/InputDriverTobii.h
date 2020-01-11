#pragma once

#include <eyex/EyeX.h>
#include "Core/System.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_TOBII_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDeviceGaze;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputDriverTobii : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverTobii();

	virtual ~InputDriverTobii();

	void destroy();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	TX_CONTEXTHANDLE m_hContext;
	TX_HANDLE m_hGlobalInteractorSnapshot;
	TX_TICKET m_hConnectionStateChangedTicket;
	TX_TICKET m_hEventHandlerTicket;
	TX_TICKET m_hPresenceStateChangedTicket;
	HWND m_hWnd;
	Ref< InputDeviceGaze > m_device;

	static void TX_CALLCONVENTION snapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param);

	static void TX_CALLCONVENTION engineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam);

	static void TX_CALLCONVENTION handleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam);

	static void TX_CALLCONVENTION engineStateChanged(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam);
};

	}
}

