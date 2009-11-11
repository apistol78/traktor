#ifndef traktor_render_ShaderGraphEditorPageFactory_H
#define traktor_render_ShaderGraphEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS ShaderGraphEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS(ShaderGraphEditorPageFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_render_ShaderGraphEditorPageFactory_H
