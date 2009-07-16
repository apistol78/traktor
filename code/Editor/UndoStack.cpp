#include "Editor/UndoStack.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/DeepClone.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.UndoStack", UndoStack, Object)

bool UndoStack::push(Serializable* current)
{
	T_ASSERT_M (current, L"Cannot push null objects onto undo stack");

	if (!m_stack.empty())
	{
		uint32_t hash1 = DeepHash(current).get();
		uint32_t hash2 = DeepHash(m_stack.back()).get();
		if (hash1 == hash2)
			return true;
	}

	Ref< Serializable > clone = DeepClone(current).create();
	if (!clone)
		return false;

	m_stack.push_back(clone);
	m_undone.clear();

	return true;
}

Serializable* UndoStack::undo(Serializable* current)
{
	if (m_stack.empty())
		return 0;

	Ref< Serializable > object = m_stack.back();
	m_stack.pop_back();

	if (current)
		m_undone.push_back(current);

	return object;
}

Serializable* UndoStack::redo(Serializable* current)
{
	if (m_undone.empty())
		return 0;

	Ref< Serializable > object = m_undone.back();
	m_undone.pop_back();

	if (current)
		m_stack.push_back(current);

	return object;
}

bool UndoStack::canUndo() const
{
	return !m_stack.empty();
}

bool UndoStack::canRedo() const
{
	return !m_undone.empty();
}

	}
}
