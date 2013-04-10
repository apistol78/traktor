#ifndef traktor_sound_SoundBatchWizardTool_H
#define traktor_sound_SoundBatchWizardTool_H

#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundBatchWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual uint32_t getFlags() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance);
};

	}
}

#endif	// traktor_sound_SoundBatchWizardTool_H
