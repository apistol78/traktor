/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IDocument_H
#define traktor_editor_IDocument_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace db
	{

class Instance;

	}

	namespace editor
	{

/*! \brief Editor page working document.
 * \ingroup Editor
 *
 * Each open editor page has an associated
 * document which can contain multiple database
 * instances.
 *
 * When the editor page is created it's provided
 * with a document containing the instance the
 * user opened. If more instances needs to be
 * edited in the same editor page these should
 * then be added to the document.
 */
class T_DLLCLASS IDocument : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Edit instance in document.
	 *
	 * Instances which are logically edited
	 * within a document, or more specifically through an
	 * editor page, must be registered with
	 * a call to editInstance.
	 *
	 * \param instance Database instance.
	 * \param object Database instance object.
	 */
	virtual void editInstance(db::Instance* instance, ISerializable* object) = 0;

	/*! \brief Check if instance is already edited through this document.
	 *
	 * \param instance Database instance.
	 * \return True if it belongs to this document.
	 */
	virtual bool containInstance(db::Instance* instance) const = 0;

	/*! \brief Get number of editing instances.
	 *
	 * \return Number of editing instances.
	 */
	virtual uint32_t getInstanceCount() const = 0;

	/*! \brief Get editing instance.
	 *
	 * \param index Index of editing instance.
	 * \return Editing instance.
	 */
	virtual db::Instance* getInstance(uint32_t index) const = 0;

	/*! \brief Get editing object.
	 *
	 * \param index Index of editing object.
	 * \return Editing object.
	 */
	virtual ISerializable* getObject(uint32_t index) const = 0;

	/*! \brief Replace editing object.
	 *
	 * \param index Index of editing object.
	 * \param object New editing object.
	 * \return True if successful.
	 */
	virtual bool setObject(uint32_t index, ISerializable* object) = 0;

	/*! \brief Force document to be modified.
	 *
	 * Document are tagged as modified even if all objects
	 * hasn't changed.
	 *
	 * Modified flag is reset when document is saved.
	 */
	virtual void setModified() = 0;

	/*! \brief Push current state of document.
	 *
	 * Push document state onto undo stack.
	 *
	 * \param meta Optional meta object associated with this push.
	 */
	virtual void push(const ISerializable* meta) = 0;

	/*! \brief Undo state of document.
	 *
	 * \param redoMeta Meta object in case this undo will get redo;ed.
	 * \param outMeta Meta object associated with pushed state.
	 * \return True if state recovered.
	 */
	virtual bool undo(const ISerializable* redoMeta, Ref< const ISerializable >* outMeta) = 0;

	/*! \brief Redo state of document.
	 *
	 * \param outMeta Meta object associated with pushed state.
	 * \return True if state recovered.
	 */
	virtual bool redo(Ref< const ISerializable >* outMeta) = 0;

	/*! \brief Get editing object.
	 *
	 * \param T Type of editing object.
	 * \param index Index of editing object.
	 * \return Editing object.
	 */
	template < typename T >
	T* getObject(uint32_t index) const
	{
		return dynamic_type_cast< T* >(getObject(index));
	}

	/*! \brief Push current state of document.
	 *
	 * Push document state onto undo stack.
	 */
	void push()
	{
		push(nullptr);
	}

	/*! \brief Undo state of document.
	 *
	 * \return True if state recovered.
	 */
	bool undo()
	{
		return undo(nullptr, nullptr);
	}

	/*! \brief Undo state of document.
	 *
	 * \param redoMeta Meta object in case this undo will get redo;ed.
	 * \param outMeta Meta object associated with pushed state.
	 * \return True if state recovered.
	 */
	template < typename T >
	bool undo(const T* redoMeta, Ref< const T >& outMeta)
	{
		Ref< const ISerializable > undoMeta;
		if (!undo(redoMeta, &undoMeta))
			return false;
		outMeta = dynamic_type_cast< const T* >(undoMeta);
		return true;
	}

	/*! \brief Redo state of document.
	 *
	 * \return True if state recovered.
	 */
	bool redo()
	{
		return redo(nullptr);
	}

	/*! \brief Redo state of document.
	 *
	 * \param outMeta Meta object associated with pushed state.
	 * \return True if state recovered.
	 */
	template < typename T >
	bool redo(Ref< const T >& outMeta)
	{
		Ref< const ISerializable > redoMeta;
		if (!redo(&redoMeta))
			return false;
		outMeta = dynamic_type_cast< const T* >(redoMeta);
		return true;
	}
};

	}
}

#endif	// traktor_editor_IDocument_H
