#ifndef traktor_amalgam_Feature_H
#define traktor_amalgam_Feature_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

/*! \brief Target feature description.
 * \ingroup Amalgam
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

	const std::wstring& getDescription() const { return m_description; }

	int32_t getPriority() const { return m_priority; }

	const Platform* getPlatform(const Guid& id) const;

	const PropertyGroup* getPipelineProperties() const { return m_pipelineProperties; }

	const PropertyGroup* getMigrateProperties() const { return m_migrateProperties; }

	const PropertyGroup* getRuntimeProperties() const { return m_runtimeProperties; }

	const std::map< std::wstring, std::wstring >& getEnvironment() const { return m_environment; }

	const std::list< Guid >& getDependencies() const { return m_dependencies; }

	virtual void serialize(ISerializer& s);

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

#endif	// traktor_amalgam_Feature_H
