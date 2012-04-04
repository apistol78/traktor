#ifndef traktor_parade_LayerData_H
#define traktor_parade_LayerData_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace script
	{

class IScriptContext;

	}

	namespace parade
	{

class Layer;

class T_DLLCLASS LayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(amalgam::IEnvironment* environment) const = 0;

	virtual bool serialize(ISerializer& s);

protected:
	friend class StagePipeline;

	std::wstring m_name;
	resource::Proxy< script::IScriptContext > m_script;
};

	}
}

#endif	// traktor_parade_LayerData_H
