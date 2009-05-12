#include <iostream>
#include "UnitTest/Case.h"

namespace traktor
{

void Case::succeeded(const std::string& message)
{
	std::cout << message << std::endl;
}

void Case::failed(const std::string& message)
{
	std::cerr << message << std::endl;
}

}
