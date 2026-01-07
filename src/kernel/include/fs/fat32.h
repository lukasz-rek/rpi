#include "stdint.h"
#include "drivers/emmc.h"
#include "fat32_files.h"


#define FAT32_FS    0x0C
#define EXT4_FS     0x83



typedef struct mbr_entry {
    uint8_t bootable;
    uint8_t chs[3];
    uint8_t partition_type;
    uint8_t chs_last_partition_sector[3];
    uint32_t lba_partition_start;
    uint32_t no_sectors;
} __attribute__((packed)) mbr_entry_t;

typedef struct bpb_entry {
    uint8_t     jump_code[3];
    char        oem[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reserved_sectors;
    uint8_t     no_of_fat_tables;
    uint16_t    no_of_root_dir_entries;
    uint16_t    total_sectors;
    uint8_t     media_descriptor;
    uint16_t    sectors_per_fat_unused; //FAT12/16 only
    uint16_t    sectors_per_track;
    uint16_t    no_of_sides;
    uint32_t    no_hidden_sectors;
    uint32_t    no_large_sectors;
    // Extended boot record, fat32 specific
    uint32_t    sectors_per_fat;
    uint16_t    flags;
    uint16_t    version;
    uint32_t    root_cluster_number;
    uint16_t    fsinfo_number;
    uint16_t    backup_sector_number;
    uint8_t     reserved[12];
    uint8_t     drive_number;
    uint8_t     windows_flags;
    uint8_t     signature; // Must be 0x28 or 0x29
    uint32_t    volume_id;
    char        volume_label[11];
    uint64_t    system_identifier;
    uint8_t     boot_code[420];
    uint16_t    bootable_signature;
} __attribute__((packed)) bpb_entry_t;


long get_fat32_sector();
int get_root_directory();

long get_next_cluster(unsigned long cluster_number);
// Needs absolute path
virt_node_t get_cluster_by_path(char* name, uint16_t path_len);

/**
 * @brief Reads from specified file
 * @param buffer always of size 512
 * @return 0 on success, otherwise failure
 */
int16_t read_from_fat32_file(virt_node_t *file, uint8_t* buffer, uint32_t offset);

// Bro I would so love string support right now :'''(, who even made this OS?
// Returns how many entries were found, -1 on failure 
int16_t parse_cluster(unsigned long cluster_number, virt_node_t retrieved_paths[MAX_FILENAME_LEN], int *idx);

void print_partition_info(mbr_entry_t* entry);