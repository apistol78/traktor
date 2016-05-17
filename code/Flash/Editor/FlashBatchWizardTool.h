#ifndef traktor_flash_FlashBatchWizardTool_H
#define traktor_flash_FlashBatchWizardTool_H

#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class T_DLLCLASS FlashBatchWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual uint32_t getFlags() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_FlashBatchWizardTool_H
