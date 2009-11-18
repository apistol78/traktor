#ifndef traktor_render_IContext_H
#define traktor_render_IContext_H

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

#define T_CONTEXT_SCOPE(context) IContext::Scope __scope__(context);

/*! \brief OpenGL context interface.
 * \ingroup OGL
 */
class IContext : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Scoped enter/leave helper.
	 * \ingroup OGL
	 */
	struct Scope
	{
		IContext* m_context;

		Scope(IContext* context)
		:	m_context(context)
		{
			m_context->enter();
		}

		virtual ~Scope()
		{
			m_context->leave();
		}
	};

	/*! \brief Delete callback. 
	 * \ingroup OGL
	 *
	 * These are enqueued in the context
	 * and are invoked as soon as it's
	 * safe to actually delete resources.
	 */
	struct IDeleteCallback
	{
		virtual void deleteResource() = 0;
	};

	virtual bool enter() = 0;

	virtual void leave() = 0;

	virtual void deleteResource(IDeleteCallback* callback) = 0;

	virtual void deleteResources() = 0;
};

	}
}

#endif	// traktor_render_IContext_H
