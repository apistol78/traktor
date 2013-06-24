#ifndef Dependency_H
#define Dependency_H

#include <Core/Serialization/ISerializable.h>

class SolutionLoader;

class Dependency : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	Dependency();

	void setLinkWithProduct(bool linkWithProduct);

	bool shouldLinkWithProduct() const;

	virtual std::wstring getName() const = 0;

	virtual std::wstring getLocation() const = 0;

	virtual bool resolve(SolutionLoader* solutionLoader) = 0;

	virtual void serialize(traktor::ISerializer& s);

private:
	bool m_linkWithProduct;
};

#endif	// Dependency_H
