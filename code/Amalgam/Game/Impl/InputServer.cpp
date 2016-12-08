#include <cmath>
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Impl/InputServer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Input/IInputDevice.h"
#include "Input/IInputDriver.h"
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

InputServer::InputServer()
:	m_inputConstantsHash(0)
,	m_inputFabricatorAbortControl(0)
,	m_inputFabricatorAbortUnbind(false)
,	m_inputFabricatorAborted(false)
,	m_inputActive(false)
{
}

bool InputServer::create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db, const SystemApplication& sysapp, const SystemWindow& syswin)
{
	m_settings = settings;
	m_inputSystem = new input::InputSystem();

	// Merge user settings with default settings in order to get new properties in case application has been updated.
	Ref< const PropertyGroup > mergedSettings = defaultSettings;
	if (settings)
		mergedSettings = defaultSettings->mergeJoin(settings);

	// Instanciate input drivers.
	std::set< std::wstring > driverTypes = mergedSettings->getProperty< PropertyStringSet >(L"Input.DriverTypes");
	for (std::set< std::wstring >::const_iterator i = driverTypes.begin(); i != driverTypes.end(); ++i)
	{
		Ref< input::IInputDriver > driver = dynamic_type_cast< input::IInputDriver* >(TypeInfo::createInstance(*i));
		if (!driver)
		{
			log::error << L"Input server failed; unable to instantiate driver \"" << *i << L"\"" << Endl;
			continue;
		}

		if (!driver->create(sysapp, syswin, input::CtKeyboard | input::CtMouse | input::CtJoystick | input::CtWheel | input::CtTouch | input::CtGaze | input::CtAcceleration | input::CtOrientation))
		{
			log::error << L"Input server failed; unable to create driver \"" << *i << L"\"" << Endl;
			continue;
		}

		m_inputSystem->addDriver(driver);
	}

	// Read default input mapping.
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

	// Read input sources.
	m_inputMappingSourceData = dynamic_type_cast< input::InputMappingSourceData* >(mergedSettings->getProperty< PropertyObject >(L"Input.Sources"));
	if (!m_inputMappingSourceData)
		m_inputMappingSourceData = DeepClone(m_inputMappingDefaultSourceData).create< input::InputMappingSourceData >();

	// Create rumble effect player.
	if (mergedSettings->getProperty< PropertyBoolean >(L"Input.Rumble", true))
		m_rumbleEffectPlayer = new input::RumbleEffectPlayer();

	m_inputFabricatorAborted = false;
	m_inputActive = false;

	// Create input mapping, and also set input constants from configuration.
	Ref< const PropertyGroup > inputConstants = mergedSettings->getProperty< PropertyGroup >(L"Input.Constants");
	m_inputConstantsHash = DeepHash(inputConstants).get();

	if (m_inputMappingSourceData && m_inputMappingStateData)
	{
		m_inputMapping = new input::InputMapping();
		m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);

		// Set global constants from configuration settings.
		if (inputConstants)
		{
			const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
			for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
			{
				m_inputMapping->setValue(
					i->first,
					PropertyFloat::get(i->second)
				);
			}
		}
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
		inputMappingSourceData = DeepClone(m_inputMappingDefaultSourceData).create< input::InputMappingSourceData >();

	Ref< const PropertyGroup > inputConstants = settings->getProperty< PropertyGroup >(L"Input.Constants");

	if (
		DeepHash(inputMappingSourceData) != DeepHash(m_inputMappingSourceData) ||
		DeepHash(inputConstants).get() != m_inputConstantsHash
	)
	{
		if (m_inputMappingSourceData && m_inputMappingStateData)
		{
			if (!m_inputMapping)
				m_inputMapping = new input::InputMapping();

			m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);

			// Set global constants from configuration settings.
			if (inputConstants)
			{
				const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
				for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
				{
					m_inputMapping->setValue(
						i->first,
						PropertyFloat::get(i->second)
					);
				}
			}
		}
		else
			m_inputMapping = 0;

		m_inputConstantsHash = DeepHash(inputConstants).get();

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
	m_inputSystem->update();

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

		// Massage input mapping even if input is disabled; this
		// is to let mapping handle timers etc.
		if (m_inputMapping)
			m_inputMapping->update(deltaTime, false);

		return;
	}

	if (!m_inputSourceFabricator)
	{
		// Normal condition; update mapping and proceed.
		if (m_inputMapping)
			m_inputMapping->update(deltaTime, true);
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
					m_inputMapping->update(deltaTime, true);

					Ref< const PropertyGroup > inputConstants = m_settings->getProperty< PropertyGroup >(L"Input.Constants");
					if (inputConstants)
					{
						const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
						for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
						{
							m_inputMapping->setValue(
								i->first,
								PropertyFloat::get(i->second)
							);
						}
					}
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
				m_inputMapping->update(deltaTime, true);

				Ref< const PropertyGroup > inputConstants = m_settings->getProperty< PropertyGroup >(L"Input.Constants");
				if (inputConstants)
				{
					const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
					for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
					{
						m_inputMapping->setValue(
							i->first,
							PropertyFloat::get(i->second)
						);
					}
				}
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
		m_inputMapping->update(1.0f / 30.0f, true);

		Ref< const PropertyGroup > inputConstants = m_settings->getProperty< PropertyGroup >(L"Input.Constants");
		if (inputConstants)
		{
			const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
			for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
			{
				m_inputMapping->setValue(
					i->first,
					PropertyFloat::get(i->second)
				);
			}
		}
	}
	else
		m_inputMapping = 0;

	return true;
}

bool InputServer::isIdle() const
{
	if (!m_inputMapping)
		return false;

	const SmallMap< input::handle_t, Ref< input::InputState > >& states = m_inputMapping->getStates();
	for (SmallMap< input::handle_t, Ref< input::InputState > >::const_iterator i = states.begin(); i != states.end(); ++i)
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
		m_inputMappingSourceData = DeepClone(m_inputMappingDefaultSourceData).create< input::InputMappingSourceData >();

	if (m_inputMappingSourceData && m_inputMappingStateData)
	{
		if (!m_inputMapping)
			m_inputMapping = new input::InputMapping();

		m_inputMapping->create(m_inputSystem, m_inputMappingSourceData, m_inputMappingStateData);
		m_inputMapping->update(1.0f / 60.0f, true);

		Ref< const PropertyGroup > inputConstants = m_settings->getProperty< PropertyGroup >(L"Input.Constants");
		if (inputConstants)
		{
			const std::map< std::wstring, Ref< IPropertyValue > >& values = inputConstants->getValues();
			for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
			{
				m_inputMapping->setValue(
					i->first,
					PropertyFloat::get(i->second)
				);
			}
		}
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
