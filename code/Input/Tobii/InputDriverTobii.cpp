#include "Core/Log/Log.h"
#include "Input/Tobii/InputDeviceGaze.h"
#include "Input/Tobii/InputDriverTobii.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

static const TX_STRING s_InteractorId = "Traktor";

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverTobii", 0, InputDriverTobii, IInputDriver)

InputDriverTobii::InputDriverTobii()
:	m_hContext(TX_EMPTY_HANDLE)
,	m_hGlobalInteractorSnapshot(TX_EMPTY_HANDLE)
,	m_hConnectionStateChangedTicket(TX_INVALID_TICKET)
,	m_hEventHandlerTicket(TX_INVALID_TICKET)
{
}

InputDriverTobii::~InputDriverTobii()
{
	destroy();
}

void InputDriverTobii::destroy()
{
	if (m_hContext != TX_EMPTY_HANDLE)
	{
		txDisableConnection(m_hContext);
		txReleaseObject(&m_hGlobalInteractorSnapshot);
		txShutdownContext(m_hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
		txReleaseContext(&m_hContext);
		txUninitializeEyeX();
		m_hContext = TX_EMPTY_HANDLE;
	}
	m_device = 0;
}

bool InputDriverTobii::create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };

	if (txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) != TX_RESULT_OK)
		return false;

	if (txCreateContext(&m_hContext, TX_FALSE) != TX_RESULT_OK)
		return false;

	if (txCreateGlobalInteractorSnapshot(
		m_hContext,
		s_InteractorId,
		&m_hGlobalInteractorSnapshot,
		&hInteractor
	) != TX_RESULT_OK)
		return false;

	if (txCreateGazePointDataBehavior(hInteractor, &params) != TX_RESULT_OK)
		return false;

	txReleaseObject(&hInteractor);

	if (txRegisterConnectionStateChangedHandler(m_hContext, &m_hConnectionStateChangedTicket, engineConnectionStateChanged, this) != TX_RESULT_OK)
		return false;

	if (txRegisterEventHandler(m_hContext, &m_hEventHandlerTicket, handleEvent, this) != TX_RESULT_OK)
		return false;

	if (txEnableConnection(m_hContext) != TX_RESULT_OK)
		return false;

	m_device = new InputDeviceGaze();
	return true;
}

int InputDriverTobii::getDeviceCount()
{
	return 1;
}

Ref< IInputDevice > InputDriverTobii::getDevice(int index)
{
	if (index == 0)
		return m_device;
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverTobii::update()
{
	return UrOk;
}

void TX_CALLCONVENTION InputDriverTobii::snapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	InputDriverTobii* this_ = reinterpret_cast< InputDriverTobii* >(param);
	T_FATAL_ASSERT (this_);

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	T_FATAL_ASSERT (result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}

void TX_CALLCONVENTION InputDriverTobii::engineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	InputDriverTobii* this_ = reinterpret_cast< InputDriverTobii* >(userParam);
	T_FATAL_ASSERT (this_);

	switch (connectionState)
	{
	case TX_CONNECTIONSTATE_CONNECTED:
		{
			log::info << L"Tobii EyeX connected." << Endl;
			if (txCommitSnapshotAsync(this_->m_hGlobalInteractorSnapshot, snapshotCommitted, this_) == TX_RESULT_OK)
				log::info << L"Gaze data streaming started." << Endl;
			else
				log::error << L"Failed to start gaze data streaming." << Endl;
		}
		break;

	case TX_CONNECTIONSTATE_DISCONNECTED:
		{
			log::info << L"Tobii EyeX disconnected." << Endl;
		}
		break;
	}
}

void TX_CALLCONVENTION InputDriverTobii::handleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	InputDriverTobii* this_ = reinterpret_cast< InputDriverTobii* >(userParam);
	T_FATAL_ASSERT (this_);

	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

	txGetAsyncDataContent(hAsyncData, &hEvent);

	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK)
	{
		TX_GAZEPOINTDATAEVENTPARAMS eventParams;
		if (txGetGazePointDataEventParams(hBehavior, &eventParams) == TX_RESULT_OK)
		{
			this_->m_device->m_positionX = eventParams.X;
			this_->m_device->m_positionY = eventParams.Y;
		}

		txReleaseObject(&hBehavior);
	}

	txReleaseObject(&hEvent);
}

	}
}
