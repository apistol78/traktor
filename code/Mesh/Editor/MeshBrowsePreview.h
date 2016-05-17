#ifndef traktor_render_MeshBrowsePreview_H
#define traktor_render_MeshBrowsePreview_H

#include "Editor/IBrowsePreview.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshBrowsePreview : public editor::IBrowsePreview
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::Bitmap > generate(const editor::IEditor* editor, db::Instance* instance) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_MeshBrowsePreview_H
