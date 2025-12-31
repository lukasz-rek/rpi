#include "stdint.h"
#define MAX_FILENAME_LEN 64



typedef struct directory {
    char        name[11];
    uint8_t     attr;
    uint8_t     win_reserved;
    uint8_t     creation_time_100ths_of_second;
    uint16_t    creation_time;
    uint16_t    creation_date;
    uint16_t    last_access_date;
    uint16_t    entrys_first_cluster_no_high_16;
    uint16_t    last_modify_time;
    uint16_t    last_modify_date;
    uint16_t    entrys_first_cluster_no_low_16;
    uint64_t    size_bytes;
} __attribute__((packed)) directory_t;

typedef struct long_name {
    uint8_t     order;
    uint16_t    first[5];
    uint8_t     attr; // Equals 0x0F
    uint8_t     type;
    uint8_t     checksum;
    uint16_t    next[6];
    uint16_t    zeroed;
    uint16_t    last[2];
} __attribute__((packed)) long_name_t;

// Can be either folder or file
typedef struct virt_node {
    char        name[MAX_FILENAME_LEN];
    uint32_t    cluster_number;
    uint8_t     is_folder;
    uint64_t    size;
} virt_node_t;
