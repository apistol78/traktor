/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Number_H
#define traktor_flash_Number_H

#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript number wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Number : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Number(float value);

	void set(float value) { m_value = value; }

	float get() const { return m_value; }

private:
	float m_value;
};

	}
}

#endif	// traktor_flash_Number_H
