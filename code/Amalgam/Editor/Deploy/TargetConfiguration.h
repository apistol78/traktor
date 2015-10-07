#ifndef traktor_amalgam_TargetConfiguration_H
#define traktor_amalgam_TargetConfiguration_H

#include <list>
#include "Core/Guid.h"
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
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
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

	virtual void serialize(ISerializer& s);

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

#endif	// traktor_amalgam_TargetConfiguration_H
