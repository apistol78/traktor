/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Editor/Deploy/DeployTool.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Deployment platform descriptor.
 * \ingroup Runtime
 */
class T_DLLCLASS Platform : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t getIconIndex() const;

	const DeployTool& getDeployTool() const;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_iconIndex = 0;
	DeployTool m_deployToolWin64;
	DeployTool m_deployToolOsX;
	DeployTool m_deployToolLinux;
};

}
