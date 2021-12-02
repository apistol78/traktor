#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace ui
	{

class Command;

	}

    namespace editor
	{

/*!
 * \ingroup Editor
 */
class T_DLLCLASS PropertiesView : public ui::Widget
{
	T_RTTI_CLASS;

public:
	/*! Attach object to property view. */
	virtual void setPropertyObject(ISerializable* object) = 0;

	/*! Handle command. */
	virtual bool handleCommand(const ui::Command& command) = 0;
};

    }
}
