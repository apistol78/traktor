#ifndef traktor_render_EmitterVariable_H
#define traktor_render_EmitterVariable_H

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

enum EmitterVariableType
{
	VtVoid,
	VtFloat,
	VtFloat2,
	VtFloat3,
	VtFloat4,
	VtFloat4x4,
	VtTexture
};

struct EmitterVariable
{
	EmitterVariableType type;
	int reg;		//< Index to first register allocated for this variable.
	int size;		//< Number of registers allocated for this variable.
	bool temporary;
};

//@}

	}
}

#endif	// traktor_render_Variable_H
