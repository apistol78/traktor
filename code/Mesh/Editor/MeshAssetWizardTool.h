#ifndef traktor_mesh_MeshAssetWizardTool_H
#define traktor_mesh_MeshAssetWizardTool_H

#include "Editor/WizardTool.h"

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

class T_DLLCLASS MeshAssetWizardTool : public editor::WizardTool
{
	T_RTTI_CLASS(MeshAssetWizardTool)

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, editor::Editor* editor, db::Group* group);
};

	}
}

#endif	// traktor_mesh_MeshAssetWizardTool_H
