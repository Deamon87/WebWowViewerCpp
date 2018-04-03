//
// Created by deamon on 02.04.18.
//

#include <cstdint>
#include <algorithm>
#include <iostream>
#include "DB2Base.h"

void DB2Base::process(std::vector<unsigned char> &db2File) {
    this->db2File = db2File;
    fileData = &this->db2File[0];

    currentOffset = 0;
    bytesRead = 0;

    readValue(header);
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

bool DB2Base::readRecord(int id, int minFieldNum, int fieldsToRead, std::function<void(int fieldNum, char *data, size_t length)> callback) {
    int numOfFieldToRead = fieldsToRead >=0 ? fieldsToRead : header->field_count;

    //1. Get id offset
    int idDiff = id - header->min_id;

    //2. Find index in id_list
    auto &sectionDef = sections[0];
    uint32_t *end = sections[0].id_list+(section_headers->id_list_size/4);
    uint32_t *indx = std::lower_bound(sections[0].id_list, end, id);
    int pos = indx-sections[0].id_list;
    if(indx == end || *indx != id)
        return false;

    //3. Read the record
    int index = pos;
    if ((header->flags & 1) == 0) {
        char * recordPointer = sectionDef.records[index].data;

        for (int i = minFieldNum; i < numOfFieldToRead; i++) {
            auto &fieldInfo = field_info[i];
            if (fieldInfo.storage_type == field_compression_none) {
                int byteOffset = fieldInfo.field_offset_bits / 8;
                int bytesToRead = fieldInfo.field_size_bits / 8;

                char * fieldDataPointer = &recordPointer[byteOffset];

                callback(i, fieldDataPointer, bytesToRead);
            }
        }
    }

    return true;
}
