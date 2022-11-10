/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Guid.h"
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

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS TargetConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetConfiguration();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setPlatform(const Guid& platform);

	const Guid& getPlatform() const;

	void setIcon(const std::wstring& icon);

	const std::wstring& getIcon() const;

	void addFeature(const Guid& feature);

	void removeFeature(const Guid& feature);

	bool haveFeature(const Guid& feature) const;

	const std::list< Guid >& getFeatures() const;

	void setRoot(const Guid& root);

	const Guid& getRoot() const;

	void setStartup(const Guid& startup);

	const Guid& getStartup() const;

	void setDefaultInput(const Guid& defaultInput);

	const Guid& getDefaultInput() const;

	void setOnlineConfig(const Guid& onlineConfig);

	const Guid& getOnlineConfig() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	Guid m_platform;
	std::wstring m_icon;
	std::list< Guid > m_features;
	Guid m_root;
	Guid m_startup;
	Guid m_defaultInput;
	Guid m_onlineConfig;
};

	}
}

