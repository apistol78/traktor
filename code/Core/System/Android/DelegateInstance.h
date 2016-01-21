#ifndef traktor_DelegateInstance_H
#define traktor_DelegateInstance_H

#include <vector>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct android_app;
struct AInputEvent;
struct ANativeActivity;

namespace traktor
{

/*! \brief Android delegable instance.
 * \ingroup Core
 */
class T_DLLCLASS DelegateInstance
{
public:
	struct IDelegate
	{
		virtual void notifyHandleCommand(DelegateInstance* instance, int32_t cmd) {}

		virtual void notifyHandleInput(DelegateInstance* instance, AInputEvent* event) {}
	};

	virtual struct android_app* getApplication() = 0;

	virtual struct ANativeActivity* getActivity() = 0;

	void addDelegate(IDelegate* delegate);

	void removeDelegate(IDelegate* delegate);

	virtual void handleCommand(int32_t cmd);

	virtual void handleInput(AInputEvent* event);

private:
	std::vector< IDelegate* > m_delegates;
};

}

#endif	// traktor_DelegateInstance_H
