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
#include <map>
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/StaticVector.h"
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

class PropertyGroup;

	namespace runtime
	{

/*! Target feature description.
 * \ingroup Runtime
 */
class T_DLLCLASS Feature : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Platform
	{
		Guid platform;
		std::wstring executableFile;
		Ref< PropertyGroup > deploy;

		void serialize(ISerializer& s);
	};

	Feature();

	void setDescription(const std::wstring& description);

	const std::wstring& getDescription() const { return m_description; }

	void setPriority(int32_t priority);

	int32_t getPriority() const { return m_priority; }

	void addPlatform(const Platform& platform);

	bool removePlatform(const Guid& id);

	std::list< Platform >& getPlatforms() { return m_platforms; }

	const std::list< Platform >& getPlatforms() const { return m_platforms; }

	Platform* getPlatform(const Guid& id);

	const Platform* getPlatform(const Guid& id) const;

	const PropertyGroup* getPipelineProperties() const { return m_pipelineProperties; }

	const PropertyGroup* getMigrateProperties() const { return m_migrateProperties; }

	const PropertyGroup* getRuntimeProperties() const { return m_runtimeProperties; }

	const std::map< std::wstring, std::wstring >& getEnvironment() const { return m_environment; }

	const std::list< Guid >& getDependencies() const { return m_dependencies; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_description;
	int32_t m_priority;
	std::list< Platform > m_platforms;
	Ref< PropertyGroup > m_pipelineProperties;
	Ref< PropertyGroup > m_migrateProperties;
	Ref< PropertyGroup > m_runtimeProperties;
	std::map< std::wstring, std::wstring > m_environment;
	std::list< Guid > m_dependencies;
};

	}
}

