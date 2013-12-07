#ifndef traktor_i18n_ImportDictionaryWizardTool_H
#define traktor_i18n_ImportDictionaryWizardTool_H

#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

class T_DLLCLASS ImportDictionaryWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual uint32_t getFlags() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance);
};

	}
}

#endif	// traktor_i18n_ImportDictionaryWizardTool_H
