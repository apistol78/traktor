#ifndef traktor_spark_FontAsset_H
#define traktor_spark_FontAsset_H

#include "Editor/Asset.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS FontAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class FontPipeline;

	resource::Id< render::Shader > m_shader;
	std::wstring m_includeCharacters;
};

	}
}

#endif	// traktor_spark_FontAsset_H
