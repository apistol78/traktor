#ifndef traktor_online_AchievementsEditorPageFactory_H
#define traktor_online_AchievementsEditorPageFactory_H

#include "Editor/IEditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class T_DLLCLASS AchievementsEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_online_AchievementsEditorPageFactory_H
