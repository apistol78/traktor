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
		virtual void notifyHandleCommand(struct android_app* app, int32_t cmd) {}

		virtual void notifyHandleInput(struct android_app* app, AInputEvent* event) {}
	};

	void addDelegate(IDelegate* delegate);

	void removeDelegate(IDelegate* delegate);

	virtual void handleCommand(struct android_app* app, int32_t cmd);

	virtual void handleInput(struct android_app* app, AInputEvent* event);

private:
	std::vector< IDelegate* > m_delegates;
};

}

#endif	// traktor_DelegateInstance_H
