#ifndef traktor_amalgam_Feature_H
#define traktor_amalgam_Feature_H

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
		bool linux;
		bool osx;
		bool ps3;
		bool win32;
		bool win64;
		bool xbox360;

		Platforms();

		bool serialize(ISerializer& s);
	};

	const std::wstring& getDescription() const { return m_description; }

	const Platforms& getPlatforms() const { return m_platforms; }

	const PropertyGroup* getPipelineProperties() const { return m_pipelineProperties; }

	const PropertyGroup* getRuntimeProperties() const { return m_runtimeProperties; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_description;
	Platforms m_platforms;
	Ref< PropertyGroup > m_pipelineProperties;
	Ref< PropertyGroup > m_runtimeProperties;
};

	}
}

#endif	// traktor_amalgam_Feature_H
