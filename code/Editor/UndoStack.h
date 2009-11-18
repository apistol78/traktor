#ifndef traktor_UndoStack_H
#define traktor_UndoStack_H

#include "Core/RefArray.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace editor
	{

/*! \brief Undo/redo stack.
 * \ingroup Editor
 *
 * Editor helper class to simplify handling
 * of undo/redo mechanism.
 * This class will take a copy of the given
 * object and store it in a stack.
 */
class T_DLLCLASS UndoStack : public Object
{
	T_RTTI_CLASS;

public:
	bool push(ISerializable* current);

	Ref< ISerializable > undo(ISerializable* current);

	Ref< ISerializable > redo(ISerializable* current);

	bool canUndo() const;

	bool canRedo() const;

private:
	RefArray< ISerializable > m_stack;
	RefArray< ISerializable > m_undone;
};

	}
}

#endif	// traktor_UndoStack_H
