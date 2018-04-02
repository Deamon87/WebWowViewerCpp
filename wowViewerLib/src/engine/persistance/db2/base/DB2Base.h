//
// Created by deamon on 02.04.18.
//

#ifndef WEBWOWVIEWERCPP_DB2BASE_H
#define WEBWOWVIEWERCPP_DB2BASE_H

#include <vector>
#include <functional>

struct wdc2_db2_header
{
    uint32_t magic;                  // 'WDC2'
    uint32_t record_count;           // this is for all sections combined now
    uint32_t field_count;
    uint32_t record_size;
    uint32_t string_table_size;      // this is for all sections combined now
    uint32_t table_hash;             // hash of the table name
    uint32_t layout_hash;            // this is a hash field that changes only when the structure of the data changes
    uint32_t min_id;
    uint32_t max_id;
    uint32_t locale;                 // as seen in TextWowEnum
    uint16_t flags;                  // possible values are listed in Known Flag Meanings
    uint16_t id_index;               // this is the index of the field containing ID values; this is ignored if flags & 0x04 != 0
    uint32_t total_field_count;      // from WDC1 onwards, this value seems to always be the same as the 'field_count' value
    uint32_t bitpacked_data_offset;  // relative position in record where bitpacked data begins; not important for parsing the file
    uint32_t lookup_column_count;
    uint32_t field_storage_info_size;
    uint32_t common_data_size;
    uint32_t pallet_data_size;
    uint32_t section_count;          // new to WDC2, this is number of sections of data (records + copy table + id list + relationship map = a section)
};

struct wdc2_section_header
{
    uint32_t wdc2_unk_header1;       // always 0 in Battle (8.0.1.26231) and unnamed in client binary
    uint32_t wdc2_unk_header2;       // always 0 in Battle (8.0.1.26231) and unnamed in client binary
    uint32_t file_offset;            // absolute position to the beginning of the section
    uint32_t record_count;           // 'record_count' for the section
    uint32_t string_table_size;      // 'string_table_size' for the section
    uint32_t copy_table_size;
    uint32_t offset_map_offset;      // Offset to array of struct {uint32_t offset; uint16_t size;}[max_id - min_id + 1];
    uint32_t id_list_size;           // List of ids present in the DB file
    uint32_t relationship_data_size;
};

struct field_structure
{
    uint16_t size;
    uint16_t offset;
};

enum field_compression
{
    // None -- the field is a 8-, 16-, 32-, or 64-bit integer in the record data
            field_compression_none,
    // Bitpacked -- the field is a bitpacked integer in the record data.  It
    // is field_size_bits long and starts at field_offset_bits.
    // A bitpacked value occupies
    //   (field_size_bits + (field_offset_bits & 7) + 7) / 8
    // bytes starting at byte
    //   field_offset_bits / 8
    // in the record data.  These bytes should be read as a little-endian value,
    // then the value is shifted to the right by (field_offset_bits & 7) and
    // masked with ((1ull << field_size_bits) - 1).
            field_compression_bitpacked,
    // Common data -- the field is assumed to be a default value, and exceptions
    // from that default value are stored in the corresponding section in
    // common_data as pairs of { uint32_t record_id; uint32_t value; }.
            field_compression_common_data,
    // Bitpacked indexed -- the field has a bitpacked index in the record data.
    // This index is used as an index into the corresponding section in
    // pallet_data.  The pallet_data section is an array of uint32_t, so the index
    // should be multiplied by 4 to obtain a byte offset.
            field_compression_bitpacked_indexed,
    // Bitpacked indexed array -- the field has a bitpacked index in the record
    // data.  This index is used as an index into the corresponding section in
    // pallet_data.  The pallet_data section is an array of uint32_t[array_count],
    //
            field_compression_bitpacked_indexed_array,
};

struct field_storage_info
{
    uint16_t          field_offset_bits;
    uint16_t          field_size_bits; // very important for reading bitpacked fields; size is the sum of all array pieces in bits - for example, uint32[3] will appear here as '96'
    // additional_data_size is the size in bytes of the corresponding section in
    // common_data or pallet_data.  These sections are in the same order as the
    // field_info, so to find the offset, add up the additional_data_size of any
    // previous fields which are stored in the same block (common_data or
    // pallet_data).
    uint32_t          additional_data_size;
    field_compression storage_type;
    union {
        struct {
            uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
            uint32_t bitpacking_size_bits; // not useful for most purposes
            uint32_t flags; // known values - 0x01: sign-extend (signed)
        } field_compression_bitpacked;
        struct {
            uint32_t default_value;
            uint32_t unk_or_unused2;
            uint32_t unk_or_unused3;
        } field_compression_common_data;
        struct {
            uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
            uint32_t bitpacking_size_bits; // not useful for most purposes
            uint32_t unk_or_unused3;
        } field_compression_bitpacked_indexed;
        struct {
            uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
            uint32_t bitpacking_size_bits; // not useful for most purposes
            uint32_t array_count;
        } field_compression_bitpacked_indexed_array;
        struct {
            uint32_t unk_or_unused1;
            uint32_t unk_or_unused2;
            uint32_t unk_or_unused3;
        } _default;
    };
};

//Section related structs
struct record_data
{
    char *data;
};
struct offset_map_entry
{
    uint32_t offset;
    uint16_t size;
};

struct copy_table_entry
{
    uint32_t id_of_new_row;
    uint32_t id_of_copied_row;
};

struct relationship_entry
{
    // This is the id of the foreign key for the record, e.g. SpellID in
    // SpellX* tables.
    uint32_t foreign_id;
    // This is the index of the record in record_data.  Note that this is
    // *not* the record's own ID.
    uint32_t record_index;
};

struct relationship_mapping
{
    uint32_t            num_entries;
    uint32_t            min_id;
    uint32_t            max_id;
    relationship_entry*  entries = nullptr;
};

struct section
{
    std::vector <record_data> records;
    char *string_data = nullptr;

    char *variable_record_data = nullptr;
    offset_map_entry *offset_map = nullptr;

    uint32_t *id_list = nullptr;
    copy_table_entry *copy_table = nullptr;

    relationship_mapping relationship_map;
};

class DB2Base {
public:
    void process(std::vector<unsigned char> &db2File);
    bool getIsLoaded() { return m_loaded; };
    bool readRecord(int id, std::function<void(int fieldNum, char *data, size_t length)>callback);
private:
    bool m_loaded = false;
    std::vector<uint8_t> db2File;
    unsigned char *fileData;
    int currentOffset;
    int bytesRead;
    template<typename T> inline void readValue(T &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = *(T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += sizeof(T);
        bytesRead += sizeof(T);
    }
    template<typename T> inline void readValue(T* &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = (T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += sizeof(T);
        bytesRead += sizeof(T);
    }
    template<typename T> inline void readValues(T* &value, int count) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = (T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += count*sizeof(T);
        bytesRead += count*sizeof(T);
    }

//fields
    wdc2_db2_header *header;
    wdc2_section_header *section_headers;
    field_structure *fields;
    field_storage_info *field_info;
    int fieldInfoLength = -1;
    char *pallet_data;
    char *common_data;
    std::vector<section> sections;
};


#endif //WEBWOWVIEWERCPP_DB2BASE_H
