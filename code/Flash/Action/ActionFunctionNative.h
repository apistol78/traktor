#ifndef traktor_flash_ActionFunctionNative_H
#define traktor_flash_ActionFunctionNative_H

#include "Core/Meta/Traits.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"

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
	ActionContext* context;
	ActionObject* self;
	ActionValueArray args;
	ActionValue ret;
};


/*! \brief Native function wrapper.
 * \ingroup Flash
 */
struct T_DLLCLASS INativeFunction : public Object
{
	virtual void call(CallArgs& ca) = 0;
};


template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct ActionValueCast { };

template < >
struct ActionValueCast< bool, false >
{
	static ActionValue set(bool v) { return ActionValue(v); }
	static bool get(const ActionValue& av) { return av.getBooleanSafe(); }
};

template < >
struct ActionValueCast< int32_t, false >
{
	static ActionValue set(int32_t v) { return ActionValue(avm_number_t(v)); }
	static int32_t get(const ActionValue& av) { return int32_t(av.getNumberSafe()); }
};

template < >
struct ActionValueCast< float, false >
{
	static ActionValue set(float v) { return ActionValue(avm_number_t(v)); }
	static float get(const ActionValue& av) { return float(av.getNumberSafe()); }
};

template < >
struct ActionValueCast< double, false >
{
	static ActionValue set(double v) { return ActionValue(avm_number_t(v)); }
	static double get(const ActionValue& av) { return double(av.getNumberSafe()); }
};

template < >
struct ActionValueCast< std::wstring, false >
{
	static ActionValue set(const std::wstring& v) { return ActionValue(v); }
	static std::wstring get(const ActionValue& av) { return av.getStringSafe(); }
};

template < >
struct ActionValueCast< const std::wstring&, false >
{
	static ActionValue set(const std::wstring& v) { return ActionValue(v); }
	static std::wstring get(const ActionValue& av) { return av.getStringSafe(); }
};

template < typename Type >
struct ActionValueCast< Ref< Type >, false >
{
	static ActionValue set(const Ref< Type >& v) { return ActionValue(v); }
	static Ref< Type > get(const ActionValue& value) { return value.getObjectSafe< Type >(); }
};

template < typename Type >
struct ActionValueCast< Type, true >
{
	static ActionValue set(Type v) { return ActionValue(v); }
	static Type get(const ActionValue& value) { return value.getObjectSafe< typename IsPointer< Type >::base_t >(); }
};


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
		ca.ret = ActionValueCast< ReturnType >::set((m_object->*m_method)(
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
		ca.ret = ActionValueCast< ReturnType >::set((m_object->*m_method)(
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
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 1);
		(m_object->*m_method)(
			checked_type_cast< SelfClassType* >(ca.self),
			ActionValueCast< Argument1Type >::get(ca.args[0])
		);
	}
};


template <
	typename ClassType,
	typename ReturnType
>
struct MethodNativeFunction_self_0 : public INativeFunction
{
	ReturnType (ClassType::*m_method)();

	virtual void call(CallArgs& ca)
	{
		ca.ret = ActionValueCast< ReturnType >::set(
			(checked_type_cast< ClassType*, false >(ca.self)->*m_method)()
		);
	}
};

template <
	typename ClassType
>
struct MethodNativeFunction_self_0 < ClassType, void > : public INativeFunction
{
	void (ClassType::*m_method)();

	virtual void call(CallArgs& ca)
	{
		(checked_type_cast< ClassType*, false >(ca.self)->*m_method)();
	}
};


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodNativeFunction_self_1 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 1);
		ca.ret = ActionValueCast< ReturnType >::set(
			(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.args[0])
			)
		);
	}
};

template <
	typename ClassType,
	typename Argument1Type
>
struct MethodNativeFunction_self_1 < ClassType, void, Argument1Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 1);
		(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
			ActionValueCast< Argument1Type >::get(ca.args[0])
		);
	}
};


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_self_2 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 2);
		ca.ret = ActionValueCast< ReturnType >::set(
			(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.args[1])
			)
		);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_self_2 < ClassType, void, Argument1Type, Argument2Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 2);
		(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
			ActionValueCast< Argument1Type >::get(ca.args[0]),
			ActionValueCast< Argument2Type >::get(ca.args[1])
		);
	}
};


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_self_3 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 3);
		ca.ret = ActionValueCast< ReturnType >::set(
			(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.args[2])
			)
		);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_self_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() == 3);
		(checked_type_cast< ClassType*, false >(ca.self)->*m_method)(
			ActionValueCast< Argument1Type >::get(ca.args[0]),
			ActionValueCast< Argument2Type >::get(ca.args[1]),
			ActionValueCast< Argument3Type >::get(ca.args[2])
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

	virtual ActionValue call(ActionContext* context, ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self);

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


template <
	typename ClassType,
	typename ReturnType
>
Ref< ActionFunctionNative > createNativeFunction(ReturnType (ClassType::*method)())
{
	Ref< MethodNativeFunction_self_0< ClassType, ReturnType > > nf = new MethodNativeFunction_self_0< ClassType, ReturnType >();
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
Ref< ActionFunctionNative > createNativeFunction(ReturnType (ClassType::*method)(Argument1Type))
{
	Ref< MethodNativeFunction_self_1< ClassType, ReturnType, Argument1Type > > nf = new MethodNativeFunction_self_1< ClassType, ReturnType, Argument1Type >();
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
Ref< ActionFunctionNative > createNativeFunction(ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
{
	Ref< MethodNativeFunction_self_2< ClassType, ReturnType, Argument1Type, Argument2Type > > nf = new MethodNativeFunction_self_2< ClassType, ReturnType, Argument1Type, Argument2Type >();
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
Ref< ActionFunctionNative > createNativeFunction(ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type))
{
	Ref< MethodNativeFunction_self_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type > > nf = new MethodNativeFunction_self_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >();
	nf->m_method = method;
	return new ActionFunctionNative(nf);
}


	}
}

#endif	// traktor_flash_ActionFunctionNative_H
