/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Editor/Deploy/ITargetAction.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace runtime
	{

class Target;
class TargetConfiguration;

/*! Launch target action.
 * \ingroup Runtime
 */
class T_DLLCLASS LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const std::wstring& targetName,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& deployHost,
		const std::wstring& outputPath
	);

	virtual bool execute(IProgressListener* progressListener) override final;

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	std::wstring m_targetName;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_deployHost;
	std::wstring m_outputPath;
};

	}
}

