#ifndef traktor_UndoStack_H
#define traktor_UndoStack_H

#include "Core/Heap/Ref.h"
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

class Serializable;

	namespace editor
	{

/*! \brief Undo/redo stack.
 *
 * Editor helper class to simplify handling
 * of undo/redo mechanism.
 * This class will take a copy of the given
 * object and store it in a stack.
 */
class T_DLLCLASS UndoStack : public Object
{
	T_RTTI_CLASS(UndoStack)

public:
	bool push(Serializable* current);

	Serializable* undo(Serializable* current);

	Serializable* redo(Serializable* current);

	bool canUndo() const;

	bool canRedo() const;

private:
	RefList< Serializable > m_stack;
	RefList< Serializable > m_undone;
};

	}
}

#endif	// traktor_UndoStack_H
