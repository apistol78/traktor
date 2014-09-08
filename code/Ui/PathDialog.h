#ifndef traktor_ui_PathDialog_H
#define traktor_ui_PathDialog_H

#include "Core/Io/Path.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IPathDialog;
class Widget;

/*! \brief Path dialog.
 * \ingroup UI
 */
class T_DLLCLASS PathDialog : public EventSubject
{
	T_RTTI_CLASS;

public:
	PathDialog();

	virtual ~PathDialog();

	bool create(Widget* parent, const std::wstring& title);

	void destroy();

	int showModal(Path& outPath);

private:
	IPathDialog* m_pathDialog;
};

	}
}

#endif	// traktor_ui_PathDialog_H
