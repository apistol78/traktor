/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_EnvelopeKey_H
#define traktor_ui_custom_EnvelopeKey_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	EnvelopeKey(float T, float value, bool fixedT = false, bool fixedValue = false);

	void setT(float T) { m_T = T; }

	float getT() const { return m_T; }

	void setValue(float value) { m_value = value; }

	float getValue() const { return m_value; }

	bool isFixedT() const { return m_fixedT; }

	bool isFixedValue() const { return m_fixedValue; }

private:
	float m_T;
	float m_value;
	bool m_fixedT;
	bool m_fixedValue;
};

		}
	}
}

#endif	// traktor_ui_custom_EnvelopeKey_H
