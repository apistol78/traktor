/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
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

}

namespace traktor::db
{

class Database;

}

namespace traktor::runtime
{

class Target;
class TargetConfiguration;

/*! Build target action.
 * \ingroup Runtime
 */
class T_DLLCLASS BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	explicit BuildTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const PropertyGroup* defaultPipelineSettings,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& outputPath,
		const PropertyGroup* tweakSettings,
		bool force
	);

	virtual bool execute(IProgressListener* progressListener) override final;

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	Ref< const PropertyGroup > m_defaultPipelineSettings;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_outputPath;
	Ref< const PropertyGroup > m_tweakSettings;
	bool m_force;
};

}
