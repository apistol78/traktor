#ifndef traktor_online_Result_H
#define traktor_online_Result_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

/*! \brief Deferred result.
 *
 * As some operations are expected
 * to take considerable time this
 * class is used to defer results
 * until it's ready.
 *
 * In order to be as simple as possible
 * it's designed to either to used
 * with a callback, polled or blocked.
 *
 * Ref< online::AttachmentResult > result = m_sessionManager->getSaveData()->get(L"MySaveGame");
 *
 * // Either defer result as a callback.
 * result->defer(this, &GameState::deferredLoadFinished, ...);
 *
 * // Or poll until result is ready.
 * if (result->ready())
 * {
 *   if (result->succeeded())
 *     saveGame = result->get< MySaveGame >();
 * }
 *
 * // Or block.
 * saveGame = result->get< MySaveGame >();
 *
 */
class T_DLLCLASS Result : public Object
{
	T_RTTI_CLASS;

public:
	struct IDeferred : public Object
	{
		virtual void dispatch(const Result& result) const = 0;
	};

	template < typename ClassType >
	class DeferredMethod : public IDeferred
	{
	public:
		typedef void (ClassType::*method_t)(const Result& result);

		DeferredMethod(ClassType* object, method_t method)
		:	m_object(object)
		,	m_method(method)
		{
		}

		virtual void dispatch(const Result& result) const T_OVERRIDE T_FINAL
		{
			(m_object->*m_method)(result);
		}

	private:
		ClassType* m_object;
		method_t m_method;
	};

	Result();

	Result(bool succeed);

	void succeed();

	void fail();

	bool ready() const;

	bool succeeded() const;

	void defer(IDeferred* deferred_)
	{
		m_deferred = deferred_;
		deferred();
	}

	template < typename ClassType >
	void defer(ClassType* object, typename DeferredMethod< ClassType >::method_t method)
	{
		defer(new DeferredMethod< ClassType >(object, method));
	}

protected:
	void wait() const;

private:
	bool m_ready;
	bool m_succeeded;
	Ref< IDeferred > m_deferred;

	void deferred();
};

	}
}

#endif	// traktor_online_Result_H
