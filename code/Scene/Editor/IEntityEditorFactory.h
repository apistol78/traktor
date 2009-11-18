#ifndef traktor_scene_IEntityEditorFactory_H
#define traktor_scene_IEntityEditorFactory_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class IEntityEditor;
class SceneEditorContext;

/*! \brief
 * \ingroup Scene
 */
class T_DLLCLASS IEntityEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get supported set of entities.
	 *
	 * \return Type Set of EntityData types.
	 */
	virtual const TypeInfoSet getEntityDataTypes() const = 0;

	/*! \brief Create entity editor.
	 *
	 * \param context Scene editor context.
	 * \param entityDataType Entity data type.
	 * \return Entity editor instance.
	 */
	virtual Ref< IEntityEditor > createEntityEditor(SceneEditorContext* context, const TypeInfo& entityDataType) const = 0;
};

	}
}

#endif	// traktor_scene_IEntityEditorFactory_H
