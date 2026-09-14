#ifndef SPECIALIZED_CONSTANTS_BUILDER_H
#define SPECIALIZED_CONSTANTS_BUILDER_H

#include "../../../include/dataWriter.h"
#include "../volk.h"

struct SpecConstHolder {
    std::vector<uint8_t> dataBuff;
    std::vector<VkSpecializationMapEntry> metadata;
};

class SpecializedConstantsBuilder
{
private:
    DataWriter m_dataWriter;
    std::vector<VkSpecializationMapEntry> m_metadata;
    uint32_t m_constantID;
    uint32_t m_maxConstants;

    inline constexpr void addMetadata(std::size_t size)
    {
        uint32_t offset = static_cast<uint32_t>(m_dataWriter.GetOffset());

        assert(m_constantID < m_maxConstants);
        auto &newRec = m_metadata[m_constantID];

        newRec.constantID = m_constantID;
        newRec.offset = offset;
        newRec.size = size;

        m_constantID++;
    }

public:
    /// Construct with initial data buffer size
    constexpr explicit SpecializedConstantsBuilder(std::size_t dataSize, std::size_t maxConstants = 64)
        : m_dataWriter(dataSize)
        , m_maxConstants(maxConstants)
        , m_metadata(maxConstants)
        , m_constantID(0)
    {
    }

    /// Get current data offset
    constexpr std::size_t GetOffset() const { return m_dataWriter.GetOffset(); }

    /// Set data offset explicitly
    constexpr SpecializedConstantsBuilder& SetOffset(std::size_t offset)
    {
        m_dataWriter.SetOffset(offset);
        return *this;
    }

    /// Write signed 8-bit integer
    constexpr SpecializedConstantsBuilder& WriteInt8(int8_t value)
    {
        constexpr auto WriteSize = 1u;
        addMetadata(WriteSize);
        m_dataWriter.WriteInt8(value);
        return *this;
    }

    /// Write unsigned 8-bit integer
    constexpr SpecializedConstantsBuilder& WriteUInt8(uint8_t value)
    {
        constexpr auto WriteSize = 1u;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt8(value);
        return *this;
    }

    /// Write big-endian signed 16-bit
    constexpr SpecializedConstantsBuilder& WriteInt16BE(int16_t value)
    {
        constexpr std::size_t WriteSize = 2;
        addMetadata(WriteSize);
        m_dataWriter.WriteInt16BE(value);
        return *this;
    }

    /// Write big-endian unsigned 16-bit
    constexpr SpecializedConstantsBuilder& WriteUInt16BE(uint16_t value)
    {
        constexpr std::size_t WriteSize = 2;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt16BE(value);
        return *this;
    }

    /// Write big-endian unsigned 24-bit
    constexpr SpecializedConstantsBuilder& WriteUInt24BE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 3;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt24BE(value);
        return *this;
    }

    /// Write big-endian signed 32-bit
    constexpr SpecializedConstantsBuilder& WriteInt32BE(int32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        addMetadata(WriteSize);
        m_dataWriter.WriteInt32BE(value);
        return *this;
    }

    /// Write big-endian unsigned 32-bit
    constexpr SpecializedConstantsBuilder& WriteUInt32BE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt32BE(value);
        return *this;
    }

    /// Write little-endian unsigned 32-bit
    constexpr SpecializedConstantsBuilder& WriteInt32LE(uint32_t value)
    {
        constexpr std::size_t WriteSize = 4;
        addMetadata(WriteSize);
        m_dataWriter.WriteInt32LE(value);
        return *this;
    }

    /// Write big-endian unsigned 40-bit
    constexpr SpecializedConstantsBuilder& WriteUInt40BE(uint64_t value)
    {
        constexpr std::size_t WriteSize = 5;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt40BE(value);
        return *this;
    }

    /// Write little-endian unsigned 64-bit
    constexpr SpecializedConstantsBuilder& WriteUInt64LE(uint64_t value)
    {
        constexpr std::size_t WriteSize = 8;
        addMetadata(WriteSize);
        m_dataWriter.WriteUInt64LE(value);
        return *this;
    }

    /// Write a NUL-terminated string
    constexpr SpecializedConstantsBuilder& WriteNullTermString(const std::string& str)
    {
        std::size_t writeSize = str.size() + 1;
        addMetadata(writeSize);
        m_dataWriter.WriteNullTermString(str);
        return *this;
    }

    /// Write a byte array
    constexpr SpecializedConstantsBuilder& WriteUint8Array(const std::vector<uint8_t>& data)
    {
        addMetadata(data.size());
        m_dataWriter.WriteUint8Array(data);
        return *this;
    }

    /// Write raw bytes from a pointer
    constexpr SpecializedConstantsBuilder& WriteBytes(const uint8_t* data, std::size_t size)
    {
        addMetadata(size);
        m_dataWriter.WriteBytes(data, size);
        return *this;
    }

    /// Finalize and return both data buffer and metadata as VkSpecializationMapEntry vector

    constexpr SpecConstHolder toBuffer() const
    {
        assert(m_metadata.size() == m_constantID);
        return SpecConstHolder({std::move(m_dataWriter.toBuffer()), std::move(m_metadata)});
    }


    // constexpr std::vector<uint8_t> toBuffer() const
    // {
    //     return std::move(m_dataWriter.toBuffer());
    // }
};
#endif
