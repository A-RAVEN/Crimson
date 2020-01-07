#pragma once
#include <deque>

namespace Crimson
{
	template<typename integer_type>
	class IndexPool
	{
	public:
		IndexPool() : m_Counter(0){};
		~IndexPool() {};
		integer_type Allocate()
		{
			if (!m_Availables.empty())
			{
				integer_type return_val = m_Availables.front();
				m_Availables.pop_front();
				return return_val;
			}
			return m_Counter++;
		}
		void Recycle(integer_type _id)
		{
			m_Availables.push_back(_id);
		}
		void Clear()
		{
			m_Counter = 0;
			m_Availables.clear();
		}
	private:
		integer_type				m_Counter;
		std::deque<integer_type>	m_Availables;
	};
}