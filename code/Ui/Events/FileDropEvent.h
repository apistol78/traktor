#ifndef traktor_ui_FileDropEvent_H
#define traktor_ui_FileDropEvent_H

#include "Ui/Event.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
	
/*! \brief File drag'n'drop event.
 * \ingroup UI
 */
class T_DLLCLASS FileDropEvent : public Event
{
	T_RTTI_CLASS(FileDropEvent)
	
public:
	FileDropEvent(EventSubject* sender, Object* item, const std::vector< Path >& files);

	const std::vector< Path >& getFiles() const;
	
private:
	std::vector< Path > m_files;
};
	
	}
}

#endif	// traktor_ui_FileDropEvent_H
