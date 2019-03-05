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

namespace traktor
{
	namespace runtime
	{

/*! \brief Deployment tool description.
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
}

