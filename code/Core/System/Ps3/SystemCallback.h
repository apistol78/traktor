/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SystemCallback_H
#define traktor_SystemCallback_H

#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief System callback manager.
 * \ingroup Core
 *
 * \note
 * This is only available for PS3 and it's
 * peculiar system callbacks.
 */
class T_DLLCLASS SystemCallback : public ISingleton
{
public:
	typedef void (*callback_t)(uint64_t status, uint64_t param);

	static SystemCallback& getInstance();

	void add(callback_t callback);

	void update();

protected:
	SystemCallback();

	virtual ~SystemCallback();

	virtual void destroy();

private:
	std::vector< callback_t > m_callbacks;

	static void systemCallback(uint64_t status, uint64_t param, void* userData);
};

}

#endif	// traktor_SystemCallback_H
