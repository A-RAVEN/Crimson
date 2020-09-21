#pragma once
#include <deque>
#include <vector>

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

	class ByteVector
	{
	public:
		template<typename push_in_type>
		void PushData(push_in_type const& data)
		{
			size_t originalSize = dataVector.size();
			dataVector.resize(originalSize + sizeof(data));
			memcpy(&dataVector[originalSize], &data, sizeof(data));
		}

		void Clear()
		{
			dataVector.clear();
		}
	private:
		std::vector<byte> dataVector;
	};
}