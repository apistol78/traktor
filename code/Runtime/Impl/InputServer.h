/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IInputServer.h"
#include "Core/Platform.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::db
{

class Database;

}

namespace traktor::input
{

class IInputDevice;
class InputMappingSourceData;
class InputSourceFabricator;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class InputServer : public IInputServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db, const SystemApplication& sysapp, const SystemWindow& syswin);

	void destroy();

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float deltaTime, bool renderViewActive);

	void updateRumble(float deltaTime, bool paused);

	virtual bool createInputMapping(const input::InputMappingStateData* stateData) override final;

	virtual bool fabricateInputSource(const std::wstring& sourceId, input::InputCategory category, bool analogue) override final;

	virtual bool isFabricating() const override final;

	virtual bool abortedFabricating() const override final;

	virtual bool resetInputSource(const std::wstring& sourceId) override final;

	virtual bool isIdle() const override final;

	virtual void apply() override final;

	virtual void revert() override final;

	virtual input::InputSystem* getInputSystem() override final;

	virtual input::InputMapping* getInputMapping() override final;

	virtual input::RumbleEffectPlayer* getRumbleEffectPlayer() override final;

private:
	Ref< PropertyGroup > m_settings;
	Ref< input::InputSystem > m_inputSystem;
	Ref< input::InputMapping > m_inputMapping;
	Ref< const input::InputMappingSourceData > m_inputMappingDefaultSourceData;
	Ref< input::InputMappingSourceData > m_inputMappingSourceData;
	Ref< const input::InputMappingStateData > m_inputMappingStateData;
	uint32_t m_inputConstantsHash = 0;
	Ref< input::InputSourceFabricator > m_inputSourceFabricator;
	std::wstring m_inputSourceFabricatorId;
	Ref< input::IInputDevice > m_inputFabricatorAbortDevice;
	int32_t m_inputFabricatorAbortControl = 0;
	bool m_inputFabricatorAbortUnbind = false;
	bool m_inputFabricatorAborted = false;
	bool m_inputActive = false;
	Ref< input::RumbleEffectPlayer > m_rumbleEffectPlayer;
};

}
