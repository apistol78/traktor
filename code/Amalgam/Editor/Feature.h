#ifndef traktor_amalgam_Feature_H
#define traktor_amalgam_Feature_H

#include <list>
#include "Core/Guid.h"
#include "Core/Ref.h"
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
	struct Platforms
	{
		bool ios;
		bool linuks;
		bool mobile6;
		bool osx;
		bool ps3;
		bool win32;
		bool win64;
		bool xbox360;
		bool emscripten;
		bool android;

		Platforms();

		void serialize(ISerializer& s);
	};

	Feature();

	const std::wstring& getDescription() const { return m_description; }

	int32_t getPriority() const { return m_priority; }

	const Platforms& getPlatforms() const { return m_platforms; }

	const PropertyGroup* getPipelineProperties() const { return m_pipelineProperties; }

	const PropertyGroup* getMigrateProperties() const { return m_migrateProperties; }

	const PropertyGroup* getRuntimeProperties() const { return m_runtimeProperties; }

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_description;
	int32_t m_priority;
	Platforms m_platforms;
	Ref< PropertyGroup > m_pipelineProperties;
	Ref< PropertyGroup > m_migrateProperties;
	Ref< PropertyGroup > m_runtimeProperties;
};

	}
}

#endif	// traktor_amalgam_Feature_H
