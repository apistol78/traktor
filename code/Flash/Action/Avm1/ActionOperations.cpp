#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Timer/Timer.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionDictionary.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/Avm1/ActionFunction1.h"
#include "Flash/Action/Avm1/ActionFunction2.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/ActionVMImage1.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Avm1/Classes/AsArray.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Avm1/Classes/AsMovieClip.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if defined(__IOS__) || defined(__ANDROID__)

template < int Size >
struct UnalignedView {};

template < >
struct UnalignedView< 1 > { uint8_t value; };

template < >
struct UnalignedView< 2 > { uint16_t value __attribute__(( packed )); };

template < >
struct UnalignedView< 4 > { uint32_t value __attribute__(( packed )); };

template < typename T >
T unalignedRead(const void* ptr)
{
	return ((UnalignedView< sizeof(T) >*)ptr)->value;
}

#else

template < typename T >
T unalignedRead(const void *ptr)
{
	return *(T T_UNALIGNED *)(ptr);
}

#endif

//#define T_IF_TRACE(x) \
//	{ if (state.trace) { x } }

#define T_IF_TRACE(x)

int32_t parseIndex(const char* str)
{
	int32_t value = 0;
	for (; *str; ++str)
	{
		if (*str < '0' || *str > '9')
			return -1;

		value = (value * 10) + int32_t(*str - '0');
	}
	return value;
}

bool getMemberOrProperty(ExecutionState& state, ActionObject* self, int32_t variableId, ActionValue& outValue)
{
	ActionValueStack& stack = state.frame->getStack();
	Ref< ActionFunction > propertyGet;

	if (self->getPropertyGet(variableId, propertyGet))
	{
		stack.push(ActionValue(avm_number_t(0)));
		outValue = propertyGet->call(state.frame, self);
		return true;
	}

	return self->getMember(variableId, outValue);
}

void setMemberOrProperty(ExecutionState& state, ActionObject* self, int32_t variableId, const ActionValue& value)
{
	ActionValueStack& stack = state.frame->getStack();
	Ref< ActionFunction > propertySet;

	if (self->getPropertySet(variableId, propertySet))
	{
		stack.push(value);
		stack.push(ActionValue(avm_number_t(1)));
		propertySet->call(state.frame, self);
	}
	else
		self->setMember(variableId, value);
}

ActionValue getVariable(ExecutionState& state, const ActionValue& variable)
{
	ActionValue value;

	// Resolve variable numeric id.
	int32_t variableId = -1;
	if (variable.isString())
		variableId = variable.getStringId();
	if (variableId < 0)
		variableId = state.context->getString(variable.getString());

	// Get "with" instance member first.
	if (state.with)
	{
		if (getMemberOrProperty(state, state.with, variableId, value))
			return value;
	}

	// Get frame instance member.
	if (state.frame->getVariable(variableId, value))
		return value;

	// Get frame scope variable.
	if (state.frame->getScopeVariable(variableId, value))
		return value;

	// Get movie clip member.
	if (state.movieClip)
	{
		FlashCharacterInstance* movieClip = state.movieClip;
		while (movieClip)
		{
			ActionObject* movieClipAS = movieClip->getAsObject(state.context);
			T_ASSERT (movieClipAS);

			if (getMemberOrProperty(state, movieClipAS, variableId, value))
				return value;

			movieClip = movieClip->getParent();
		}
	}

	// Get from self instance.
	if (state.self)
	{
		if (getMemberOrProperty(state, state.self, variableId, value))
			return value;
	}

	// Variable path.
	{
		std::string variableName = variable.getString();
		StringSplit< std::string > variableNameSplit(variableName, ".");

		Ref< ActionObject > object = state.global;
		for (StringSplit< std::string >::const_iterator i = variableNameSplit.begin(); i != variableNameSplit.end(); ++i)
		{
			if (!object->getLocalMember(*i, value))
				break;

			if ((object = value.getObject()) == 0)
				break;
		}
	}

	return value;
}

void opx_nextFrame(ExecutionState& state)
{
	state.movieClip->gotoNext();
}

void opx_prevFrame(ExecutionState& state)
{
	state.movieClip->gotoPrevious();
}

void opx_play(ExecutionState& state)
{
	state.movieClip->setPlaying(true);
}

void opx_stop(ExecutionState& state)
{
	state.movieClip->setPlaying(false);
}

void opx_toggleQuality(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopToggleQuality: Not implemented"  << Endl;
	)
}

void opx_stopSounds(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopStopSounds: Not implemented"  << Endl;
	)
}

void opx_add(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);

	if (number1.isNumeric() && number2.isNumeric())
		number1 = ActionValue(number1.getNumber() + number2.getNumber());
	else
		number1 = ActionValue();

	stack.drop(1);
}

void opx_subtract(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);
	number1 = number1 - number2;
	stack.drop(1);
}

void opx_multiply(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);
	number1 = number1 * number2;
	stack.drop(1);
}

void opx_divide(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);

	if (number1.isNumeric() && number2.isNumeric())
		number1 = ActionValue(number1.getNumber() / number2.getNumber());
	else
		number1 = ActionValue();

	stack.drop(1);
}

void opx_equal(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);

	if (number1.isNumeric() && number2.isNumeric())
		number1 = ActionValue(bool(number1.getNumber() == number2.getNumber()));
	else
		number1 = ActionValue();

	stack.drop(1);
}

void opx_lessThan(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue& number2 = stack.top(0);
	ActionValue& number1 = stack.top(-1);

	if (number1.isNumeric() && number2.isNumeric())
		number1 = ActionValue(bool(number1.getNumber() < number2.getNumber()));
	else
		number1 = ActionValue();

	stack.drop(1);
}

void opx_logicalAnd(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue number2 = stack.pop().toBoolean();
	ActionValue number1 = stack.pop().toBoolean();
	stack.push(ActionValue(bool(number1.getBoolean() && number2.getBoolean())));
}

void opx_logicalOr(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue number2 = stack.pop().toBoolean();
	ActionValue number1 = stack.pop().toBoolean();
	stack.push(ActionValue(bool(number1.getBoolean() || number2.getBoolean())));
}

void opx_logicalNot(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value = stack.pop().toBoolean();
	stack.push(ActionValue(bool(!value.getBoolean())));
}

void opx_stringEqual(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue str2 = stack.pop();
	ActionValue str1 = stack.pop();
	stack.push(ActionValue(bool(str1.getString().compare(str2.getString()) == 0)));
}

void opx_stringLength(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue str = stack.pop();
	stack.push(ActionValue(avm_number_t(str.getString().length())));
}

void opx_stringExtract(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue count = stack.pop();
	ActionValue index = stack.pop();
	ActionValue strng = stack.pop();
	if (count.isNumeric() && index.isNumeric())
	{
		std::string str = strng.getString();
		int32_t offset = int32_t(index.getNumber());
		if (offset >= 0 && offset < int32_t(str.length()))
		{
			std::string res = str.substr(offset, int(count.getNumber()));
			stack.push(ActionValue(res));
		}
		else
			stack.push(ActionValue(""));
	}
	else
		stack.push(ActionValue(""));
}

void opx_pop(ExecutionState& state)
{
	state.frame->getStack().pop();
}

void opx_int(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number = stack.top();
	if (number.isNumeric())
		number = ActionValue(avm_number_t(std::floor(number.getNumber())));
	else if (number.isString())
		number = ActionValue(avm_number_t(parseString< int32_t >(number.getString())));
	else
		number = ActionValue();
}

void opx_getVariable(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue variable = stack.pop();

	ActionValue variableValue = getVariable(state, variable);

	T_IF_TRACE(
		*state.trace << L"AopGetVariable: \"" << variable.getWideString() << L"\" => \"" << variableValue.getWideString() << L"\"" << Endl;
	)

	stack.push(variableValue);
}

void opx_setVariable(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value = stack.pop();
	ActionValue variable = stack.pop();
	std::string variableName = variable.getString();

	uint32_t variableId = state.context->getString(variableName);

	T_IF_TRACE(
		*state.trace << L"AopSetVariable: \"" << variable.getWideString() << L"\"" << Endl;
	)

	ActionValue* variableValue = state.frame->getVariableValue(variableId);
	if (variableValue)
	{
		*variableValue = value;

		T_IF_TRACE(
			*state.trace << L"<set in frame variable>" << Endl;
		)
		return;
	}

	if (state.movieClip)
	{
		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		setMemberOrProperty(state, movieClipAS, variableId, value);

		T_IF_TRACE(
			*state.trace << L"<set in movie clip variable>" << Endl;
		)
		return;
	}

	T_IF_TRACE(
		*state.trace << L"<variable lost>" << Endl;
	)
}

void opx_setTargetExpr(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopSetTargetExpr: Not implemented"  << Endl;
	)
}

void opx_stringConcat(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue str2 = stack.pop();
	ActionValue str1 = stack.pop();
	stack.push(ActionValue(str1.getString() + str2.getString()));
}

void opx_getProperty(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue index = stack.pop();
	ActionValue target = stack.pop();

	FlashSpriteInstance* movieClip = state.movieClip;
	if (target.isObject())
	{
		movieClip = target.getObject()->getRelay< FlashSpriteInstance >();
		if (!movieClip)
		{
			stack.push(ActionValue());
			return;
		}
	}

	switch (int(index.getNumber()))
	{
	// _x
	case 0:
		stack.push(ActionValue(movieClip->getTransform().e31 / 20.0f));
		break;

	// _y
	case 1:
		stack.push(ActionValue(movieClip->getTransform().e32 / 20.0f));
		break;

	// _xscale
	case 2:
		stack.push(ActionValue(movieClip->getTransform().e11 * 100.0f));
		break;

	// _yscale
	case 3:
		stack.push(ActionValue(movieClip->getTransform().e22 * 100.0f));
		break;

	// _currentframe
	case 4:
		stack.push(ActionValue(avm_number_t(movieClip->getCurrentFrame())));
		break;

	// _totalframes
	case 5:
		stack.push(ActionValue(avm_number_t(movieClip->getSprite()->getFrameCount())));
		break;

	// _alpha
	case 6:
		{
			const SwfCxTransform& colorTransform = movieClip->getColorTransform();
			stack.push(ActionValue(colorTransform.alpha[0] * 100.0f));
		}
		break;

	// _visible
	case 7:
		stack.push(ActionValue(movieClip->isVisible()));
		break;

	// _width
	case 8:
		{
			Aabb2 bounds = movieClip->getBounds();
			stack.push(ActionValue((bounds.mx.x - bounds.mn.x) / 20.0f));
		}
		break;

	// _height
	case 9:
		{
			Aabb2 bounds = movieClip->getBounds();
			stack.push(ActionValue((bounds.mx.y - bounds.mn.y) / 20.0f));
		}
		break;

	// _rotation
	//case 10:
	//	break;

	// _target
	case 11:
		{
			stack.push(ActionValue(movieClip->getTarget()));
		}
		break;

	// _framesloaded
	case 12:
		{
			stack.push(ActionValue(avm_number_t(movieClip->getSprite()->getFrameCount())));
		}
		break;

	// _name
	case 13:
		{
			stack.push(ActionValue(movieClip->getName()));
		}
		break;

	// _droptarget
	//case 14:
	//	break;

	// _url
	//case 15:
	//	break;

	// _highquality
	//case 16:
	//	break;

	// _focusrect
	//case 17:
	//	break;

	// _soundbuftime
	//case 18:
	//	break;

	// _quality
	//case 19:
	//	break;

	// _xmouse
	case 20:
		{
			stack.push(ActionValue(avm_number_t(movieClip->getMouseX())));
		}
		break;

	// _ymouse
	case 21:
		{
			stack.push(ActionValue(avm_number_t(movieClip->getMouseY())));
		}
		break;

	default:
		{
			T_IF_TRACE(
				*state.trace << L"AopGetProperty: Unknown index " << int32_t(index.getNumber()) << Endl;
			)
			stack.push(ActionValue());
		}
		break;
	}
}

void opx_setProperty(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value = stack.pop();
	ActionValue index = stack.pop();
	ActionValue target = stack.pop();

	FlashSpriteInstance* movieClip = state.movieClip;
	if (target.isObject())
	{
		movieClip = target.getObject()->getRelay< FlashSpriteInstance >();
		if (!movieClip)
		{
			stack.push(ActionValue());
			return;
		}
	}

	switch (int(index.getNumber()))
	{
	// _x
	case 0:
		{
			Matrix33 transform = movieClip->getTransform();
			transform.e13 = float(value.getNumber() * 20.0f);
			movieClip->setTransform(transform);
		}
		break;

	// _y
	case 1:
		{
			Matrix33 transform = movieClip->getTransform();
			transform.e23 = float(value.getNumber() * 20.0f);
			movieClip->setTransform(transform);
		}
		break;

	// _xscale
	case 2:
		{
			Matrix33 transform = movieClip->getTransform();
			transform.e11 = float(value.getNumber() * 100.0f);
			movieClip->setTransform(transform);
		}
		break;

	// _yscale
	case 3:
		{
			Matrix33 transform = movieClip->getTransform();
			transform.e22 = float(value.getNumber() * 100.0f);
			movieClip->setTransform(transform);
		}
		break;

	// _alpha
	case 6:
		{
			SwfCxTransform colorTransform = movieClip->getColorTransform();
			colorTransform.alpha[0] = value.getNumber() / 100.0f;
			movieClip->setColorTransform(colorTransform);
		}
		break;

	// _visible
	case 7:
		{
			movieClip->setVisible(value.getBoolean());
		}
		break;

	// _name
	case 13:
		{
			movieClip->setName(value.getString());
		}
		break;

	default:
		{
			T_IF_TRACE(
				*state.trace << L"AopSetProperty: Unknown index " << int32_t(index.getNumber()) << Endl;
			)
		}
		break;
	}
}

void opx_cloneSprite(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue depth = stack.pop();
	ActionValue target = stack.pop();
	ActionValue source = stack.pop();

	Ref< FlashSpriteInstance > sourceClip = source.getObjectAlways(state.context)->getRelay< FlashSpriteInstance >();
	Ref< FlashSpriteInstance > cloneClip = sourceClip->clone();

	cloneClip->setName(target.getString());

	state.movieClip->getDisplayList().showObject(
		int32_t(depth.getNumber()),
		cloneClip->getSprite()->getId(),
		cloneClip,
		true
	);
}

void opx_removeSprite(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopRemoveSprite: Not implemented"  << Endl;
	)
}

void opx_trace(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	std::wstring trace = stack.pop().getWideString();
	if (trace != L"** BREAK **")
		log::info << L"TRACE \"" << trace << L"\"" << Endl;
	else
		T_BREAKPOINT;
}

void opx_startDragMovie(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue target = stack.pop();
	uint32_t lockCenter = uint32_t(stack.pop().getNumber());
	uint32_t constraint = uint32_t(stack.pop().getNumber());
	if (constraint)
	{
		ActionValue y2 = stack.pop();
		ActionValue x2 = stack.pop();
		ActionValue y1 = stack.pop();
		ActionValue x1 = stack.pop();
	}
	// @fixme
}

void opx_stopDragMovie(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopStopDragMovie: Not implemented"  << Endl;
	)
}

void opx_stringCompare(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue str2 = stack.pop();
	ActionValue str1 = stack.pop();
	stack.push(ActionValue(avm_number_t(str1.getString().compare(str2.getString()))));
}

void opx_throw(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopThrow: Not implemented"  << Endl;
	)
}

void opx_castOp(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue objectValue = stack.pop();
	ActionValue classValue = stack.pop();

	// Check if object is derived from class.
	if (objectValue.isObject() && classValue.isObject())
	{
		Ref< ActionObject > object = objectValue.getObject();
		Ref< ActionObject > classFunction = classValue.getObject();

		if (object && classFunction)
		{
			ActionObject* __proto__ = object->get__proto__();
			T_ASSERT (__proto__);

			ActionValue prototypeValue;
			classFunction->getLocalMember(ActionContext::IdPrototype, prototypeValue);

			if (prototypeValue.isObject())
			{
				ActionObject* prototype = prototypeValue.getObject();
				T_ASSERT (prototype);

				for (;;)
				{
					if (__proto__ == prototype)
					{
						stack.push(objectValue);
						return;
					}

					ActionObject* parent__proto__ = __proto__->get__proto__();
					if (!parent__proto__ || parent__proto__ == __proto__)
						break;

					__proto__ = parent__proto__;
				}
			}
		}
	}

	// Use native class "xplicit" coersion.
	if (classValue.isObject())
	{
		Ref< ActionFunction > classFunction = classValue.getObject< ActionFunction >();
		if (classFunction)
		{
			ActionValueArray argv(state.context->getPool(), 1);
			argv[0] = objectValue;
			stack.push(classFunction->call(argv));
			return;
		}
	}

	stack.push(ActionValue((avm_number_t)0));
}

void opx_implementsOp(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue implementationClass = stack.pop();
	uint32_t interfaceCount = uint32_t(stack.pop().getNumber());

	for (uint32_t i = 0; i < interfaceCount; ++i)
	{
		ActionValue interfaceClass = stack.pop();
		if (interfaceClass.isObject())
			implementationClass.getObject()->addInterface(interfaceClass.getObject());
	}
}

void opx_random(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& max = stack.top();
	if (max.isNumeric())
	{
		int32_t rnd = std::rand() % std::max< int32_t >(int32_t(max.getNumber()), 1);
		max = ActionValue(avm_number_t(rnd));
	}
	else
		max = ActionValue();
}

void opx_mbLength(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopMbLength: Not implemented"  << Endl;
	)
}

void opx_ord(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopOrd: Not implemented"  << Endl;
	)
}

void opx_chr(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopChr: Not implemented"  << Endl;
	)
}

void opx_getTime(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	avm_number_t time = avm_number_t(int32_t(state.timer->getElapsedTime() * 1000.0f));
	stack.push(ActionValue(time));
}

void opx_mbSubString(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopMbSubString: Not implemented"  << Endl;
	)
}

void opx_mbOrd(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopMbOrd: Not implemented"  << Endl;
	)
}

void opx_mbChr(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopMbChr: Not implemented"  << Endl;
	)
}

void opx_delete(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	std::string memberName = stack.pop().getString();
	ActionValue objectValue = stack.pop();
	if (objectValue.isObject())
	{
		uint32_t memberNameId = state.context->getString(memberName);
		bool deleted = objectValue.getObject()->deleteMember(memberNameId);
		stack.push(ActionValue(deleted));
	}
	else
		stack.push(ActionValue(false));
}

void opx_delete2(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	std::string variableName = stack.pop().getString();

	uint32_t variableNameId = state.context->getString(variableName);

	if (state.with)
	{
		if (state.with->deleteMember(variableNameId))
		{
			stack.push(ActionValue(true));
			return;
		}
	}

	if (state.self)
	{
		if (state.self->deleteMember(variableNameId))
		{
			stack.push(ActionValue(true));
			return;
		}
	}

	if (state.movieClip)
	{
		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		if (movieClipAS->deleteMember(variableNameId))
		{
			stack.push(ActionValue(true));
			return;
		}
	}

	stack.push(ActionValue(false));
}

void opx_defineLocal(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue variableValue = stack.pop();
	std::string variableName = stack.pop().getString();

#if defined(_DEBUG)
	ActionFunction* variableFunction = variableValue.getObject< ActionFunction >();
	if (variableFunction && variableFunction->getName().empty())
		variableFunction->setName(variableName);
#endif

	uint32_t variableNameId = state.context->getString(variableName);

	if (state.frame->getCallee())
	{
		state.frame->setVariable(variableNameId, variableValue);
		return;
	}

	if (state.movieClip)
	{
		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		movieClipAS->setMember(variableNameId, variableValue);
		return;
	}
}

void opx_callFunction(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue functionName = stack.pop();

	ActionValue functionObject = getVariable(state, functionName);
	Ref< ActionFunction > fn = functionObject.getObject< ActionFunction >();
	if (fn)
	{
		stack.push(fn->call(state.frame));
		return;
	}

	T_IF_TRACE(
		*state.trace << L"AopCallFunction: Undefined function \"" << functionName.getWideString() << L"\"" << Endl;
	)

	int argCount = int(stack.pop().getNumber());
	stack.drop(argCount);
	stack.push(ActionValue());
}

void opx_modulo(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue number2 = stack.pop();
	ActionValue number1 = stack.pop();
	if (number1.isNumeric() && number2.isNumeric())
	{
		int n2 = int(number2.getNumber());
		int n1 = int(number1.getNumber());
		stack.push(ActionValue(avm_number_t(n1 % n2)));
	}
	else
		stack.push(ActionValue());
}

void opx_new(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue classFunctionValue = stack.pop();

	// Get class function.
	Ref< ActionFunction > classFunction = classFunctionValue.getObject< ActionFunction >();
	if (!classFunction)
	{
		classFunctionValue = getVariable(state, classFunctionValue);
		classFunction = classFunctionValue.getObject< ActionFunction >();
	}

	if (!classFunction)
	{
		int32_t argCount = int32_t(stack.pop().getNumber());
		stack.drop(argCount);
		stack.push(ActionValue());
		return;
	}

	// Get class prototype.
	ActionValue prototypeValue;
	classFunction->getLocalMember(ActionContext::IdPrototype, prototypeValue);

	if (!prototypeValue.isObject())
	{
		int32_t argCount = int32_t(stack.pop().getNumber());
		stack.drop(argCount);
		stack.push(ActionValue());
		return;
	}

	Ref< ActionObject > prototype = prototypeValue.getObject();

	// Create instance.
	Ref< ActionObject > self = new ActionObject(state.context, prototype);
	self->setMember(ActionContext::Id__ctor__, classFunctionValue);

	classFunction->call(state.frame, self);
	stack.push(ActionValue(self));
}

void opx_defineLocal2(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	std::string variableName = stack.pop().getString();

	uint32_t variableNameId = state.context->getString(variableName);

	if (state.frame->getCallee())
	{
		state.frame->setVariable(variableNameId, ActionValue());
		return;
	}

	if (state.movieClip)
	{
		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		movieClipAS->setMember(variableNameId, ActionValue());
	}
}

void opx_initArray(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue arrayClassMember;
	if (
		state.global->getLocalMember("Array", arrayClassMember) &&
		arrayClassMember.isObject< ActionClass >()
	)
	{
		ActionFunction* arrayClass = arrayClassMember.getObject< ActionFunction >();
		T_ASSERT (arrayClass);

		// Allocate array instance.
		Ref< ActionObject > self = new ActionObject(state.context, "Array");
		self->setMember(ActionContext::Id__ctor__, arrayClassMember);

		// Initialize array through constructor.
		arrayClass->call(state.frame, self);

		// Push array instance.
		stack.push(ActionValue(self));
	}
	else
		stack.push(ActionValue());
}

void opx_initObject(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	int32_t initialPropertyCount = int32_t(stack.pop().getNumber());

	Ref< ActionObject > scriptObject = new ActionObject(state.context);
	for (int32_t i = 0; i < initialPropertyCount; ++i)
	{
		ActionValue value = stack.pop();
		ActionValue name = stack.pop();
		setMemberOrProperty(state, scriptObject, name.getStringId(), value);
		T_IF_TRACE(
			*state.trace << L"AopInitObject: " << i << L" \"" << name.getWideString() << L"\" = " << value.getWideString() << Endl;
		)
	}

	stack.push(ActionValue(scriptObject));
}

void opx_typeOf(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	ActionValue value = stack.pop();
	if (value.isNumeric())
	{
		stack.push(ActionValue("number"));
	}
	else if (value.isBoolean())
	{
		stack.push(ActionValue("boolean"));
	}
	else if (value.isString())
	{
		stack.push(ActionValue("string"));
	}
	else if (value.isObject())
	{
		Ref< ActionObject > object = value.getObject();
		if (!object)
			stack.push(ActionValue("null"));
		else if (is_a< FlashSpriteInstance >(object->getRelay()))
			stack.push(ActionValue("movieclip"));
		else if (is_a< ActionFunction >(object))
			stack.push(ActionValue("function"));
		else
			stack.push(ActionValue("object"));
	}
	else
		stack.push(ActionValue("undefined"));
}

void opx_targetPath(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopTargetPath: Not implemented"  << Endl;
	)
}

void opx_enumerate(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopEnumerate: Not implemented"  << Endl;
	)
}

void opx_newAdd(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value2 = stack.pop();
	ActionValue value1 = stack.pop();
	stack.push(value1 + value2);
}

void opx_newLessThan(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value2 = stack.pop();
	ActionValue value1 = stack.pop();
	if (value2.isString() || value1.isString())
	{
		ActionValue string2 = value2.toString();
		ActionValue string1 = value1.toString();
		if (string2.isString() && string1.isString())
		{
			std::string str2 = string2.getString();
			std::string str1 = string1.getString();
			stack.push(ActionValue(bool(str1.compare(str2) < 0)));
		}
		else
			stack.push(ActionValue());
	}
	else
	{
		ActionValue number2 = value2.toNumber();
		ActionValue number1 = value1.toNumber();
		if (number2.isNumeric() && number1.isNumeric())
		{
			avm_number_t n2 = number2.getNumber();
			avm_number_t n1 = number1.getNumber();
			stack.push(ActionValue(bool(n1 < n2)));
		}
		else
			stack.push(ActionValue());
	}
}

void opx_newEquals(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value2 = stack.pop();
	ActionValue value1 = stack.pop();
	stack.push(ActionValue(value1 == value2));
}

void opx_toNumber(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	stack.top() = stack.top().toNumber();
}

void opx_toString(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	stack.top() = stack.top().toString();
}

void opx_dup(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	stack.push(stack.top());
}

void opx_swap(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue v1 = stack.pop();
	ActionValue v2 = stack.pop();
	stack.push(v1);
	stack.push(v2);
}

void opx_getMember(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue memberNameValue = stack.pop();
	ActionValue targetValue = stack.pop();
	ActionValue memberValue;

	Ref< ActionObject > target = targetValue.getObjectAlways(state.context);

	// Special case for arrays; we cannot use string table for all entries thus need to explicitly access elements.
	const ActionObjectRelay* relay = target->getRelay();
	if (
		relay != 0 &&
		&type_of(relay) == &type_of< Array >()
	)
	{
		const Array* arr = checked_type_cast< const Array* >(relay);
		int32_t index;

		if (memberNameValue.isNumeric())
			index = int32_t(memberNameValue.getNumber());
		else
			index = parseIndex(memberNameValue.getString().c_str());

		if (index >= 0 && index < int32_t(arr->length()))
		{
			stack.push((*arr)[index]);
			return;
		}
	}

	std::string memberName;
	int32_t memberId = -1;

	if (memberNameValue.isString())
	{
		memberName = memberNameValue.getString();
		memberId = memberNameValue.getStringId();
	}
	if (memberId < 0)
	{
		memberName = memberNameValue.getString();
		memberId = state.context->getString(memberName);
	}

	if (target->getMember(memberId, memberValue))
	{
		T_IF_TRACE(
			*state.trace << L"AopGetMember: \"" << mbstows(memberName) << L"\" => \"" << memberValue.getWideString() << L"\"" << Endl;
		)
		stack.push(memberValue);
		return;
	}

	Ref< ActionFunction > propertyGet;
	if (target->getPropertyGet(memberId, propertyGet))
	{
		stack.push(ActionValue(avm_number_t(0)));
		memberValue = propertyGet->call(state.frame, target);

		T_IF_TRACE(
			*state.trace << L"AopGetMember: \"" << mbstows(memberName) << L"\" => \"" << memberValue.getWideString() << L"\"" << Endl;
		)
		stack.push(memberValue);
		return;
	}

	T_IF_TRACE(
		*state.trace << L"AopGetMember: \"" << mbstows(memberName) << L"\", no such member" << Endl;
	)

	stack.push(ActionValue());
}

void opx_setMember(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue memberValue = stack.pop();
	ActionValue memberNameValue = stack.pop();
	ActionValue targetValue = stack.pop();

	T_IF_TRACE(
		*state.trace << L"Target: \"" << targetValue.getWideString() << L"\"" << Endl;
		*state.trace << L"Member: \"" << memberNameValue.getWideString() << L"\"" << Endl;
		*state.trace << L"Value: \"" << memberValue.getWideString() << L"\"" << Endl;
	)

	Ref< ActionObject > target = targetValue.getObjectAlways(state.context);

	// Special case for arrays; we cannot use string table for all entries thus need to explicitly access elements.
	ActionObjectRelay* relay = target->getRelay();
	if (relay != 0 && &type_of(relay) == &type_of< Array >())
	{
		Array* arr = checked_type_cast< Array* >(relay);
		int32_t index;

		if (memberNameValue.isNumeric())
			index = int32_t(memberNameValue.getNumber());
		else
			index = parseIndex(memberNameValue.getString().c_str());

		if (index >= 0 && index < int32_t(arr->length()))
		{
			(*arr)[index] = memberValue;
			return;
		}
	}

#if defined(_DEBUG)
	std::string memberName;
#endif
	int32_t memberId = -1;

	if (memberNameValue.isString())
	{
#if defined(_DEBUG)
		memberName = memberNameValue.getString();
#endif
		memberId = memberNameValue.getStringId();
	}
	if (memberId < 0)
	{
#if !defined(_DEBUG)
		std::string // ...
#endif
		memberName = memberNameValue.getString();
		memberId = state.context->getString(memberName);
	}

	Ref< ActionFunction > propertySet;
	if (target->getPropertySet(memberId, propertySet))
	{
		stack.push(memberValue);
		stack.push(ActionValue(avm_number_t(1)));
		propertySet->call(state.frame, target);
	}
	else
		target->setMember(memberId, memberValue);

#if defined(_DEBUG)
	ActionFunction* memberFunction = memberValue.getObject< ActionFunction >();
	if (memberFunction && memberFunction->getName().empty())
		memberFunction->setName(memberName);
#endif
}

void opx_increment(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number = stack.top();
	if (number.isNumeric())
		number = ActionValue(number.getNumber() + 1);
	else
		number = ActionValue();
}

void opx_decrement(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number = stack.top();
	if (number.isNumeric())
		number = ActionValue(number.getNumber() - 1);
	else
		number = ActionValue();
}

void opx_callMethod(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue methodName = stack.pop();
	ActionValue targetValue = stack.pop();
	ActionValue returnValue;

	Ref< ActionObject > target = targetValue.getObjectAlways(state.context);
	Ref< ActionFunction > method;

	if (!target && state.movieClip)
		target = state.movieClip->getAsObject(state.context);

	if (methodName.isString())
	{
		int32_t memberId = methodName.getStringId();
		if (memberId < 0)
			memberId = state.context->getString(methodName.getString());

		ActionValue memberValue;
		target->getMember(memberId, memberValue);
		method = memberValue.getObject< ActionFunction >();
	}
	else
	{
		method = dynamic_type_cast< ActionFunction* >(target);
	}

	if (method)
	{
		T_IF_TRACE(
			*state.trace << L"AopCallMethod: \"" << mbstows(method->getName()) << L"\"" << Endl;
		)

		if (is_a< ActionSuper >(target))
		{
			// We're calling through super object.
			ActionSuper* superTarget = checked_type_cast< ActionSuper* >(target);
			returnValue = method->call(state.frame, superTarget->getObject(), superTarget->getSuper());
		}
		else
		{
			// Calling ordinary object.
			returnValue = method->call(state.frame, target);
		}
	}
	else
	{
		T_IF_TRACE(
			*state.trace << L"AopCallMethod: Undefined method \"" << methodName.getWideString() << L"\"" << Endl;
		)
		int argCount = int(stack.pop().getNumber());
		stack.drop(argCount);
	}

	stack.push(returnValue);
}

void opx_newMethod(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	std::string classFunctionName = stack.pop().getString();
	Ref< ActionObject > classFunctionObject = stack.pop().getObjectAlways(state.context);

	Ref< ActionFunction > classFunction;
	if (classFunctionName.empty())
		classFunction = dynamic_type_cast< ActionFunction* >(classFunctionObject);
	else
	{
		ActionValue methodValue;
		classFunctionObject->getMember(classFunctionName, methodValue);
		classFunction = methodValue.getObject< ActionFunction >();
	}

	if (classFunction)
	{
		// Get class prototype.
		ActionValue prototypeValue;
		classFunction->getLocalMember(ActionContext::IdPrototype, prototypeValue);

		if (!prototypeValue.isObject())
		{
			stack.push(ActionValue());
			return;
		}

		Ref< ActionObject > prototype = prototypeValue.getObject();
		Ref< ActionObject > self = new ActionObject(state.context, prototype);
		self->setMember(ActionContext::Id__ctor__, ActionValue(classFunction));
		classFunction->call(state.frame, self);
		stack.push(ActionValue(self));
	}
	else
	{
		T_IF_TRACE(
			*state.trace << L"AopNewMethod: Not a class object" << Endl;
		)
		stack.push(ActionValue());
	}
}

void opx_instanceOf(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue constructorValue = stack.pop();
	ActionValue objectValue = stack.pop();

	if (objectValue.isObject() && constructorValue.isObject())
	{
		Ref< ActionObject > object = objectValue.getObject();
		Ref< ActionObject > constructor = constructorValue.getObject();

		ActionValue constructorProtoValue;
		constructor->getMember(ActionContext::IdPrototype, constructorProtoValue);

		Ref< ActionObject > objectProto = object->get__proto__();
		while (objectProto)
		{
			if (objectProto == constructorProtoValue.getObject())
			{
				stack.push(ActionValue(true));
				return;
			}

			if (objectProto->get__proto__() == objectProto)
				break;

			objectProto = objectProto->get__proto__();
		}
	}

	stack.push(ActionValue(false));
}

void opx_enum2(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue enumObject = stack.pop();

	// Push null object as terminator; only value which cannot be created from AS.
	stack.push(ActionValue((ActionObject*)0));

	if (enumObject.isObject())
	{
		Ref< ActionObject > object = enumObject.getObject();

		const ActionObject::member_map_t& members = object->getLocalMembers();
		for (ActionObject::member_map_t::const_iterator i = members.begin(); i != members.end(); ++i)
		{
			// \fixme Should only enumerate user added members.
			if (
				i->first != ActionContext::Id__proto__ &&
				i->first != ActionContext::IdPrototype &&
				i->first != ActionContext::Id__ctor__
			)
				stack.push(ActionValue(
					state.context->getString(i->first)
				));
		}

		const ActionObjectRelay* relay = object->getRelay();
		if (relay)
		{
			std::vector< uint32_t > memberNames;
			if (relay->enumerateMembers(memberNames))
			{
				for (std::vector< uint32_t >::const_iterator i = memberNames.begin(); i != memberNames.end(); ++i)
					stack.push(ActionValue(
						state.context->getString(*i)
					));
			}

			const Array* arr = dynamic_type_cast< const Array* >(relay);
			if (arr)
			{
				for (uint32_t i = 0; i < arr->length(); ++i)
					stack.push(ActionValue(avm_number_t(i)));
			}
		}

		//const ActionObject::property_map_t& properties = object->getProperties();
		//for (ActionObject::property_map_t::const_iterator i = properties.begin(); i != properties.end(); ++i)
		//	stack.push(ActionValue(i->first));
	}
}

void opx_bitwiseAnd(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue number2 = stack.pop();
	ActionValue number1 = stack.pop();
	if (number1.isNumeric() && number2.isNumeric())
	{
		int32_t n2 = int32_t(number2.getNumber());
		int32_t n1 = int32_t(number1.getNumber());
		stack.push(ActionValue(avm_number_t(n1 & n2)));
	}
	else
		stack.push(ActionValue());
}

void opx_bitwiseOr(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number2 = stack.top(-1);
	ActionValue& number1 = stack.top(0);
	if (number1.isNumeric() && number2.isNumeric())
	{
		int32_t n2 = int32_t(number2.getNumber());
		int32_t n1 = int32_t(number1.getNumber());
		number2 = ActionValue(avm_number_t(n1 | n2));
	}
	else
		number2 = ActionValue();
	stack.drop(1);
}

void opx_bitwiseXor(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue& number2 = stack.top(-1);
	ActionValue& number1 = stack.top(0);
	if (number1.isNumeric() && number2.isNumeric())
	{
		int32_t n2 = int32_t(number2.getNumber());
		int32_t n1 = int32_t(number1.getNumber());
		number2 = ActionValue(avm_number_t(n1 ^ n2));
	}
	else
		number2 = ActionValue();
	stack.drop(1);
}

void opx_shiftLeft(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopShiftLeft: Not implemented"  << Endl;
	)
}

void opx_shiftRight(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopShiftRight: Not implemented"  << Endl;
	)
}

void opx_shiftRight2(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopShiftRight2: Not implemented"  << Endl;
	)
}

void opx_strictEq(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value2 = stack.pop();
	ActionValue value1 = stack.pop();
	if (value1.getType() == value2.getType())
	{
		if (value1.isBoolean())
		{
			bool b2 = value2.getBoolean();
			bool b1 = value1.getBoolean();
			stack.push(ActionValue(b1 == b2));
		}
		else if (value1.isNumeric())
		{
			avm_number_t n2 = value2.getNumber();
			avm_number_t n1 = value1.getNumber();
			stack.push(ActionValue(n1 == n2));
		}
		else if (value1.isString())
		{
			std::string s2 = value2.getString();
			std::string s1 = value1.getString();
			stack.push(ActionValue(s1 == s2));
		}
		else if (value1.isObject())
		{
			ActionObject* o2 = value2.getObject();
			ActionObject* o1 = value1.getObject();
			stack.push(ActionValue(o1 == o2));
		}
		else
			stack.push(ActionValue(true));
	}
	else
		stack.push(ActionValue(false));
}

void opx_greater(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value2 = stack.pop();
	ActionValue value1 = stack.pop();
	if (value2.isString() || value1.isString())
	{
		ActionValue string2 = value2.toString();
		ActionValue string1 = value1.toString();
		if (string2.isString() && string1.isString())
		{
			std::string str2 = string2.getString();
			std::string str1 = string1.getString();
			stack.push(ActionValue(bool(str1.compare(str2) > 0)));
		}
		else
			stack.push(ActionValue());
	}
	else
	{
		ActionValue number2 = value2.toNumber();
		ActionValue number1 = value1.toNumber();
		if (number2.isNumeric() && number1.isNumeric())
		{
			avm_number_t n2 = number2.getNumber();
			avm_number_t n1 = number1.getNumber();
			stack.push(ActionValue(bool(n1 > n2)));
		}
		else
			stack.push(ActionValue());
	}
}

void opx_stringGreater(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue str2 = stack.pop();
	ActionValue str1 = stack.pop();
	stack.push(ActionValue(bool(str1.getString().compare(str2.getString()) > 0)));
}

void opx_extends(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue superClass = stack.pop();
	ActionValue subClass = stack.pop();

	T_ASSERT (superClass.isObject());
	T_ASSERT (subClass.isObject());

	ActionValue superPrototype;
	superClass.getObject()->getMember(ActionContext::IdPrototype, superPrototype);

	Ref< ActionObject > prototype = new ActionObject(state.context);
	prototype->setMember(ActionContext::Id__proto__, superPrototype);
	prototype->setMember(ActionContext::Id__ctor__, superClass);

	subClass.getObject()->setMember(ActionContext::IdPrototype, ActionValue(prototype));
}

void opp_gotoFrame(PreparationState& state)
{
#if defined(T_BIG_ENDIAN)
	swap8in32(*reinterpret_cast< uint16_t* >(state.data));
#endif
}

void opx_gotoFrame(ExecutionState& state)
{
	FlashSpriteInstance* movieClip = state.movieClip;

	uint16_t frame = *reinterpret_cast< const uint16_t* >(state.data);
	movieClip->gotoFrame(frame);
}

void opx_getUrl(ExecutionState& state)
{
	ActionContext* context = state.context;
	ActionValueStack& stack = state.frame->getStack();

	const char* url = reinterpret_cast< const char* >(state.data);
	const char* query = url + strlen(url) + 1;

	ActionValue getUrl;
	context->getGlobal()->getLocalMember("getURL", getUrl);
	if (getUrl.isObject< ActionFunction >())
	{
		Ref< ActionFunction > fn = getUrl.getObject< ActionFunction >();
		if (fn)
		{
			stack.push(ActionValue(query));
			stack.push(ActionValue(url));
			stack.push(ActionValue(avm_number_t(2)));
			fn->call(state.frame);
		}
		else
		{
			T_IF_TRACE(
				*state.trace << L"AopGetUrl: _global.getURL must be a function object" << Endl;
			)
		}
	}
}

void opx_setRegister(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	ActionValue value = !stack.empty() ? stack.top() : ActionValue();

	uint8_t registerIndex = *state.data;
	state.frame->setRegister(registerIndex, value);

	T_IF_TRACE(
		*state.trace << L"AopSetRegister: " << int32_t(registerIndex) << L" = " << value.getWideString() << Endl;
	)
}

void opp_constantPool(PreparationState& state)
{
#if defined(T_BIG_ENDIAN)
	swap8in32(*reinterpret_cast< uint16_t* >(state.data));
#endif
}

void opx_constantPool(ExecutionState& state)
{
	Ref< ActionDictionary > dictionary = new ActionDictionary();

	uint16_t dictionaryCount = *reinterpret_cast< const uint16_t* >(state.data);
	const char* dictionaryEntry = reinterpret_cast< const char* >(state.data + 2);

	for (uint16_t i = 0; i < dictionaryCount; ++i)
	{
		T_IF_TRACE(
			*state.trace << int32_t(i) << L" = " << mbstows(dictionaryEntry) << Endl;
		)

		int32_t index = state.context->getString(dictionaryEntry);
		dictionary->add(ActionValue(
			dictionaryEntry,
			index
		));

		dictionaryEntry += std::strlen(dictionaryEntry) + 1;
	}

	state.frame->setDictionary(dictionary);
}

void opx_waitForFrame(ExecutionState& state)
{
}

void opx_setTarget(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopSetTarget: Not implemented"  << Endl;
	)
}

void opx_gotoLabel(ExecutionState& state)
{
	FlashSpriteInstance* movieClip = state.movieClip;

	const char* label = reinterpret_cast< const char* >(state.data);
	int frame = movieClip->getSprite()->findFrame(label);
	if (frame >= 0)
	{
		movieClip->setPlaying(false);
		movieClip->gotoFrame(frame);
	}
}

void opx_waitForFrameExpression(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopWaitForFrameExpression: Not implemented"  << Endl;
	)
}

void opp_defineFunction2(PreparationState& state)
{
	uint8_t* data = state.data;

	const char* functionName = reinterpret_cast< const char* >(data);
	data += strlen(functionName) + 1;

#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint16_t*)data);
#endif
	uint16_t argumentCount = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	data += sizeof(uint8_t);

#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint16_t*)data);
#endif
	data += sizeof(uint16_t);

	for (int i = 0; i  < argumentCount; ++i)
	{
		data += sizeof(uint8_t);

		const char* variableName = reinterpret_cast< const char* >(data);
		data += strlen(variableName) + 1;
	}

#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint16_t*)data);
#endif
	uint16_t codeSize = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	state.npc += codeSize;
}

void opx_defineFunction2(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	const uint8_t* data = state.data;

	const char T_UNALIGNED * functionName = reinterpret_cast< const char* >(data);
	data += strlen(functionName) + 1;

	uint16_t argumentCount = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	uint8_t registerCount = *data;
	data += sizeof(uint8_t);

	uint16_t flags = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	std::vector< std::pair< std::string, uint8_t > > argumentsIntoRegisters(argumentCount);
	for (int i = 0; i  < argumentCount; ++i)
	{
		uint8_t registerIndex = *data;
		data += sizeof(uint8_t);

		const char* variableName = reinterpret_cast< const char* >(data);
		data += strlen(variableName) + 1;

		argumentsIntoRegisters[i] = std::make_pair(variableName, registerIndex);
	}

	uint16_t codeSize = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	Ref< ActionVMImage1 > image = new ActionVMImage1(state.npc, codeSize);

	Ref< ActionFunction > function = new ActionFunction2(
		state.context,
		functionName,
		image,
		registerCount,
		flags,
		argumentsIntoRegisters,
		state.frame->getVariables(),
		state.frame->getDictionary()
	);

	// Create prototype.
	Ref< ActionObject > prototype = new ActionObject(state.context);
	prototype->setMember(ActionContext::IdConstructor, ActionValue(function));
	function->setMember(ActionContext::IdPrototype, ActionValue(prototype));

	if (*functionName != 0)
	{
		// Assign function object into variable with the same name as the function.
		T_ASSERT (state.movieClip);

		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		movieClipAS->setMember(functionName, ActionValue(function));
	}
	else
	{
		// Push function object onto stack.
		stack.push(ActionValue(function));
	}

	T_IF_TRACE(
		*state.trace << L"AopDefineFunction2: \"" << mbstows(functionName) << L"\"" << Endl;
	)

	state.npc += codeSize;
}

void opx_try(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopTry: Not implemented"  << Endl;
	)
}

void opx_with(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	state.with = stack.pop().getObject();
}

void opp_pushData(PreparationState& state)
{
	// Try to convert values and replace with custom type.
	{
		AutoArrayPtr< uint8_t > nd(new uint8_t [65536]);
		uint8_t* ndp = nd.ptr();

		uint8_t* data = state.data;
		uint8_t* end = data + state.length;
		while (data < end)
		{
			uint8_t type = *data++;

			if (type == 0)		// String
			{
				const char T_UNALIGNED * str = (const char T_UNALIGNED *)data;
				uint32_t length = uint32_t(strlen(str));
				uint16_t index = state.image->addConstData(ActionValue(str));

				*ndp++ = 100;
				*(uint16_t*)ndp = index;
				ndp += sizeof(uint16_t);

				data += length + 1;
			}
			else if (type == 1)	// Number
			{
				float value = unalignedRead< float >(data);
#if defined(T_BIG_ENDIAN)
				swap8in32(value);
#endif
				uint16_t index = state.image->addConstData(ActionValue(value));

				*ndp++ = 100;
				*(uint16_t*)ndp = index;
				ndp += sizeof(uint16_t);

				data += sizeof(float);
			}
			else if (type == 2)	// Null
			{
				*ndp++ = 2;
			}
			else if (type == 3)	// Undefined
			{
				*ndp++ = 3;
			}
			else if (type == 4)	// Register
			{
				*ndp++ = 4;
				*ndp++ = *data++;
			}
			else if (type == 5)	// Boolean
			{
				*ndp++ = 5;
				*ndp++ = *data++;
			}
			else if (type == 6)	// Double
			{
				union { double d; uint8_t b[8]; uint32_t dw[2]; } w;

#if defined(T_LITTLE_ENDIAN)
				w.dw[0] = *(const uint32_t T_UNALIGNED *)&data[4];
				w.dw[1] = *(const uint32_t T_UNALIGNED *)&data[0];
#elif defined(T_BIG_ENDIAN)
				w.b[0] = data[3];
				w.b[1] = data[2];
				w.b[2] = data[1];
				w.b[3] = data[0];
				w.b[4] = data[7];
				w.b[5] = data[6];
				w.b[6] = data[5];
				w.b[7] = data[4];
#endif

				uint16_t index = state.image->addConstData(ActionValue(avm_number_t(w.d)));

				*ndp++ = 100;
				*(uint16_t*)ndp = index;
				ndp += sizeof(uint16_t);

				data += sizeof(double);
			}
			else if (type == 7)	// Integer (32bit)
			{
				int32_t value = unalignedRead< int32_t >(data);
#if defined(T_BIG_ENDIAN)
				swap8in32(value);
#endif
				uint16_t index = state.image->addConstData(ActionValue(avm_number_t(value)));

				*ndp++ = 100;
				*(uint16_t*)ndp = index;
				ndp += sizeof(uint16_t);

				data += sizeof(int32_t);
			}
			else if (type == 8)	// Dictionary (8bit index)
			{
				*ndp++ = 8;
				*ndp++ = *data++;
			}
			else if (type == 9)	// Dictionary (16bit index)
			{
				uint16_t index = unalignedRead< uint16_t >(data);
#if defined(T_BIG_ENDIAN)
				swap8in32(index);
#endif

				*ndp++ = 9;
				*(uint16_t*)ndp = index;
				ndp += sizeof(uint16_t);

				data += sizeof(uint16_t);
			}
			else
				break;
		}

		if (uint32_t(ndp - nd.ptr()) <= state.length)
		{
			while (uint32_t(ndp - nd.ptr()) < state.length)
				*ndp++ = 200;

			std::memcpy(state.data, nd.ptr(), state.length);
		}
	}
}

void opx_pushData(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	const uint8_t T_UNALIGNED * data = state.data;
	const uint8_t T_UNALIGNED * end = data + state.length;
	int32_t index = 0;

	while (data < end)
	{
		ActionValue value;
		uint8_t type = *data++;

		T_IF_TRACE(
			if (type != 200)
				*state.trace << ++index << L". " << int32_t(type);
		)

		if (type == 0)		// String
		{
			uint32_t length = uint32_t(strlen((const char*)data));
			value = ActionValue((const char*)data);
			data += length + 1;
		}
		else if (type == 1)	// Number
		{
			value = ActionValue(avm_number_t(*(const float*)data));
			data += sizeof(float);
		}
		else if (type == 2)	// Null
		{
			value = ActionValue((ActionObject*)0);
		}
		else if (type == 3)	// Undefined
		{
			// Do nothing, value is already undefined.
		}
		else if (type == 4)	// Register
		{
			uint8_t index2 = *data++;
			value = state.frame->getRegister(index2);
		}
		else if (type == 5)	// Boolean
		{
			value = ActionValue(bool(*data++ ? true : false));
		}
		else if (type == 6)	// Double
		{
			union { double d; uint8_t b[8]; uint32_t dw[2]; } w;

#if defined(T_LITTLE_ENDIAN)
			w.dw[0] = *(const uint32_t*)&data[4];
			w.dw[1] = *(const uint32_t*)&data[0];
#elif defined(T_BIG_ENDIAN)
			w.b[0] = data[3];
			w.b[1] = data[2];
			w.b[2] = data[1];
			w.b[3] = data[0];
			w.b[4] = data[7];
			w.b[5] = data[6];
			w.b[6] = data[5];
			w.b[7] = data[4];
#endif

			value = ActionValue(avm_number_t(w.d));
			data += sizeof(double);
		}
		else if (type == 7)	// Integer (32bit)
		{
			int32_t n = unalignedRead< int32_t >(data);
			value = ActionValue(avm_number_t(n));
			data += sizeof(int32_t);
		}
		else if (type == 8)	// Dictionary (8bit index)
		{
			uint8_t index2 = *data++;
			value = state.frame->getDictionary()->get(index2);

			T_IF_TRACE(
				*state.trace << L" (" << int32_t(index2) << L")";
			)
		}
		else if (type == 9)	// Dictionary (16bit index)
		{
			uint16_t index2 = *reinterpret_cast< const uint16_t* >(data);
			value = state.frame->getDictionary()->get(index2);
			data += sizeof(uint16_t);

			T_IF_TRACE(
				*state.trace << L" (" << int32_t(index2) << L")";
			)
		}
		else if (type == 100)	// Preconverted constant value.
		{
			uint16_t index2 = *reinterpret_cast< const uint16_t* >(data);
			value = state.image->getConstData(index2);
			data += sizeof(uint16_t);
		}
		else if (type == 200)	// End
			break;
		else
			break;

		T_IF_TRACE(
			*state.trace << L" = " << value.getWideString() << Endl;
		)

		stack.push(value);
	}
}

void opp_branchAlways(PreparationState& state)
{
#if defined(T_BIG_ENDIAN)
	swap8in32(*reinterpret_cast< int16_t* >(state.data));
#endif
}

void opx_branchAlways(ExecutionState& state)
{
	int16_t offset = *reinterpret_cast< const int16_t* >(state.data);
	state.npc += offset;
}

void opx_getUrl2(ExecutionState& state)
{
	ActionContext* context = state.context;
	ActionValueStack& stack = state.frame->getStack();

	ActionValue getUrl;
	context->getGlobal()->getLocalMember("getURL", getUrl);
	if (getUrl.isObject< ActionFunction >())
	{
		Ref< ActionFunction > fn = getUrl.getObject< ActionFunction >();
		if (fn)
		{
			ActionValue target = stack.pop();
			ActionValue url = stack.pop();

			stack.push(target);
			stack.push(url);
			stack.push(ActionValue(avm_number_t(2)));

			fn->call(state.frame);
		}
		else
		{
			T_IF_TRACE(
				*state.trace << L"AopGetUrl2: _global.getURL must be a function object" << Endl;
			)
		}
	}
}

void opp_defineFunction(PreparationState& state)
{
	uint8_t* data = state.data;

	const char* functionName = reinterpret_cast< const char* >(data);
	data += strlen(functionName) + 1;

#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint16_t*)data);
#endif
	uint16_t argumentCount = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	for (int i = 0; i  < argumentCount; ++i)
	{
		const char* argumentName = reinterpret_cast< const char* >(data);
		data += strlen(argumentName) + 1;
	}

#if defined(T_BIG_ENDIAN)
	swap8in32(*(uint16_t*)data);
#endif
	uint16_t codeSize = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	state.npc += codeSize;
}

void opx_defineFunction(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	const uint8_t* data = state.data;

	const char* functionName = reinterpret_cast< const char* >(data);
	data += strlen(functionName) + 1;

	uint16_t argumentCount = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	std::vector< std::string > argumentsIntoVariables(argumentCount);
	for (int i = 0; i  < argumentCount; ++i)
	{
		const char* argumentName = reinterpret_cast< const char* >(data);
		data += strlen(argumentName) + 1;

		argumentsIntoVariables[i] = argumentName;
	}

	uint16_t codeSize = *reinterpret_cast< const uint16_t* >(data);
	data += sizeof(uint16_t);

	Ref< ActionVMImage1 > image = new ActionVMImage1(state.npc, codeSize);

	Ref< ActionFunction > function = new ActionFunction1(
		state.context,
		functionName,
		image,
		argumentCount,
		argumentsIntoVariables,
		state.frame->getVariables(),
		state.frame->getDictionary()
	);

	// Create prototype.
	Ref< ActionObject > prototype = new ActionObject(state.context);
	prototype->setMember(ActionContext::IdConstructor, ActionValue(function));
	function->setMember(ActionContext::IdPrototype, ActionValue(prototype));

	if (*functionName != 0)
	{
		// Assign function object into variable with the same name as the function.
		T_ASSERT (state.movieClip);

		ActionObject* movieClipAS = state.movieClip->getAsObject(state.context);
		T_ASSERT (movieClipAS);

		movieClipAS->setMember(functionName, ActionValue(function));
	}
	else
	{
		// Push function object onto stack.
		stack.push(ActionValue(function));
	}

	T_IF_TRACE(
		*state.trace << L"AopDefineFunction: \"" << mbstows(functionName) << L"\"" << Endl;
	)

	state.npc += codeSize;
}

void opp_branchIfTrue(PreparationState& state)
{
#if defined(T_BIG_ENDIAN)
	swap8in32(*reinterpret_cast< int16_t* >(state.data));
#endif
}

void opx_branchIfTrue(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();

	bool condition = stack.pop().getBoolean();
	if (condition)
	{
		int16_t offset = *reinterpret_cast< const int16_t* >(state.data);
		state.npc += offset;
	}
}

void opx_callFrame(ExecutionState& state)
{
	T_IF_TRACE(
		*state.trace << L"AopCallFrame: Not implemented"  << Endl;
	)
}

void opx_gotoFrame2(ExecutionState& state)
{
	ActionValueStack& stack = state.frame->getStack();
	FlashSpriteInstance* movieClip = state.movieClip;

	ActionValue frame = stack.pop();
	int frameIndex = -1;

	if (frame.isNumeric())
	{
		frameIndex = int(frame.getNumber());
	}
	else if (frame.isString())
	{
		frameIndex = movieClip->getSprite()->findFrame(frame.getString());
	}
	else
	{
		T_IF_TRACE(
			*state.trace << L"AopGotoFrame2: Invalid frame" << Endl;
		)
		return;
	}

	if (frameIndex < 0)
	{
		T_IF_TRACE(
			*state.trace << L"AopGotoFrame2: No such frame" << Endl;
		)
		return;
	}

	// Frame bias
	if ((state.data[0] & 1) == 1)
	{
		uint16_t frameBias = *reinterpret_cast< const uint16_t* >(&state.data[1]);
		frameIndex += frameBias;
	}

	// Play or stop
	if ((state.data[0] & 2) == 2)
		movieClip->setPlaying(true);
	else
		movieClip->setPlaying(false);

	movieClip->gotoFrame(frameIndex);
}

		}

const OperationInfo c_operationInfos[] =
{
	// 0x00
	{ AopEnd, "AopEnd", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopNextFrame, "AopNextFrame", 0, &opx_nextFrame },
	{ AopPrevFrame, "AopPrevFrame", 0, &opx_prevFrame },
	{ AopPlay, "AopPlay", 0, &opx_play },
	{ AopStop, "AopStop", 0, &opx_stop },
	{ AopToggleQuality, "AopToggleQuality", 0, &opx_toggleQuality },
	{ AopStopSounds, "AopStopSounds", 0, &opx_stopSounds },
	{ AopAdd, "AopAdd", 0, &opx_add },
	{ AopSubtract, "AopSubtract", 0, &opx_subtract },
	{ AopMultiply, "AopMultiply", 0, &opx_multiply },
	{ AopDivide, "AopDivide", 0, &opx_divide },
	{ AopEqual, "AopEqual", 0, &opx_equal },
	{ AopLessThan, "AopLessThan", 0, &opx_lessThan },

	// 0x10
	{ AopLogicalAnd, "AopLogicalAnd", 0, &opx_logicalAnd },
	{ AopLogicalOr, "AopLogicalOr", 0, &opx_logicalOr },
	{ AopLogicalNot, "AopLogicalNot", 0, &opx_logicalNot },
	{ AopStringEqual, "AopStringEqual", 0, &opx_stringEqual },
	{ AopStringLength, "AopStringLength", 0, &opx_stringLength },
	{ AopStringExtract, "AopStringExtract", 0, &opx_stringExtract },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopPop, "AopPop", 0, &opx_pop },
	{ AopInt, "AopInt", 0, &opx_int },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopGetVariable, "AopGetVariable", 0, &opx_getVariable },
	{ AopSetVariable, "AopSetVariable", 0, &opx_setVariable },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },

	// 0x20
	{ AopSetTargetExpression, "AopSetTargetExpression", 0, &opx_setTargetExpr },
	{ AopStringConcat, "AopStringConcat", 0, &opx_stringConcat },
	{ AopGetProperty, "AopGetProperty", 0, &opx_getProperty },
	{ AopSetProperty, "AopSetProperty", 0, &opx_setProperty },
	{ AopCloneSprite, "AopCloneSprite", 0, &opx_cloneSprite },
	{ AopRemoveSprite, "AopRemoveSprite", 0, &opx_removeSprite },
	{ AopTrace, "AopTrace", 0, &opx_trace },
	{ AopStartDragMovie, "AopStartDragMovie", 0, &opx_startDragMovie },
	{ AopStopDragMovie, "AopStopDragMovie", 0, &opx_stopDragMovie },
	{ AopStringCompare, "AopStringCompare", 0, &opx_stringCompare },
	{ AopThrow, "AopThrow", 0, &opx_throw },
	{ AopCastOp, "AopCastOp", 0, &opx_castOp },
	{ AopImplementsOp, "AopImplementsOp", 0, &opx_implementsOp },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },

	// 0x30
	{ AopRandom, "AopRandom", 0, &opx_random },
	{ AopMbLength, "AopMbLength", 0, &opx_mbLength },
	{ AopOrd, "AopOrd", 0, &opx_ord },
	{ AopChr, "AopChr", 0, &opx_chr },
	{ AopGetTime, "AopGetTime", 0, &opx_getTime },
	{ AopMbSubString, "AopMbSubString", 0, &opx_mbSubString },
	{ AopMbOrd, "AopMbOrd", 0, &opx_mbOrd },
	{ AopMbChr, "AopMbChr", 0, &opx_mbChr },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopDelete, "AopDelete", 0, &opx_delete },
	{ AopDelete2, "AopDelete2", 0, &opx_delete2 },
	{ AopDefineLocal, "AopDefineLocal", 0, &opx_defineLocal },
	{ AopCallFunction, "AopCallFunction", 0, &opx_callFunction },
	{ AopReturn, "AopReturn", 0, 0 },
	{ AopModulo, "AopModulo", 0, &opx_modulo },

	// 0x40
	{ AopNew, "AopNew", 0, &opx_new },
	{ AopDefineLocal2, "AopDefineLocal2", 0, &opx_defineLocal2 },
	{ AopInitArray, "AopInitArray", 0, &opx_initArray },
	{ AopInitObject, "AopInitObject", 0, &opx_initObject },
	{ AopTypeOf, "AopTypeOf", 0, &opx_typeOf },
	{ AopTargetPath, "AopTargetPath", 0, &opx_targetPath },
	{ AopEnumerate, "AopEnumerate", 0, &opx_enumerate },
	{ AopNewAdd, "AopNewAdd", 0, &opx_newAdd },
	{ AopNewLessThan, "AopNewLessThan", 0, &opx_newLessThan },
	{ AopNewEquals, "AopNewEquals", 0, &opx_newEquals },
	{ AopToNumber, "AopToNumber", 0, &opx_toNumber },
	{ AopToString, "AopToString", 0, &opx_toString },
	{ AopDup, "AopDup", 0, &opx_dup },
	{ AopSwap, "AopSwap", 0, &opx_swap },
	{ AopGetMember, "AopGetMember", 0, &opx_getMember },
	{ AopSetMember, "AopSetMember", 0, &opx_setMember },

	// 0x50
	{ AopIncrement, "AopIncrement", 0, &opx_increment },
	{ AopDecrement, "AopDecrement", 0, &opx_decrement },
	{ AopCallMethod, "AopCallMethod", 0, &opx_callMethod },
	{ AopNewMethod, "AopNewMethod", 0, &opx_newMethod },
	{ AopInstanceOf, "AopInstanceOf", 0, &opx_instanceOf },
	{ AopEnum2, "AopEnum2", 0, &opx_enum2 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },

	// 0x60
	{ AopBitwiseAnd, "AopBitwiseAnd", 0, &opx_bitwiseAnd },
	{ AopBitwiseOr, "AopBitwiseOr", 0, &opx_bitwiseOr },
	{ AopBitwiseXor, "AopBitwiseXor", 0, &opx_bitwiseXor },
	{ AopShiftLeft, "AopShiftLeft", 0, &opx_shiftLeft },
	{ AopShiftRight, "AopShiftRight", 0, &opx_shiftRight },
	{ AopShiftRight2, "AopShiftRight2", 0, &opx_shiftRight2 },
	{ AopStrictEq, "AopStrictEq", 0, &opx_strictEq },
	{ AopGreater, "AopGreater", 0, &opx_greater },
	{ AopStringGreater, "AopStringGreater", 0, &opx_stringGreater },
	{ AopExtends, "AopExtends", 0, &opx_extends },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },

	// 0x70
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },

	// 0x80
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopGotoFrame, "AopGotoFrame", &opp_gotoFrame, &opx_gotoFrame },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopGetUrl, "AopGetUrl", 0, &opx_getUrl },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopSetRegister, "AopSetRegister", 0, &opx_setRegister },
	{ AopConstantPool, "AopConstantPool", &opp_constantPool, &opx_constantPool },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopWaitForFrame, "AopWaitForFrame", 0, &opx_waitForFrame },
	{ AopSetTarget, "AopSetTarget", 0, &opx_setTarget },
	{ AopGotoLabel, "AopGotoLabel", 0, &opx_gotoLabel },
	{ AopWaitForFrameExpression, "AopWaitForFrameExpression", 0, &opx_waitForFrameExpression },
	{ AopDefineFunction2, "AopDefineFunction2", &opp_defineFunction2, &opx_defineFunction2 },
	{ AopTry, "AopTry", 0, &opx_try },

	// 0x90
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopWith, "AopWith", 0, &opx_with },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopPushData, "AopPushData", &opp_pushData, &opx_pushData },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopBranchAlways, "AopBranchAlways", &opp_branchAlways, &opx_branchAlways },
	{ AopGetUrl2, "AopGetUrl2", 0, &opx_getUrl2 },
	{ AopDefineFunction, "AopDefineFunction", &opp_defineFunction, &opx_defineFunction },
	{ AopReserved, "AopReserved", 0, 0 },
	{ AopBranchIfTrue, "AopBranchIfTrue", &opp_branchIfTrue, &opx_branchIfTrue },
	{ AopCallFrame, "AopCallFrame", 0, &opx_callFrame },
	{ AopGotoFrame2, "AopGotoFrame2", 0, &opx_gotoFrame2 }
};

	}
}
