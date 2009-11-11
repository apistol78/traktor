#ifndef traktor_ui_custom_EnvelopeControl_H
#define traktor_ui_custom_EnvelopeControl_H

#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class EnvelopeEvaluator;
class EnvelopeKey;

/*! \brief Envelope control.
 * \ingroup UIC
 */
class T_DLLCLASS EnvelopeControl : public Widget
{
	T_RTTI_CLASS(EnvelopeControl)

public:
	bool create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue = -1.0f, float maxValue = 1.0f, int style = WsNone);

	void insertKey(EnvelopeKey* key);

	const RefArray< EnvelopeKey >& getKeys() const;

	void addChangeEventHandler(EventHandler* eventHandler);

private:
	Ref< EnvelopeEvaluator > m_evaluator;
	float m_minValue;
	float m_maxValue;
	RefArray< EnvelopeKey > m_keys;
	Rect m_rcEnv;
	Ref< EnvelopeKey > m_selectedKey;

	void eventButtonDown(Event* e);

	void eventButtonUp(Event* e);

	void eventMouseMove(Event* e);

	void eventPaint(Event* e);
};

		}
	}
}

#endif	// traktor_ui_custom_EnvelopeControl_H
