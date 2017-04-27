/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Signal_H
#define traktor_Signal_H

#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Thread signal.
 * \ingroup Core
 */
class T_DLLCLASS Signal : public IWaitable
{
public:
	Signal();

	virtual ~Signal();
	
	void set();

	void reset();

	virtual bool wait(int32_t timeout = -1);
	
private:
	void* m_handle;
};

}

#endif	// traktor_Signal_H
