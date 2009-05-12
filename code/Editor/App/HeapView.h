#ifndef traktor_editor_HeapView_H
#define traktor_editor_HeapView_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class ProfileControl;

		}
	}

	namespace editor
	{

class HeapView : public ui::Container
{
	T_RTTI_CLASS(HeapView)

public:
	bool create(ui::Widget* widget);

	void destroy();

private:
	Ref< ui::custom::ProfileControl > m_profileControl;

	void eventDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_HeapView_H
