#ifndef traktor_spark_Stage_H
#define traktor_spark_Stage_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace script
	{

class IScriptContext;

	}

	namespace spark
	{

class Character;
class StageInstance;

class T_DLLCLASS Stage : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< StageInstance > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	resource::Id< script::IScriptContext > m_script;
	RefArray< Character > m_characters;
};

	}
}

#endif	// traktor_spark_Stage_H
