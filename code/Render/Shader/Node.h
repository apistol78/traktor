#ifndef traktor_render_Node_H
#define traktor_render_Node_H

#include <string>
#include <vector>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class InputPin;
class OutputPin;

/*! \brief Shader graph node.
 * \ingroup Render
 */
class T_DLLCLASS Node : public ISerializable
{
	T_RTTI_CLASS;

public:
	Node();

	/*! \brief Set comment.
	 *
	 * \param comment New comment.
	 */
	void setComment(const std::wstring& comment);

	/*! \brief Get comment.
	 *
	 * \return Comment.
	 */
	const std::wstring& getComment() const;

	/*! \brief Get information.
	 *
	 * \return Information.
	 */
	virtual std::wstring getInformation() const;

	/*! \brief Set position.
	 *
	 * \param position New position.
	 */
	void setPosition(const std::pair< int, int >& position);

	/*! \brief Get position.
	 *
	 * \return Position.
	 */
	const std::pair< int, int >& getPosition() const;

	/*! \brief Get number of input pins.
	 *
	 * \return Number of input pins.
	 */
	virtual int getInputPinCount() const = 0;

	/*! \brief Get input pin.
	 *
	 * \param index Index of input pin.
	 * \return Pointer to input pin, null if no such input pin.
	 */
	virtual const InputPin* getInputPin(int index) const = 0;

	/*! \brief Get number of output pins.
	 *
	 * \return Number of output pins.
	 */
	virtual int getOutputPinCount() const = 0;

	/*! \brief Get output pin.
	 *
	 * \param index Index of output pin.
	 * \return Pointer to output pin, null if no such output pin.
	 */
	virtual const OutputPin* getOutputPin(int index) const = 0;

	/*! \brief Find input pin by name.
	 *
	 * \param name Name of input pin.
	 * \return Pointer to input pin, null if no such input pin.
	 */
	const InputPin* findInputPin(const std::wstring& name) const;

	/*! \brief Find output pin by name.
	 *
	 * \param name Name of output pin.
	 * \return Pointer to output pin, null if no such output pin.
	 */
	const OutputPin* findOutputPin(const std::wstring& name) const;
	
	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	std::wstring m_comment;
	std::pair< int, int > m_position;
};

	}
}

#endif	// traktor_render_Node_H
