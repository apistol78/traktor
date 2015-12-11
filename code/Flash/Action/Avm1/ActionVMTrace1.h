#ifndef traktor_flash_ActionVMTrace1_H
#define traktor_flash_ActionVMTrace1_H

#include <vector>
#include "Core/Object.h"

namespace traktor
{

class OutputStream;

	namespace flash
	{

class ActionValue;
struct ExecutionState;
struct OperationInfo;

class ActionVMTrace1 : public Object
{
	T_RTTI_CLASS;

public:
	ActionVMTrace1();

	virtual ~ActionVMTrace1();

	void beginDispatcher();

	void endDispatcher();

	void preDispatch(const ExecutionState& state, const OperationInfo& info);
	
	void postDispatch(const ExecutionState& state, const OperationInfo& info);

	OutputStream& getTraceStream();
	
private:
	Ref< OutputStream > m_stream;
};

	}
}

#endif	// traktor_flash_ActionVMTrace1_H
