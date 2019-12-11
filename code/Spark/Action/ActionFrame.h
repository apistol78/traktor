#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Spark/Action/ActionValue.h"
#include "Spark/Action/ActionValueArray.h"
#include "Spark/Action/ActionValueStack.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class ActionContext;
class ActionObject;
class ActionDictionary;
class ActionFunction;

/*! ActionScript execution stack frame.
 * \ingroup Spark
 */
class T_DLLCLASS ActionFrame : public Object
{
	T_RTTI_CLASS;

public:
	ActionFrame(
		ActionContext* context,
		ActionObject* self,
		uint16_t localRegisters,
		const ActionDictionary* dictionary,
		ActionFunction* callee
	);

	void setRegister(uint16_t index, const ActionValue& value) { m_localRegisters[index] = value; }

	const ActionValue& getRegister(uint16_t index) const { return m_localRegisters[index]; }

	bool hasVariable(uint32_t variableName) const;

	void setVariable(uint32_t variableName, const ActionValue& variableValue);

	bool getVariable(uint32_t variableName, ActionValue& outVariableValue) const;

	ActionValue* getVariableValue(uint32_t variableName);

	bool hasScopeVariable(uint32_t variableName) const;

	void setScopeVariables(const SmallMap< uint32_t, ActionValue >& variables);

	void setScopeVariable(uint32_t variableName, const ActionValue& variableValue);

	bool getScopeVariable(uint32_t variableName, ActionValue& outVariableValue) const;

	ActionValue* getScopeVariableValue(uint32_t variableName);

	void setDictionary(const ActionDictionary* dictionary);

	ActionContext* getContext() const { return m_context; }

	ActionObject* getSelf() const { return m_self; }

	const ActionValueArray& getRegisters() const { return m_localRegisters; }

	const SmallMap< uint32_t, ActionValue >& getVariables() const { return m_localVariables; }

	const SmallMap< uint32_t, ActionValue >& getScopeVariables() const { return m_scopeVariables; }

	const ActionDictionary* getDictionary() const { return m_dictionary; }

	ActionFunction* getCallee() const { return m_callee; }

	ActionValueStack& getStack() { return m_stack; }

private:
	ActionContext* m_context;
	Ref< ActionObject > m_self;
	ActionValueArray m_localRegisters;
	SmallMap< uint32_t, ActionValue > m_localVariables;
	SmallMap< uint32_t, ActionValue > m_scopeVariables;
	Ref< const ActionDictionary > m_dictionary;
	ActionFunction* m_callee;
	ActionValueStack m_stack;
};

	}
}

