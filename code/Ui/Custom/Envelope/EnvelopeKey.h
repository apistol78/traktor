#ifndef traktor_ui_custom_EnvelopeKey_H
#define traktor_ui_custom_EnvelopeKey_H

#include "Core/Object.h"

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

/*! \brief Envelope key.
 * \ingroup UIC
 */
class T_DLLCLASS EnvelopeKey : public Object
{
	T_RTTI_CLASS;

public:
	EnvelopeKey(float T = 0.0f, float value = 0.0f);

	void setT(float T);

	float getT() const;

	void setValue(float value);

	float getValue() const;

private:
	float m_T;
	float m_value;
};

		}
	}
}

#endif	// traktor_ui_custom_EnvelopeKey_H
