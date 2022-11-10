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
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

class TargetConfiguration;

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS Target : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setIdentifier(const std::wstring& identifier);

	const std::wstring& getIdentifier() const;

	void setVersion(const std::wstring& version);

	const std::wstring& getVersion() const;

	void addConfiguration(TargetConfiguration* configuration);

	void removeConfiguration(TargetConfiguration* configuration);

	void removeAllConfigurations();

	const RefArray< TargetConfiguration >& getConfigurations() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_identifier;
	std::wstring m_version;
	RefArray< TargetConfiguration > m_configurations;
};

	}
}

