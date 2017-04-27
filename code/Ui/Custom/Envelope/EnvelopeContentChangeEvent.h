/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_EnvelopeContentChangeEvent_H
#define traktor_ui_custom_EnvelopeContentChangeEvent_H

#include "Ui/Events/ContentChangeEvent.h"

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

class EnvelopeKey;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS EnvelopeContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	EnvelopeContentChangeEvent(EventSubject* sender, EnvelopeKey* key);
	
	EnvelopeKey* getKey() const;

private:
	Ref< EnvelopeKey > m_key;
};

		}
	}
}

#endif	// traktor_ui_custom_EnvelopeContentChangeEvent_H
