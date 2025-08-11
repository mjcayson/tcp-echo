#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace tcp_echo::util
{
    // Tiny growable byte buffer with append/consume helpers
    class Buffer
    {
    public:
        void Append(const uint8_t* p, std::size_t n)
        {
            m_data.insert(m_data.end(), p, p + n);
        }
        
        void Append(const std::vector<uint8_t>& v)
        {
            Append(v.data(), v.size()); 
        }

        // Consume first n bytes (no bounds check for speed; call carefully)
        void Consume(std::size_t numBytes)
        {
            if (numBytes >= m_data.size())
            {
                m_data.clear();
                return;
            }
            m_data.erase(m_data.begin(), m_data.begin() + static_cast<long>(numBytes));
        }

        std::size_t size() const { return m_data.size(); }
        bool empty() const { return m_data.empty(); }

        uint8_t* data() { return m_data.data(); }
        const uint8_t* data() const { return m_data.data(); }

        std::vector<uint8_t>& vec() { return m_data; }
        const std::vector<uint8_t>& vec() const { return m_data; }

    private:
        std::vector<uint8_t> m_data;
    };
} // namespace tcp_echo::util
