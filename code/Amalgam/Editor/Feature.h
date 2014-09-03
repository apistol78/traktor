#ifndef traktor_amalgam_Feature_H
#define traktor_amalgam_Feature_H

#include <list>
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

class Feature : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Platform
	{
		Guid platform;
		std::list< std::wstring > deployFiles;

		void serialize(ISerializer& s);
	};

	Feature();

	const std::wstring& getDescription() const { return m_description; }

	int32_t getPriority() const { return m_priority; }

	const Platform* getPlatform(const Guid& id) const;

	const PropertyGroup* getPipelineProperties() const { return m_pipelineProperties; }

	const PropertyGroup* getMigrateProperties() const { return m_migrateProperties; }

	const PropertyGroup* getRuntimeProperties() const { return m_runtimeProperties; }

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_description;
	int32_t m_priority;
	std::list< Platform > m_platforms;
	Ref< PropertyGroup > m_pipelineProperties;
	Ref< PropertyGroup > m_migrateProperties;
	Ref< PropertyGroup > m_runtimeProperties;
};

	}
}

#endif	// traktor_amalgam_Feature_H
