#ifndef traktor_editor_Assets_H
#define traktor_editor_Assets_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Root assets.
 * \ingroup Editor
 */
class T_DLLCLASS Assets : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	friend class AssetsPipeline;

	std::vector< Guid > m_dependencies;
};

	}
}

#endif	// traktor_editor_Assets_H
