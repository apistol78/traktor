#ifndef traktor_amalgam_DeployTool_H
#define traktor_amalgam_DeployTool_H

#include <map>
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

/*! \brief Deployment tool description.
 * \ingroup Amalgam
 */
class T_DLLCLASS DeployTool : public ISerializable
{
	T_RTTI_CLASS;

public:
	const std::wstring& getExecutable() const;

	const std::map< std::wstring, std::wstring >& getEnvironment() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_executable;
	std::map< std::wstring, std::wstring > m_environment;
};

	}
}

#endif	// traktor_amalgam_DeployTool_H
