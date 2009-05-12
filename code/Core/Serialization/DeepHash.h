#ifndef traktor_DeepHash_H
#define traktor_DeepHash_H

#include "Core/Object.h"
#include "Core/Misc/MD5.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;

/*! \brief MD5 hash of object.
 * \ingroup Core
 *
 * Calculates a MD5 hash from a given object
 * using serialization, thus other objects referenced
 * by the object is part of the final hash value.
 */
class T_DLLCLASS DeepHash : public Object
{
	T_RTTI_CLASS(DeepHash)

public:
	DeepHash(const Serializable* object);

	const MD5& getMD5() const;

	bool operator == (const DeepHash& hash) const;

	bool operator != (const DeepHash& hash) const;

	bool operator == (const DeepHash* hash) const;

	bool operator != (const DeepHash* hash) const;

	bool operator == (const Serializable* object) const;

	bool operator != (const Serializable* object) const;

	bool operator == (const MD5& md5) const;

	bool operator != (const MD5& md5) const;

private:
	MD5 m_md5;
};

}

#endif	// traktor_DeepHash_H
