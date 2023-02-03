/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Deployment tool description.
 * \ingroup Runtime
 */
class T_DLLCLASS DeployTool : public ISerializable
{
	T_RTTI_CLASS;

public:
	const std::wstring& getExecutable() const;

	const std::map< std::wstring, std::wstring >& getEnvironment() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_executable;
	std::map< std::wstring, std::wstring > m_environment;
};

}
