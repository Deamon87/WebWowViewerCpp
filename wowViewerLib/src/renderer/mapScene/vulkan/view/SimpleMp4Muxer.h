//
// Simple MP4 Muxer for H.264 Annex B streams
// Converts raw H.264 bitstream to valid MP4 container format
//

#ifndef SIMPLE_MP4_MUXER_H
#define SIMPLE_MP4_MUXER_H

#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <iostream>

class SimpleMp4Muxer {
public:
    SimpleMp4Muxer(const std::string& filename, uint32_t width, uint32_t height, uint32_t fps)
        : m_width(width), m_height(height), m_fps(fps), m_timeScale(fps * 1000), m_frameCount(0) {
        m_file.open(filename, std::ios::binary);
        if (!m_file.is_open()) {
            throw std::runtime_error("Failed to open output file: " + filename);
        }
        
        // Write ftyp box immediately
        writeFtypBox();
        
        // Reserve space for mdat header (will be updated at finalization)
        m_mdatHeaderPos = m_file.tellp();
        writeMdatHeader(0); // Placeholder, will update with actual size
    }
    
    ~SimpleMp4Muxer() {
        if (m_file.is_open()) {
            finalize();
        }
    }
    
    // Add a frame (H.264 Annex B format with 00 00 00 01 start codes)
    void addFrame(const uint8_t* data, size_t size, bool isKeyframe) {
        if (size == 0) return;
        
        // Parse NAL units and extract SPS/PPS if this is first keyframe
        if (isKeyframe && m_sps.empty()) {
            extractSpsPps(data, size);
        }
        
        // Convert Annex B to AVCC format and write to mdat
        std::vector<uint8_t> avccData = annexBToAvcc(data, size);
        
        if (!avccData.empty()) {
            // Track sample info
            SampleInfo sample;
            sample.size = static_cast<uint32_t>(avccData.size());
            sample.isKeyframe = isKeyframe;
            sample.duration = m_timeScale / m_fps; // Duration in timescale units
            m_samples.push_back(sample);
            
            // Write to mdat section
            m_file.write(reinterpret_cast<const char*>(avccData.data()), avccData.size());
            m_mdatSize += avccData.size();
            m_frameCount++;
        }
    }
    
    void finalize() {
        if (!m_file.is_open()) return;
        
        std::streampos endPos = m_file.tellp();
        
        // Update mdat header with actual size
        m_file.seekp(m_mdatHeaderPos);
        writeMdatHeader(m_mdatSize);
        
        // Seek to end and write moov box
        m_file.seekp(endPos);
        writeMoovBox();
        
        m_file.close();
        std::cout << "MP4 file finalized. Frames: " << m_frameCount << std::endl;
    }
    
private:
    struct SampleInfo {
        uint32_t size;
        uint32_t duration;
        bool isKeyframe;
    };
    
    std::ofstream m_file;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_fps;
    uint32_t m_timeScale;
    uint32_t m_frameCount;
    std::streampos m_mdatHeaderPos;
    uint64_t m_mdatSize = 0;
    
    std::vector<SampleInfo> m_samples;
    std::vector<uint8_t> m_sps;
    std::vector<uint8_t> m_pps;
    
    // Helper to write big-endian values
    void writeU32BE(uint32_t val) {
        uint8_t buf[4] = {
            static_cast<uint8_t>((val >> 24) & 0xFF),
            static_cast<uint8_t>((val >> 16) & 0xFF),
            static_cast<uint8_t>((val >> 8) & 0xFF),
            static_cast<uint8_t>(val & 0xFF)
        };
        m_file.write(reinterpret_cast<char*>(buf), 4);
    }
    
    void writeU16BE(uint16_t val) {
        uint8_t buf[2] = {
            static_cast<uint8_t>((val >> 8) & 0xFF),
            static_cast<uint8_t>(val & 0xFF)
        };
        m_file.write(reinterpret_cast<char*>(buf), 2);
    }
    
    void writeU64BE(uint64_t val) {
        writeU32BE(static_cast<uint32_t>(val >> 32));
        writeU32BE(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    
    void writeFourCC(const char* fourcc) {
        m_file.write(fourcc, 4);
    }
    
    void writeBytes(const uint8_t* data, size_t size) {
        m_file.write(reinterpret_cast<const char*>(data), size);
    }
    
    void writeZeros(size_t count) {
        std::vector<uint8_t> zeros(count, 0);
        m_file.write(reinterpret_cast<char*>(zeros.data()), count);
    }
    
    // Find NAL unit start codes
    std::vector<std::pair<size_t, size_t>> findNalUnits(const uint8_t* data, size_t size) {
        std::vector<std::pair<size_t, size_t>> nalUnits;
        
        size_t i = 0;
        while (i < size) {
            // Look for start code (00 00 00 01 or 00 00 01)
            size_t startCodeLen = 0;
            if (i + 3 < size && data[i] == 0 && data[i+1] == 0 && data[i+2] == 0 && data[i+3] == 1) {
                startCodeLen = 4;
            } else if (i + 2 < size && data[i] == 0 && data[i+1] == 0 && data[i+2] == 1) {
                startCodeLen = 3;
            }
            
            if (startCodeLen > 0) {
                size_t nalStart = i + startCodeLen;
                
                // Find end of this NAL unit (next start code or end of data)
                size_t nalEnd = size;
                for (size_t j = nalStart; j + 2 < size; j++) {
                    if (data[j] == 0 && data[j+1] == 0 && 
                        (data[j+2] == 1 || (j + 3 < size && data[j+2] == 0 && data[j+3] == 1))) {
                        nalEnd = j;
                        break;
                    }
                }
                
                if (nalEnd > nalStart) {
                    nalUnits.push_back({nalStart, nalEnd - nalStart});
                }
                i = nalEnd;
            } else {
                i++;
            }
        }
        
        return nalUnits;
    }
    
    void extractSpsPps(const uint8_t* data, size_t size) {
        auto nalUnits = findNalUnits(data, size);
        
        for (const auto& [offset, length] : nalUnits) {
            if (length == 0) continue;
            
            uint8_t nalType = data[offset] & 0x1F;
            
            if (nalType == 7 && m_sps.empty()) { // SPS
                m_sps.assign(data + offset, data + offset + length);
                std::cout << "Extracted SPS: " << length << " bytes" << std::endl;
            } else if (nalType == 8 && m_pps.empty()) { // PPS
                m_pps.assign(data + offset, data + offset + length);
                std::cout << "Extracted PPS: " << length << " bytes" << std::endl;
            }
        }
    }
    
    // Convert H.264 Annex B (start codes) to AVCC format (length prefixes)
    std::vector<uint8_t> annexBToAvcc(const uint8_t* data, size_t size) {
        std::vector<uint8_t> result;
        auto nalUnits = findNalUnits(data, size);
        
        for (const auto& [offset, length] : nalUnits) {
            if (length == 0) continue;
            
            uint8_t nalType = data[offset] & 0x1F;
            
            // Skip SPS (7), PPS (8), and AUD (9) - they're in the header
            if (nalType == 7 || nalType == 8 || nalType == 9) {
                continue;
            }
            
            // Write 4-byte length prefix (big-endian)
            uint32_t len = static_cast<uint32_t>(length);
            result.push_back((len >> 24) & 0xFF);
            result.push_back((len >> 16) & 0xFF);
            result.push_back((len >> 8) & 0xFF);
            result.push_back(len & 0xFF);
            
            // Write NAL unit data
            result.insert(result.end(), data + offset, data + offset + length);
        }
        
        return result;
    }
    
    void writeFtypBox() {
        // ftyp box
        writeU32BE(20); // Box size
        writeFourCC("ftyp");
        writeFourCC("isom"); // Major brand
        writeU32BE(512);     // Minor version
        writeFourCC("isom"); // Compatible brand
    }
    
    void writeMdatHeader(uint64_t size) {
        if (size + 8 > 0xFFFFFFFF) {
            // Extended size (64-bit)
            writeU32BE(1);
            writeFourCC("mdat");
            writeU64BE(size + 16);
        } else {
            writeU32BE(static_cast<uint32_t>(size + 8));
            writeFourCC("mdat");
        }
    }
    
    void writeMoovBox() {
        std::streampos moovStart = m_file.tellp();
        
        // Write placeholder for size
        writeU32BE(0);
        writeFourCC("moov");
        
        writeMvhdBox();
        writeTrakBox();
        
        // Update moov box size
        std::streampos moovEnd = m_file.tellp();
        uint32_t moovSize = static_cast<uint32_t>(moovEnd - moovStart);
        m_file.seekp(moovStart);
        writeU32BE(moovSize);
        m_file.seekp(moovEnd);
    }
    
    void writeMvhdBox() {
        uint64_t duration = static_cast<uint64_t>(m_samples.size()) * m_timeScale / m_fps;
        
        writeU32BE(108); // Box size
        writeFourCC("mvhd");
        writeU32BE(0);   // Version and flags
        writeU32BE(0);   // Creation time
        writeU32BE(0);   // Modification time
        writeU32BE(m_timeScale); // Timescale
        writeU32BE(static_cast<uint32_t>(duration)); // Duration
        writeU32BE(0x00010000); // Rate (1.0)
        writeU16BE(0x0100);     // Volume (1.0)
        writeZeros(10);         // Reserved
        
        // Matrix (identity)
        writeU32BE(0x00010000); writeU32BE(0); writeU32BE(0);
        writeU32BE(0); writeU32BE(0x00010000); writeU32BE(0);
        writeU32BE(0); writeU32BE(0); writeU32BE(0x40000000);
        
        writeZeros(24); // Pre-defined
        writeU32BE(2);  // Next track ID
    }
    
    void writeTrakBox() {
        std::streampos trakStart = m_file.tellp();
        
        writeU32BE(0); // Placeholder
        writeFourCC("trak");
        
        writeTkhdBox();
        writeMdiaBox();
        
        std::streampos trakEnd = m_file.tellp();
        uint32_t trakSize = static_cast<uint32_t>(trakEnd - trakStart);
        m_file.seekp(trakStart);
        writeU32BE(trakSize);
        m_file.seekp(trakEnd);
    }
    
    void writeTkhdBox() {
        uint64_t duration = static_cast<uint64_t>(m_samples.size()) * m_timeScale / m_fps;
        
        writeU32BE(92); // Box size
        writeFourCC("tkhd");
        writeU32BE(0x00000003); // Version=0, flags=enabled|in_movie
        writeU32BE(0);   // Creation time
        writeU32BE(0);   // Modification time
        writeU32BE(1);   // Track ID
        writeU32BE(0);   // Reserved
        writeU32BE(static_cast<uint32_t>(duration)); // Duration
        writeZeros(8);   // Reserved
        writeU16BE(0);   // Layer
        writeU16BE(0);   // Alternate group
        writeU16BE(0);   // Volume (0 for video)
        writeU16BE(0);   // Reserved
        
        // Matrix (identity)
        writeU32BE(0x00010000); writeU32BE(0); writeU32BE(0);
        writeU32BE(0); writeU32BE(0x00010000); writeU32BE(0);
        writeU32BE(0); writeU32BE(0); writeU32BE(0x40000000);
        
        writeU32BE(m_width << 16);  // Width (fixed point 16.16)
        writeU32BE(m_height << 16); // Height (fixed point 16.16)
    }
    
    void writeMdiaBox() {
        std::streampos mdiaStart = m_file.tellp();
        
        writeU32BE(0);
        writeFourCC("mdia");
        
        writeMdhdBox();
        writeHdlrBox();
        writeMinfBox();
        
        std::streampos mdiaEnd = m_file.tellp();
        uint32_t mdiaSize = static_cast<uint32_t>(mdiaEnd - mdiaStart);
        m_file.seekp(mdiaStart);
        writeU32BE(mdiaSize);
        m_file.seekp(mdiaEnd);
    }
    
    void writeMdhdBox() {
        uint64_t duration = static_cast<uint64_t>(m_samples.size()) * m_timeScale / m_fps;
        
        writeU32BE(32);
        writeFourCC("mdhd");
        writeU32BE(0);   // Version and flags
        writeU32BE(0);   // Creation time
        writeU32BE(0);   // Modification time
        writeU32BE(m_timeScale);
        writeU32BE(static_cast<uint32_t>(duration));
        writeU32BE(0x55C40000); // Language (undetermined) + quality
    }
    
    void writeHdlrBox() {
        writeU32BE(45);
        writeFourCC("hdlr");
        writeU32BE(0);   // Version and flags
        writeU32BE(0);   // Pre-defined
        writeFourCC("vide"); // Handler type
        writeZeros(12);  // Reserved
        m_file.write("VideoHandler", 12);
        m_file.put(0);   // Null terminator
    }
    
    void writeMinfBox() {
        std::streampos minfStart = m_file.tellp();
        
        writeU32BE(0);
        writeFourCC("minf");
        
        writeVmhdBox();
        writeDinfBox();
        writeStblBox();
        
        std::streampos minfEnd = m_file.tellp();
        uint32_t minfSize = static_cast<uint32_t>(minfEnd - minfStart);
        m_file.seekp(minfStart);
        writeU32BE(minfSize);
        m_file.seekp(minfEnd);
    }
    
    void writeVmhdBox() {
        writeU32BE(20);
        writeFourCC("vmhd");
        writeU32BE(1);   // Version=0, flags=1
        writeU16BE(0);   // Graphics mode
        writeU16BE(0);   // Opcolor R
        writeU16BE(0);   // Opcolor G
        writeU16BE(0);   // Opcolor B
    }
    
    void writeDinfBox() {
        writeU32BE(36);
        writeFourCC("dinf");
        
        // dref box
        writeU32BE(28);
        writeFourCC("dref");
        writeU32BE(0);   // Version and flags
        writeU32BE(1);   // Entry count
        
        // url box (self-contained)
        writeU32BE(12);
        writeFourCC("url ");
        writeU32BE(1);   // Flags = self-contained
    }
    
    void writeStblBox() {
        std::streampos stblStart = m_file.tellp();
        
        writeU32BE(0);
        writeFourCC("stbl");
        
        writeStsdBox();
        writeSttsBox();
        writeStscBox();
        writeStszBox();
        writeStcoBox();
        writeStssBox(); // Keyframe index
        
        std::streampos stblEnd = m_file.tellp();
        uint32_t stblSize = static_cast<uint32_t>(stblEnd - stblStart);
        m_file.seekp(stblStart);
        writeU32BE(stblSize);
        m_file.seekp(stblEnd);
    }
    
    void writeStsdBox() {
        std::streampos stsdStart = m_file.tellp();
        
        writeU32BE(0); // Placeholder
        writeFourCC("stsd");
        writeU32BE(0); // Version and flags
        writeU32BE(1); // Entry count
        
        writeAvc1Box();
        
        std::streampos stsdEnd = m_file.tellp();
        uint32_t stsdSize = static_cast<uint32_t>(stsdEnd - stsdStart);
        m_file.seekp(stsdStart);
        writeU32BE(stsdSize);
        m_file.seekp(stsdEnd);
    }
    
    void writeAvc1Box() {
        std::streampos avc1Start = m_file.tellp();
        
        writeU32BE(0); // Placeholder
        writeFourCC("avc1");
        writeZeros(6);      // Reserved
        writeU16BE(1);      // Data reference index
        writeU16BE(0);      // Pre-defined
        writeU16BE(0);      // Reserved
        writeZeros(12);     // Pre-defined
        writeU16BE(m_width);
        writeU16BE(m_height);
        writeU32BE(0x00480000); // Horizontal resolution (72 dpi)
        writeU32BE(0x00480000); // Vertical resolution (72 dpi)
        writeU32BE(0);          // Reserved
        writeU16BE(1);          // Frame count
        writeZeros(32);         // Compressor name
        writeU16BE(0x0018);     // Depth (24-bit)
        writeU16BE(0xFFFF);     // Pre-defined
        
        writeAvcCBox();
        
        std::streampos avc1End = m_file.tellp();
        uint32_t avc1Size = static_cast<uint32_t>(avc1End - avc1Start);
        m_file.seekp(avc1Start);
        writeU32BE(avc1Size);
        m_file.seekp(avc1End);
    }
    
    void writeAvcCBox() {
        // If we don't have SPS/PPS, create minimal ones
        if (m_sps.empty()) {
            // Minimal SPS for specified resolution
            m_sps = {0x67, 0x42, 0xc0, 0x1e, 0xda, 0x01, 0x40, 0x16, 0xe8, 0x06, 0xd0, 0xa1, 0x35};
        }
        if (m_pps.empty()) {
            // Minimal PPS
            m_pps = {0x68, 0xce, 0x38, 0x80};
        }
        
        uint32_t avcCSize = 8 + 6 + 1 + 2 + static_cast<uint32_t>(m_sps.size()) + 
                           1 + 2 + static_cast<uint32_t>(m_pps.size());
        
        writeU32BE(avcCSize);
        writeFourCC("avcC");
        
        m_file.put(1);    // Configuration version
        m_file.put(m_sps.size() > 1 ? m_sps[1] : 0x42); // Profile
        m_file.put(m_sps.size() > 2 ? m_sps[2] : 0xC0); // Profile compatibility
        m_file.put(m_sps.size() > 3 ? m_sps[3] : 0x1E); // Level
        m_file.put(0xFF); // Length size minus one (4 bytes) + reserved bits
        
        // SPS
        m_file.put(0xE1); // Number of SPS (1) + reserved bits
        writeU16BE(static_cast<uint16_t>(m_sps.size()));
        writeBytes(m_sps.data(), m_sps.size());
        
        // PPS
        m_file.put(1);    // Number of PPS
        writeU16BE(static_cast<uint16_t>(m_pps.size()));
        writeBytes(m_pps.data(), m_pps.size());
    }
    
    void writeSttsBox() {
        // All frames have same duration
        writeU32BE(24);
        writeFourCC("stts");
        writeU32BE(0);   // Version and flags
        writeU32BE(1);   // Entry count
        writeU32BE(static_cast<uint32_t>(m_samples.size())); // Sample count
        writeU32BE(m_timeScale / m_fps); // Sample delta
    }
    
    void writeStscBox() {
        // All samples in one chunk
        writeU32BE(28);
        writeFourCC("stsc");
        writeU32BE(0);   // Version and flags
        writeU32BE(1);   // Entry count
        writeU32BE(1);   // First chunk
        writeU32BE(static_cast<uint32_t>(m_samples.size())); // Samples per chunk
        writeU32BE(1);   // Sample description index
    }
    
    void writeStszBox() {
        uint32_t stszSize = 20 + static_cast<uint32_t>(m_samples.size()) * 4;
        
        writeU32BE(stszSize);
        writeFourCC("stsz");
        writeU32BE(0);   // Version and flags
        writeU32BE(0);   // Sample size (0 = variable)
        writeU32BE(static_cast<uint32_t>(m_samples.size())); // Sample count
        
        for (const auto& sample : m_samples) {
            writeU32BE(sample.size);
        }
    }
    
    void writeStcoBox() {
        // Single chunk offset (start of mdat data)
        writeU32BE(20);
        writeFourCC("stco");
        writeU32BE(0);   // Version and flags
        writeU32BE(1);   // Entry count
        writeU32BE(static_cast<uint32_t>(m_mdatHeaderPos) + 8); // Offset to first chunk
    }
    
    void writeStssBox() {
        // Sync sample (keyframe) table
        std::vector<uint32_t> keyframes;
        for (size_t i = 0; i < m_samples.size(); i++) {
            if (m_samples[i].isKeyframe) {
                keyframes.push_back(static_cast<uint32_t>(i + 1)); // 1-indexed
            }
        }
        
        if (keyframes.empty()) return; // No stss box if no keyframes (all are sync)
        
        uint32_t stssSize = 16 + static_cast<uint32_t>(keyframes.size()) * 4;
        
        writeU32BE(stssSize);
        writeFourCC("stss");
        writeU32BE(0);   // Version and flags
        writeU32BE(static_cast<uint32_t>(keyframes.size())); // Entry count
        
        for (uint32_t keyframe : keyframes) {
            writeU32BE(keyframe);
        }
    }
};

#endif // SIMPLE_MP4_MUXER_H

