#ifndef DATAWRITER_H
#define DATAWRITER_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <string>
#include <utility>

class DataWriter
{
private:
    std::vector<uint8_t> m_buffer;
    std::size_t m_offset;

public:
    /// Construct with initial buffer size
    constexpr explicit DataWriter(std::size_t size)
        : m_buffer(size, 0)
        , m_offset(0)
    {
    }

    /// Get current offset
    constexpr std::size_t GetOffset() const { return m_offset; }

    /// Set offset explicitly (must still be in-range)
    constexpr DataWriter& SetOffset(std::size_t offset)
    {
        assert(offset <= m_buffer.size());
        m_offset = offset;
        return *this;
    }

    /// Write signed 8-bit integer
    constexpr DataWriter& WriteInt8(int8_t value)
    {
        constexpr auto WriteSize = 1u;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset] = static_cast<uint8_t>(value);
        m_offset += WriteSize;
        return *this;
    }

    /// Write unsigned 8-bit integer
    constexpr DataWriter& WriteUInt8(uint8_t value)
    {
        constexpr auto WriteSize = 1u;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset] = value;
        m_offset += WriteSize;
        return *this;
    }

    /// Write big-endian signed 16-bit
    constexpr DataWriter& WriteInt16BE(int16_t value)
    {
        constexpr std::size_t WriteSize = 2;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write big-endian unsigned 16-bit
    constexpr DataWriter& WriteUInt16BE(uint16_t value)
    {
        constexpr std::size_t WriteSize = 2;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write big-endian unsigned 24-bit
    constexpr DataWriter& WriteUInt24BE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 3;
        assert(m_offset + WriteSize <= m_buffer.size());
        assert(value <= 0xFFFFFF); // Ensure it fits in 24 bits

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write big-endian signed 32-bit
    constexpr DataWriter& WriteInt32BE(int32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write big-endian unsigned 32-bit
    constexpr DataWriter& WriteUInt32BE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write little-endian signed 32-bit
    constexpr DataWriter& WriteInt32LE(int32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>(value & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write little-endian unsigned 32-bit
    constexpr DataWriter& WriteUInt32LE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>(value & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);

        m_offset += WriteSize;
        return *this;
    }


    /// Write big-endian unsigned 40-bit
    constexpr DataWriter& WriteUInt40BE(uint64_t value)
    {
        constexpr std::size_t WriteSize = 5;
        assert(m_offset + WriteSize <= m_buffer.size());
        assert(value <= 0xFFFFFFFFFFULL); // Ensure it fits in 40 bits

        m_buffer[m_offset]     = static_cast<uint8_t>((value >> 32) & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 4] = static_cast<uint8_t>(value & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write little-endian unsigned 64-bit
    constexpr DataWriter& WriteUInt64LE(uint64_t value)
    {
        constexpr std::size_t WriteSize = 8;
        assert(m_offset + WriteSize <= m_buffer.size());

        m_buffer[m_offset]     = static_cast<uint8_t>(value & 0xFF);
        m_buffer[m_offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        m_buffer[m_offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        m_buffer[m_offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        m_buffer[m_offset + 4] = static_cast<uint8_t>((value >> 32) & 0xFF);
        m_buffer[m_offset + 5] = static_cast<uint8_t>((value >> 40) & 0xFF);
        m_buffer[m_offset + 6] = static_cast<uint8_t>((value >> 48) & 0xFF);
        m_buffer[m_offset + 7] = static_cast<uint8_t>((value >> 56) & 0xFF);

        m_offset += WriteSize;
        return *this;
    }

    /// Write a NUL-terminated string
    constexpr DataWriter& WriteNullTermString(const std::string& str)
    {
        std::size_t writeSize = str.size() + 1; // +1 for NUL terminator
        assert(m_offset + writeSize <= m_buffer.size());

        // Copy string data
        for (std::size_t i = 0; i < str.size(); ++i)
        {
            m_buffer[m_offset + i] = static_cast<uint8_t>(str[i]);
        }
        // Add NUL terminator
        m_buffer[m_offset + str.size()] = 0;

        m_offset += writeSize;
        return *this;
    }

    /// Write a byte array
    constexpr DataWriter& WriteUint8Array(const std::vector<uint8_t>& data)
    {
        assert(m_offset + data.size() <= m_buffer.size());

        for (std::size_t i = 0; i < data.size(); ++i)
        {
            m_buffer[m_offset + i] = data[i];
        }

        m_offset += data.size();
        return *this;
    }

    /// Write raw bytes from a pointer
    constexpr DataWriter& WriteBytes(const uint8_t* data, std::size_t size)
    {
        assert(m_offset + size <= m_buffer.size());

        for (std::size_t i = 0; i < size; ++i)
        {
            m_buffer[m_offset + i] = data[i];
        }

        m_offset += size;
        return *this;
    }

    /// Finalize and return the buffer
    constexpr std::vector<uint8_t> toBuffer() const
    {
        return std::move(m_buffer);
    }
};

#endif// DATAWRITER_H