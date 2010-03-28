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


/*! \brief Native function wrapper.
 * \ingroup Flash
 */
struct INativeFunction : public Object
{
	virtual void call(CallArgs& ca) = 0;
};


template < typename Type >
struct ActionValueCast { };

template < >
struct ActionValueCast< bool > { static bool get(const ActionValue& av) { return av.getBooleanSafe(); } };

template < >
struct ActionValueCast< int32_t > { static int32_t get(const ActionValue& av) { return int32_t(av.getNumberSafe()); } };

template < >
struct ActionValueCast< float > { static float get(const ActionValue& av) { return float(av.getNumberSafe()); } };

template < >
struct ActionValueCast< double > { static double get(const ActionValue& av) { return double(av.getNumberSafe()); } };

template < >
struct ActionValueCast< std::wstring > { static std::wstring get(const ActionValue& av) { return av.getStringSafe(); } };


template <
	typename CallClassType
>
struct MethodNativeFunction : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(CallArgs&);

	virtual void call(CallArgs& ca)
	{
		(m_object->*m_method)(ca);
	}
};


template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType
>
struct MethodNativeFunction_0 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*) const;

	virtual void call(CallArgs& ca)
	{
		ca.ret = ActionValue((m_object->*m_method)(
			checked_type_cast< SelfClassType* >(ca.self)
		));
	}
};

template <
	typename CallClassType,
	typename SelfClassType
>
struct MethodNativeFunction_0 < CallClassType, SelfClassType, void > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*) const;

	virtual void call(CallArgs& ca)
	{
		(m_object->*m_method)(
			checked_type_cast< SelfClassType* >(ca.self)
		);
	}
};


template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodNativeFunction_1 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 1);
		ca.ret = ActionValue((m_object->*m_method)(
			checked_type_cast< SelfClassType* >(ca.self),
			ActionValueCast< Argument1Type >::get(ca.args[0])
		));
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type
>
struct MethodNativeFunction_1 < CallClassType, SelfClassType, void, Argument1Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 1);
		(m_object->*m_method)(
			checked_type_cast< SelfClassType* >(ca.self),
			ActionValueCast< Argument1Type >::get(ca.args[0])
		);
	}
};


/*! \brief ActionScript native function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunctionNative : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunctionNative(INativeFunction* nativeFunction);

	virtual ActionValue call(const IActionVM* vm, ActionContext* context, ActionObject* self, const std::vector< ActionValue >& args);

	virtual ActionValue call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self);

private:
	Ref< INativeFunction > m_nativeFunction;
};


template <
	typename CallClassType
>
Ref< ActionFunctionNative > createNativeFunction(CallClassType* object, void (CallClassType::*method)(CallArgs&))
{
	Ref< MethodNativeFunction< CallClassType > > nf = new MethodNativeFunction< CallClassType >();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType
>
Ref< ActionFunctionNative > createNativeFunction(CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*) const)
{
	Ref< MethodNativeFunction_0< CallClassType, SelfClassType, ReturnType > > nf = new MethodNativeFunction_0< CallClassType, SelfClassType, ReturnType >();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type
>
Ref< ActionFunctionNative > createNativeFunction(CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type) const)
{
	Ref< MethodNativeFunction_1< CallClassType, SelfClassType, ReturnType, Argument1Type > > nf = new MethodNativeFunction_1< CallClassType, SelfClassType, ReturnType, Argument1Type >();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


	}
}

#endif	// traktor_flash_ActionFunctionNative_H
