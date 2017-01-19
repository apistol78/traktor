#ifndef traktor_editor_ISettingsPage_H
#define traktor_editor_ISettingsPage_H

#include <list>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class Container;
class Command;

	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class T_DLLCLASS ISettingsPage : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) = 0;

	virtual void destroy() = 0;

	virtual bool apply(PropertyGroup* settings) = 0;
};

	}
}

#endif	// traktor_editor_ISettingsPage_H
