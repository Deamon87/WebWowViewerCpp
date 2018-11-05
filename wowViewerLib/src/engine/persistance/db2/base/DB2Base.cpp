//
// Created by deamon on 02.04.18.
//

#include <cstdint>
#include <algorithm>
#include <iostream>
#include "DB2Base.h"
#include "../../../../include/wowScene.h"

void DB2Base::process(const std::vector<unsigned char> &db2File, const std::string &fileName) {
    this->db2File = db2File;
    fileData = &this->db2File[0];

    currentOffset = 0;
    bytesRead = 0;

    readValue(header);
    if (header->magic != 'WDC2') return;

    readValues(section_headers, header->section_count);
    readValues(fields, header->total_field_count);
    fieldInfoLength = header->field_storage_info_size / sizeof(field_storage_info);
    readValues(field_info, fieldInfoLength);
    readValues(pallet_data, header->pallet_data_size);
    readValues(common_data, header->common_data_size);

    //Read section
    for (int i = 0; i < header->section_count; i++) {
        auto &itemSectionHeader = section_headers[i];
        sections.resize(sections.size()+1);
        section &section = sections[sections.size()-1];

        if ((header->flags & 1) == 0) {
            // Normal records

            for (int j = 0; j < section_headers->record_count; j++) {
                record_data recordData;
                readValues(recordData.data, header->record_size);

                section.records.push_back(recordData);
            }

            readValues(section.string_data, itemSectionHeader.string_table_size);
        } else {
            // Offset map records -- these records have null-terminated strings inlined, and
            // since they are variable-length, they are pointed to by an array of 6-byte
            // offset+size pairs.
            readValues(section.variable_record_data, itemSectionHeader.offset_map_offset - itemSectionHeader.file_offset);
            readValues(section.offset_map, header->max_id - header->min_id + 1);
        }

        readValues(section.id_list, itemSectionHeader.id_list_size / 4);
        if (itemSectionHeader.copy_table_size > 0) {
            readValues(section.copy_table, itemSectionHeader.copy_table_size / sizeof(copy_table_entry));
        }
        if (itemSectionHeader.relationship_data_size > 0) {
            // In some tables, this relationship mapping replaced columns that were used
            // only as a lookup, such as the SpellID in SpellX* tables.
            readValue(section.relationship_map.num_entries);
            readValue(section.relationship_map.min_id);
            readValue(section.relationship_map.max_id);
            readValues(section.relationship_map.entries, section.relationship_map.num_entries);
        }
    }

    m_loaded = true;
}

//Returns index
int DB2Base::readRecord(int id, bool useRelationMappin, int minFieldNum, int fieldsToRead, std::function<void(int fieldNum, int stringOffset, char *data, size_t length)> callback) {
    //1. Get id offset
    int idDiff = id - header->min_id;

    //2. Find index
    int pos;
    auto &sectionDef = sections[0];
    if (useRelationMappin) {
        //2.1 Find index using relation mapping
        relationship_entry entry;
        entry.foreign_id = id;
        entry.record_index = 0;
        relationship_entry *start = sections[0].relationship_map.entries + 3; //hack
        relationship_entry *end = sections[0].relationship_map.entries + (sections[0].relationship_map.num_entries - 1);
        relationship_entry *indx = std::find_if(start, end, [&id](relationship_entry &entry) -> bool {
            return entry.foreign_id == id;
        });
        pos = indx - sections[0].relationship_map.entries;
        if (indx == end || indx->foreign_id != id)
            return false;
    } else {
        //2.2 Find index in id_list

        uint32_t *end = sections[0].id_list + (section_headers->id_list_size / 4);
        uint32_t *indx = std::lower_bound(sections[0].id_list, end, id);
        pos = indx - sections[0].id_list;
        if (indx == end || *indx != id)
            return -1;
    }
    //3. Read the record
    readRecordByIndex(pos, minFieldNum, fieldsToRead, callback);

    return pos;
}

std::string DB2Base::readString(int index) {
    return std::string(&sections[0].string_data[index]);
}
bool DB2Base::readRecordByIndex(int index, int minFieldNum, int fieldsToRead,
                                std::function<void(int fieldNum, int stringOffset, char *data, size_t length)> callback) {
    auto &sectionDef = sections[0];
    int numOfFieldToRead = fieldsToRead >=0 ? fieldsToRead : header->field_count;

    if ((header->flags & 1) == 0) {
        for (int i = minFieldNum; i < numOfFieldToRead; i++) {
            auto &fieldInfo = field_info[i];
            if (fieldInfo.storage_type == field_compression_none) {
                char * recordPointer = sectionDef.records[index].data;
                int byteOffset = fieldInfo.field_offset_bits >> 3;
                int bytesToRead = fieldInfo.field_size_bits >> 3;

                char * fieldDataPointer = &recordPointer[byteOffset];

                callback(i, &recordPointer[byteOffset] - sectionDef.string_data, fieldDataPointer,  bytesToRead);
            } else if (fieldInfo.storage_type == field_compression_bitpacked) {
                char * recordPointer = sectionDef.records[index].data;
                char buffer[128];

                int byteOffset = fieldInfo.additional_data_size * index + (fieldInfo.field_offset_bits) >> 3;
                int bitOffset = fieldInfo.field_size_bits & 8;
                int bitesToRead = fieldInfo.field_size_bits;

                //Zero the buffer
                for (int j = 0; j < bitesToRead/8; j++) buffer[j] = 0;

                //Read bites
                int firstBitesToExtract = bitesToRead+bitOffset > 8 ? 8-bitOffset : bitesToRead ;
                unsigned char startMask = (unsigned char) (((1 << firstBitesToExtract) - 1)) << bitOffset;
                unsigned char endMask = (unsigned char) (0xFF ^ startMask);

                buffer[0] = recordPointer[byteOffset] & (startMask) >> bitOffset;
                int totalBytesToRead = (bitesToRead + 7) >> 3;
                for (int j = 0; j < totalBytesToRead-1; j++) {
                    buffer[j] = buffer[j] | (recordPointer[byteOffset + j]     & (endMask) << bitOffset);
                    buffer[j + 1] =         (recordPointer[byteOffset + j + 1] & (startMask) >> bitOffset);
                }
                int endByteOffset = totalBytesToRead-1;
                buffer[endByteOffset] = buffer[endByteOffset] | (recordPointer[byteOffset] & (endMask) << bitOffset);

                callback(i, 0, &buffer[0], bitesToRead >> 3);
            }
        }
    }

    return false;
}
