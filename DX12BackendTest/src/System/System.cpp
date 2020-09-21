#include <headers/System/System.h>

void ISystem::ConfigureListeningBits(std::vector<ComponentBits> const& bits)
{
	m_ListeningBits = bits;
}

bool ISystem::TestingBits(ComponentBits const& srcbits)
{
	for (auto& bits : m_ListeningBits)
	{
		if ((bits & srcbits) == bits)
		{
			return true;
		}
	}
	return false;
}
