#ifndef traktor_amalgam_InputServer_H
#define traktor_amalgam_InputServer_H

#include "Amalgam/IInputServer.h"

namespace traktor
{

class PropertyGroup;
struct SystemWindow;

	namespace db
	{

class Database;

	}

	namespace input
	{

class IInputDevice;
class InputMappingSourceData;
class InputSourceFabricator;

	}

	namespace amalgam
	{

class IEnvironment;

class InputServer : public IInputServer
{
	T_RTTI_CLASS;

public:
	InputServer();

	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db, void* nativeHandle, const SystemWindow& systemWindow);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float deltaTime, bool renderViewActive);

	void updateRumble(float deltaTime, bool paused);

	virtual bool createInputMapping(const input::InputMappingStateData* stateData);

	virtual bool fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue);

	virtual bool isFabricating() const;

	virtual bool abortedFabricating() const;

	virtual bool resetInputSource(const std::wstring& sourceId);

	virtual bool isIdle() const;

	virtual void apply();

	virtual void revert();

	virtual input::InputSystem* getInputSystem();

	virtual input::InputMapping* getInputMapping();

	virtual input::RumbleEffectPlayer* getRumbleEffectPlayer();

private:
	Ref< PropertyGroup > m_settings;
	Ref< input::InputSystem > m_inputSystem;
	Ref< input::InputMapping > m_inputMapping;
	Ref< const input::InputMappingSourceData > m_inputMappingDefaultSourceData;
	Ref< input::InputMappingSourceData > m_inputMappingSourceData;
	Ref< const input::InputMappingStateData > m_inputMappingStateData;
	uint32_t m_inputConstantsHash;
	Ref< input::InputSourceFabricator > m_inputSourceFabricator;
	std::wstring m_inputSourceFabricatorId;
	Ref< input::IInputDevice > m_inputFabricatorAbortDevice;
	int32_t m_inputFabricatorAbortControl;
	bool m_inputFabricatorAbortUnbind;
	bool m_inputFabricatorAborted;
	bool m_inputActive;
	Ref< input::RumbleEffectPlayer > m_rumbleEffectPlayer;
};

	}
}

#endif	// traktor_amalgam_InputServer_H
