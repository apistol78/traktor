#ifndef traktor_render_Variable_H
#define traktor_render_Variable_H

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

enum VariableType
{
	VtVoid,
	VtFloat,
	VtFloat2,
	VtFloat3,
	VtFloat4,
	VtFloat4x4,
	VtTexture
};

struct Variable
{
	VariableType type;
	int reg;		//< Index to first register allocated for this variable.
	int size;		//< Number of registers allocated for this variable.
};

//@}

	}
}

#endif	// traktor_render_Variable_H
