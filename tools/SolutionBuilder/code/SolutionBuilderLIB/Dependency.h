#ifndef Dependency_H
#define Dependency_H

#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

class SolutionLoader;

class Dependency : public traktor::Serializable
{
	T_RTTI_CLASS(Dependency)

public:
	virtual std::wstring getName() const = 0;

	virtual std::wstring getLocation() const = 0;

	virtual bool resolve(SolutionLoader* solutionLoader) = 0;
};

#endif	// Dependency_H
