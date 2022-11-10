#pragma once

#include "Drawing/ITransferFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Common blend transfer function.
 * \ingroup Drawing
 */
class T_DLLCLASS BlendFunction : public ITransferFunction
{
	T_RTTI_CLASS;

public:
	enum Factor
	{
		BfOne,
		BfZero,
		BfSourceColor,
		BfOneMinusSourceColor,
		BfDestinationColor,
		BfOneMinusDestinationColor,
		BfSourceAlpha,
		BfOneMinusSourceAlpha,
		BfDestinationAlpha,
		BfOneMinusDestinationAlpha
	};

	enum Operation
	{
		BoAdd,
		BoSubtract,
		BoReverseSubtract,
		BoMin,
		BoMax
	};

	explicit BlendFunction(Factor sourceFactor, Factor destinationFactor, Operation operation);

protected:
	virtual void evaluate(const Color4f& in, Color4f& out) const override final;

private:
	Factor m_sourceFactor;
	Factor m_destinationFactor;
	Operation m_operation;
};

}
