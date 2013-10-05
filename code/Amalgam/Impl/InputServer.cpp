#include <cmath>
#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/InputServer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#if defined(_WIN32) || defined(_XBOX)
#	if !defined(WINCE)
#		include "Input/Xi/InputDriverXi.h"
#	endif
#	if !defined(_XBOX)
#		if !defined(WINCE)
#			include "Input/Di8/InputDriverDi8.h"
#		endif
#		include "Input/Win32/InputDriverWin32.h"
#	endif
#elif defined(_PS3)
#	include "Input/Ps3/InputDriverPs3.h"
#elif TARGET_OS_MAC
#	if TARGET_OS_IPHONE
#		include "Input/iPhone/InputDriverIPhone.h"
#	else
#		include "Input/OsX/InputDriverOsX.h"
#	endif
#elif defined(__LINUX__)
#	include "Input/X11/InputDriverX11.h"
#endif
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffectFactory.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputMappingResource.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputSourceFabricator.h"
#include "Input/Binding/InputState.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

bool anyControlPressed(input::InputSystem* inputSystem, input::InputCategory deviceCategory)
{
	int32_t deviceCount = inputSystem->getDeviceCount(deviceCategory, true);
	for (int32_t i = 0; i < deviceCount; ++i)
	{
		input::IInputDevice* device = inputSystem->getDevice(deviceCategory, i, true);
		if (!device)
			continue;

		int32_t controlCount = device->getControlCount();
		for (int32_t j = 0; j < controlCount; ++j)
		{
			if (device->isControlAnalogue(j))
				continue;

			if (device->getControlValue(j) > 0.5f)
				return true;
		}
	}
	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.InputServer", InputServer, IInputServer)

bool InputServer::create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db, const SystemWindow& systemWindow)
{
	m_settings = settings;
	m_inputSystem = new input::InputSystem();

#if defined(_WIN32)
#	if !defined(WINCE)
	
	// XInput2
	m_inputSystem->addDriver(new input::InputDriverXi());

	// DirectInput 8
	Ref< input::InputDriverDi8 > inputDriverDi8 = new input::InputDriverDi8();
	if (inputDriverDi8->create(systemWindow, input::CtMouse | input::CtJoystick))
		m_inputSystem->addDriver(inputDriverDi8);

#	endif

	// Win32 API
	Ref< input::InputDriverWin32 > inputDriverWin32 = new input::InputDriverWin32();
	if (inputDriverWin32->create(systemWindow, input::CtKeyboard))
		m_inputSystem->addDriver(inputDriverWin32);

#elif TARGET_OS_MAC
#	if TARGET_OS_IPHONE

	// iOS Touch
	Ref< input::InputDriverIPhone > inputDriverIPhone = new input::InputDriverIPhone();
	if (inputDriverIPhone->create(systemWindow))
		m_inputSystem->addDriver(inputDriverIPhone);

#	else

	// OSX HID
	Ref< input::InputDriverOsX > inputDriverOsX = new input::InputDriverOsX();
	if (inputDriverOsX->create(input::CtKeyboard | input::CtMouse | input::CtJoystick))
		m_inputSystem->addDriver(inputDriverOsX);

#	endif
#elif defined(_PS3)

	// PS3
	m_inputSystem->addDriver(new input::InputDriverPs3(4));

#elif defined(__LINUX__)

	// X11
	Ref< input::InputDriverX11 > inputDriverX11 = new input::InputDriverX11();
	if (inputDriverX11->create(systemWindow, input::CtKeyboard | input::CtMouse))
		m_inputSystem->addDriver(inputDriverX11);

#endif

	Guid defaultSourceDataGuid(defaultSettings->getProperty< PropertyString >(L"Input.Default"));
	if (defaultSourceDataGuid.isNotNull())
	{
		Ref< input::InputMappingResource > inputMappingResource = db->getObjectReadOnly< input::InputMappingResource >(defaultSourceDataGuid);
		if (!inputMappingResource)
		{
			log::error << L"Input server failed; unable to read default input configuration" << Endl;
			return false;
		}

		m_inputMappingDefaultSourceData = inputMappingResource->getSourceData();
		m_inputMappingStateData = inputMappingResource->getStateData();
	}

	m_inputMappingSourceData = dynamic_type_cast< input::InputMappingSourceData* >(settings->getProperty< PropertyObject >(L"Input.Sources"));
	if (!m_inputMappingSourceData)
		m_inputMappingSourceData = m_inputMappingDefaultSourceData;

	if (settings->getProperty< PropertyBoolean >(L"Input.Rumble", true))
		m_rumbleEffectPlayer = new input::RumbleEffectPlayer();

	m_inputFabricatorAborted = false;
	m_inputActive = false;

	if (m_inputMappingSourceData && m_inputMappingStateData)
	{
		m_inputMapping = new input::InputMapping();
		m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
	}

	return true;
}

void InputServer::destroy()
{
	m_rumbleEffectPlayer = 0;
	m_inputSystem = 0;
}

void InputServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new input::RumbleEffectFactory(database));
}

int32_t InputServer::reconfigure(const PropertyGroup* settings)
{
	int32_t result = CrUnaffected;

	Ref< input::InputMappingSourceData > inputMappingSourceData = dynamic_type_cast< input::InputMappingSourceData* >(settings->getProperty< PropertyObject >(L"Input.Sources"));
	if (!inputMappingSourceData)
		inputMappingSourceData = m_inputMappingDefaultSourceData;

	if (DeepHash(inputMappingSourceData) != DeepHash(m_inputMappingSourceData))
	{
		if (m_inputMappingSourceData && m_inputMappingStateData)
		{
			if (!m_inputMapping)
				m_inputMapping = new input::InputMapping();

			m_inputMapping->create(
				m_inputSystem,
				m_inputMappingSourceData,
				m_inputMappingStateData
			);
		}
		else
			m_inputMapping = 0;

		result |= CrAccepted;
	}

	bool enableRumble = settings->getProperty< PropertyBoolean >(L"Input.Rumble", true);
	if (enableRumble != bool(m_rumbleEffectPlayer != 0))
	{
		if (enableRumble)
			m_rumbleEffectPlayer = new input::RumbleEffectPlayer();
		else if (m_rumbleEffectPlayer)
		{
			m_rumbleEffectPlayer->stopAll();
			m_rumbleEffectPlayer = 0;
		}
		result |= CrAccepted;
	}

	return result;
}

void InputServer::update(float deltaTime, bool renderViewActive)
{
	if (!m_inputSystem)
		return;

	// Don't update input if render view is inactive.
	if (!renderViewActive)
		m_inputActive = false;

	// Poll all devices.
	m_inputSystem->update(deltaTime);

	// If input is disabled then only check if we can become active.
	if (!m_inputActive)
	{
		// Cannot become active as long as any mouse button is pressed
		// in case application became active with mouse.
		if (!anyControlPressed(m_inputSystem, input::CtMouse))
		{
			if (m_inputMapping)
				m_inputMapping->reset();
			m_inputActive = true;
		}
		return;
	}

	if (!m_inputSourceFabricator)
	{
		// Normal condition; update mapping and proceed.
		if (m_inputMapping)
			m_inputMapping->update(deltaTime);
	}
	else if (m_inputFabricatorAbortDevice)
	{
		// Abort has been triggered; wait until abort key has been released.
		if (m_inputFabricatorAbortDevice->getControlValue(m_inputFabricatorAbortControl) < 0.5f)
		{
			m_inputSourceFabricator = 0;
			if (m_inputFabricatorAbortUnbind)
			{
				if (m_inputMappingSourceData)
					m_inputMappingSourceData->setSourceData(m_inputSourceFabricatorId, 0);

				// Update mapping with new, fabricated, source.
				if (m_inputMappingSourceData && m_inputMappingStateData)
				{
					if (!m_inputMapping)
						m_inputMapping = new input::InputMapping();

					m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
					m_inputMapping->update(deltaTime);
				}
				else
					m_inputMapping = 0;
			}
			else
				m_inputFabricatorAborted = true;

			m_inputFabricatorAbortDevice = 0;
			m_inputFabricatorAbortControl = 0;
			m_inputFabricatorAbortUnbind = false;
		}
	}
	else
	{
		Ref< input::IInputSourceData > sourceData = m_inputSourceFabricator->update();
		if (sourceData)
		{
			m_inputSourceFabricator = 0;

			if (m_inputMappingSourceData)
			{
				uint32_t sourceHash = DeepHash(sourceData).get();
				
				// Discard duplicated input sources.
				const std::map< std::wstring, Ref< input::IInputSourceData > >& currentSourceData = m_inputMappingSourceData->getSourceData();
				for (std::map< std::wstring, Ref< input::IInputSourceData > >::const_iterator i = currentSourceData.begin(); i != currentSourceData.end(); ++i)
				{
					if (DeepHash(i->second) == sourceHash)
						m_inputMappingSourceData->setSourceData(i->first, 0);
				}

				m_inputMappingSourceData->setSourceData(m_inputSourceFabricatorId, sourceData);
			}

			// Update mapping with new, fabricated, source.
			if (m_inputMappingSourceData && m_inputMappingStateData)
			{
				if (!m_inputMapping)
					m_inputMapping = new input::InputMapping();

				m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
				m_inputMapping->update(deltaTime);
			}
			else
				m_inputMapping = 0;

			m_inputFabricatorAborted = false;
		}
		else
		{
			// Abort fabrication if escape or backspace on any keyboard has been pressed.
			int32_t keyboardCount = m_inputSystem->getDeviceCount(input::CtKeyboard, true);
			for (int32_t i = 0; i < keyboardCount; ++i)
			{
				Ref< input::IInputDevice > keyboardDevice = m_inputSystem->getDevice(input::CtKeyboard, i, true);
				if (!keyboardDevice)
					continue;

				int32_t control;

				if (
					keyboardDevice->getDefaultControl(input::DtKeyEscape, false, control) &&
					keyboardDevice->getControlValue(control) >= 0.5f
				)
				{
					// Found pressed escape key; initiate abort sequence as we need to wait until escape is released.
					m_inputFabricatorAbortDevice = keyboardDevice;
					m_inputFabricatorAbortControl = control;
					m_inputFabricatorAbortUnbind = false;
					break;
				}

				if (
					keyboardDevice->getDefaultControl(input::DtKeyBack, false, control) &&
					keyboardDevice->getControlValue(control) >= 0.5f
				)
				{
					// Found pressed backspace key; initiate abort sequence as we need to wait until backspace is released.
					m_inputFabricatorAbortDevice = keyboardDevice;
					m_inputFabricatorAbortControl = control;
					m_inputFabricatorAbortUnbind = true;
					break;
				}
			}
		}
	}
}

void InputServer::updateRumble(float deltaTime, bool paused)
{
	if (m_rumbleEffectPlayer)
	{
		if (paused)
			m_rumbleEffectPlayer->stopAll();
		else
			m_rumbleEffectPlayer->update(deltaTime);
	}
}

bool InputServer::createInputMapping(const input::InputMappingStateData* stateData)
{
	if (!m_inputMappingSourceData)
	{
		log::error << L"Unable to create input mapping; no source data" << Endl;
		return false;
	}

	if ((m_inputMappingStateData = stateData) == 0)
	{
		log::error << L"Unable to create input mapping; no state data" << Endl;
		return false;
	}

	if (!m_inputMapping)
		m_inputMapping = new input::InputMapping();

	m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
	return true;
}

bool InputServer::fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue)
{
	if (!m_inputSystem || sourceId.empty() || category == input::CtUnknown)
		return false;

	// Ensure mapping are reset so no lingering states
	// cause "stuck" behavior.
	if (m_inputMapping)
		m_inputMapping->reset();

	// Create source fabricator; will consume all inputs
	// until source has been fabricated.
	m_inputSourceFabricatorId = sourceId;
	m_inputSourceFabricator = new input::InputSourceFabricator(
		m_inputSystem,
		category,
		analogue
	);

	m_inputFabricatorAborted = false;
	return true;
}

bool InputServer::isFabricating() const
{
	return m_inputSourceFabricator != 0;
}

bool InputServer::abortedFabricating() const
{
	return m_inputFabricatorAborted;
}

bool InputServer::resetInputSource(const std::wstring& sourceId)
{
	if (!m_inputMappingDefaultSourceData || !m_inputMappingSourceData)
		return false;

	const std::map< std::wstring, Ref< input::IInputSourceData > >& defaultSourceData = m_inputMappingDefaultSourceData->getSourceData();

	std::map< std::wstring, Ref< input::IInputSourceData > >::const_iterator i = defaultSourceData.find(sourceId);
	if (i == defaultSourceData.end())
		return false;

	m_inputMappingSourceData->setSourceData(sourceId, i->second);

	if (m_inputMappingStateData)
	{
		if (!m_inputMapping)
			m_inputMapping = new input::InputMapping();

		m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
		m_inputMapping->update(1.0f / 30.0f);
	}
	else
		m_inputMapping = 0;

	return true;
}

bool InputServer::isIdle() const
{
	if (!m_inputMapping)
		return false;

	const std::map< std::wstring, Ref< input::InputState > >& states = m_inputMapping->getStates();
	for (std::map< std::wstring, Ref< input::InputState > >::const_iterator i = states.begin(); i != states.end(); ++i)
	{
		float dV = i->second->getValue() - i->second->getPreviousValue();
		if (std::abs(dV) > FUZZY_EPSILON)
			return false;
	}

	return true;
}

void InputServer::apply()
{
	if (m_inputMappingSourceData)
		m_settings->setProperty< PropertyObject >(L"Input.Sources", m_inputMappingSourceData);
}

void InputServer::revert()
{
	m_inputMappingSourceData = dynamic_type_cast< input::InputMappingSourceData* >(m_settings->getProperty< PropertyObject >(L"Input.Sources"));
	if (!m_inputMappingSourceData)
		m_inputMappingSourceData = m_inputMappingDefaultSourceData;

	if (m_inputMappingSourceData && m_inputMappingStateData)
	{
		if (!m_inputMapping)
			m_inputMapping = new input::InputMapping();

		m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
		m_inputMapping->update(1.0f / 60.0f);
	}
	else
		m_inputMapping = 0;
}

input::InputSystem* InputServer::getInputSystem()
{
	return m_inputSystem;
}

input::InputMapping* InputServer::getInputMapping()
{
	return m_inputMapping;
}

input::RumbleEffectPlayer* InputServer::getRumbleEffectPlayer()
{
	return m_rumbleEffectPlayer;
}

	}
}
