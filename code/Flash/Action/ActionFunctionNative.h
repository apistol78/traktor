#ifndef traktor_flash_ActionFunctionNative_H
#define traktor_flash_ActionFunctionNative_H

#include "Flash/Action/ActionFunction.h"

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

class ActionContext;

/*! \brief Native call arguments.
 * \ingroup Flash
 */
struct CallArgs
{
	const IActionVM* vm;
	ActionContext* context;
	ActionObject* self;
	std::vector< ActionValue > args;
	ActionValue ret;
};

#define T_REQUIRES_X_ARGS(ca, n) \
	if (ca.args.size() != (n)) \
		return ActionValue();

/*! \brief Callable function.
 * \ingroup Flash
 */
class CallFnc : public Object
{
public:
	virtual void call(CallArgs& ca) = 0;
};

/*! \brief Static function.
 * \ingroup Flash
 */
class FunctionFnc : public CallFnc
{
public:
	typedef void (*F)(CallArgs& ca);

	FunctionFnc(F function)
	:	m_function(function)
	{
	}

	virtual void call(CallArgs& ca)
	{
		if (m_function)
			(*m_function)(ca);
	}

private:
	F m_function;
};

/*! \brief Class method.
 * \ingroup Flash
 */
template < typename T >
class MethodFnc : public CallFnc
{
public:
	typedef void (T::*M)(CallArgs& ca);

	MethodFnc< T >(T* object, M method)
	:	m_object(object)
	,	m_method(method)
	{
	}

	virtual void call(CallArgs& ca)
	{
		if (m_object && m_method)
			(m_object->*m_method)(ca);
	}

private:
	T* m_object;
	M m_method;
};

/*! \brief ActionScript native function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunctionNative : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunctionNative(CallFnc* callFnc);

	virtual ActionValue call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self);

private:
	Ref< CallFnc > m_callFnc;
};

/*! \brief Create native function.
 * \ingroup Flash
 */
inline Ref< ActionFunctionNative > createNativeFunction(FunctionFnc::F function)
{
	return new ActionFunctionNative(new FunctionFnc(function));
}

/*! \brief Create native function as value.
 * \ingroup Flash
 */
inline ActionValue createNativeFunctionValue(FunctionFnc::F function)
{
	return ActionValue::fromObject(createNativeFunction(function));
}

/*! \brief Create native function.
 * \ingroup Flash
 */
template < typename TargetType >
inline ActionFunctionNative* createNativeFunction(TargetType* object, typename MethodFnc< TargetType >::M method)
{
	return new ActionFunctionNative(new MethodFnc< TargetType >(object, method));
}

/*! \brief Create native function as value.
 * \ingroup Flash
 */
template < typename TargetType >
inline ActionValue createNativeFunctionValue(TargetType* object, typename MethodFnc< TargetType >::M method)
{
	return ActionValue::fromObject(createNativeFunction(object, method));
}

	}
}

#endif	// traktor_flash_ActionFunctionNative_H
