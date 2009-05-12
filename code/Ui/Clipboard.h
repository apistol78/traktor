#ifndef traktor_ui_Clipboard_H
#define traktor_ui_Clipboard_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class DeepClone;
class Serializable;

	namespace ui
	{

/*! \brief Clipboard
 * \ingroup UI
 */
class T_DLLCLASS Clipboard : public Object
{
	T_RTTI_CLASS(Clipboard)

public:
	void setObject(Serializable* object);

	Serializable* getObject();

private:
	Ref< DeepClone > m_clone;
};

	}
}

#endif	// traktor_ui_Clipboard_H
