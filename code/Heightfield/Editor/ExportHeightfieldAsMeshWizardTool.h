#ifndef traktor_hf_ExportHeightfieldAsMeshWizardTool_H
#define traktor_hf_ExportHeightfieldAsMeshWizardTool_H

#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS ExportHeightfieldAsMeshWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual uint32_t getFlags() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance);
};

	}
}

#endif	// traktor_hf_ExportHeightfieldAsMeshWizardTool_H
