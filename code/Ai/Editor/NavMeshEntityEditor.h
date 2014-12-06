#ifndef traktor_ai_NavMeshEntityEditor_H
#define traktor_ai_NavMeshEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

namespace traktor
{
	namespace ai
	{

/*! \brief Navigation mesh entity editor.
 * \ingroup AI
 */
class NavMeshEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	NavMeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;
};

	}
}

#endif	// traktor_ai_NavMeshEntityEditor_H
