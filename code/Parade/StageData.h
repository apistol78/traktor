#ifndef traktor_parade_StageData_H
#define traktor_parade_StageData_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class LayerData;
class Stage;

class T_DLLCLASS StageData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< Stage > createInstance(amalgam::IEnvironment* environment, const Object* params) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StagePipeline;

	RefArray< LayerData > m_layers;
	resource::Id< script::IScriptContext > m_script;
	std::map< std::wstring, Guid > m_transitions;
	Guid m_resourceBundle;
};

	}
}

#endif	// traktor_parade_StageData_H
