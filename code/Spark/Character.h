#ifndef traktor_spark_Character_H
#define traktor_spark_Character_H

#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"

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

	namespace spark
	{

class CharacterInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Character : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const = 0;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
	Matrix33 m_transform;
};

	}
}

#endif	// traktor_spark_Character_H
