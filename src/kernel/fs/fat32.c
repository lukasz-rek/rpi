#include "fs/fat32.h"
#include "printf.h"
#include "arm/util.h"
// #include "string.h"


static unsigned long fat_sector;
static bpb_entry_t bpb;
// Some stuff we will use
static int fat_size;
static unsigned long first_data_sector;

#define SECTOR_FROM_CLUSTER(CLUSTER, SECTORS) ((CLUSTER - 2) * SECTORS + first_data_sector + fat_sector)  


/**
 * @brief Looks at BPB and gets root directory
 */
int get_root_directory() {
    printf("Reading sector %d\n", fat_sector);
    emmc_read_block(fat_sector, (uint8_t*) &bpb);
    if(bpb.signature != 0x29 || bpb.bootable_signature != 0xAA55) {
        printf("Mismatch in BPB signatures\nFirst one %x and %x\n", bpb.signature, bpb.bootable_signature);
    } else {
        printf("BPB located succesfully!\n");
        printf("Bytes per sector: %d\n", bpb.bytes_per_sector);
        printf("Sectors per cluster: %d\n", bpb.sectors_per_cluster);
        printf("Root cluster addr: %d\n", bpb.root_cluster_number);
        fat_size = bpb.sectors_per_fat;
        first_data_sector = bpb.reserved_sectors + (bpb.no_of_fat_tables * fat_size);
        // root_cluster = SECTOR_FROM_CLUSTER(bpb.root_cluster_number, bpb.sectors_per_cluster);
        printf("Sector of root cluster: %d\n", SECTOR_FROM_CLUSTER(bpb.root_cluster_number, bpb.sectors_per_cluster));    

        printf("Root cluster by path: %d\n", get_cluster_by_path("/", 1).cluster_number);
        printf("/config.txt by path: %d\n", get_cluster_by_path("/config.txt", 11).cluster_number);

        long secto = get_cluster_by_path("/elem/elo.txt", 13).cluster_number;
        printf("/elem/elo.txt by path: %d\n", secto);      
        
        // char read_data[512];
        // emmc_read_block(SECTOR_FROM_CLUSTER(secto, bpb.sectors_per_cluster), (uint8_t*) read_data);
        // printf("Attempting read: %s\n", read_data);

        return 0;
    }
    return -1;
}

virt_node_t get_cluster_by_path(char* name, uint16_t path_len) {
    // Base cases
    virt_node_t result;

    if (name[path_len] == '\0')
        path_len--;
    if (name[path_len] == '/' && path_len > 1 )
        path_len--;
    if (name[path_len] == '/') {
        printf("Got root\n");
        result.cluster_number = bpb.root_cluster_number;
        return result;
    }

    // Now try to parse path
    char cur_name[MAX_FILENAME_LEN] = {'\0'};
    int name_start = path_len;
    while(name[name_start] != '/') {
        name_start--;
    }
    memcpy((unsigned long) cur_name,(unsigned long) &name[name_start + 1], path_len - name_start);
    // Now retrieve the files of parent and check if we can match stuff
    virt_node_t found_files[64];
    int files_num = 0;
    long parent_cluster = get_cluster_by_path(name, name_start).cluster_number;

    parse_cluster(parent_cluster, found_files, &files_num);
    for(int i = 0; i < files_num; i++) {
        // printf("Comparing with %s\n", found_files[i].name);
        if (!str_compare(found_files[i].name, cur_name, path_len - name_start)) {
            // printf("Found match at %d\n" found_files)
            return found_files[i];
        }
    }
    // We haven't found a match
    result.cluster_number = 0; // Nothing can have 0 as cluster num
    return result; 

}


long get_next_cluster(unsigned long cluster_number) {
    // TODO: caching would be damn sweet here
    // Read the cluster chain info
    uint8_t fat_table[512];
    uint32_t fat_offset = cluster_number * 4;
    // We have to include the overall beginning of fat sector, via the extra add at the end
    uint32_t fat_table_sector = bpb.reserved_sectors + (fat_offset / 512) + fat_sector;
    uint32_t entry_offset = fat_offset % 512;
    // printf("Fat table sector num: %d\n", fat_table_sector);
    emmc_read_block(fat_table_sector, fat_table);
    uint32_t table_value = *(uint32_t*)&fat_table[entry_offset] & 0x0FFFFFFF;

    if (table_value == 0x0FFFFFF7){
        return -1;
    }
    return table_value;
}


int16_t parse_cluster(unsigned long cluster_number, virt_node_t retrieved_paths[64], int* idx) {
    // printf("Parsing cluster no: %d\n", cluster_number);
    uint64_t sector_num = SECTOR_FROM_CLUSTER(cluster_number, bpb.sectors_per_cluster);
    // printf("Sectors %d - %d\n", sector_num, sector_num + 3);
    
    uint32_t table_value = get_next_cluster(cluster_number);
    

    if (table_value == -1){
        printf("Bad cluster\n");
        return -1;
    }

    uint8_t data[512];
    char filename_buffer[MAX_FILENAME_LEN];
    filename_buffer[MAX_FILENAME_LEN - 1] = '\0';
    int filename_idx = MAX_FILENAME_LEN - 2;
    emmc_read_block(sector_num++, data);
    for(int count = 0; count < 4; count++) {

        for(int i = 0;i < 512; i+= 32) {
            uint8_t* entry = &data[i];
            if(entry[0] == 0x00) {
                printf("No more entries\n");
                count = 5; // So we go to the end immediately
                break;
            } else if(entry[0] == 0xE5) {
                printf("Entry unused, advance\n");
                continue;
            } else if(entry[11] == 0x0F) {
                // It is long file name
                long_name_t* name = (long_name_t*) entry;
                // We do be readin backwards
                // Read last chars
                for(int i = 1; i >= 0; i--) {
                    uint16_t ch = name->last[i];
                    if(ch == 0x0000 || ch == 0xFFFF) ch = '\0';
                    filename_buffer[filename_idx--] = (char)ch;
                }
                // Read next 6 chars
                for(int i = 5; i >= 0; i--) {
                    uint16_t ch = name->next[i];
                    if(ch == 0x0000 || ch == 0xFFFF) ch = '\0';
                    filename_buffer[filename_idx--] = (char)ch;
                }
                                    
                for(int i = 4; i >= 0; i--) {
                    uint16_t ch = name->first[i];
                    if(ch == 0x0000 || ch == 0xFFFF) ch = '\0';
                    filename_buffer[filename_idx--] = (char)ch;
                }
                continue;
            } else if (filename_idx != 62){
                //It is 8.3 dir
                // printf("Filename: %s\n", &filename_buffer[filename_idx + 1]);
                // printf("Parsed file no: %d\n", *idx);
                directory_t* dir = (directory_t*) entry;
                retrieved_paths[(*idx)].cluster_number = (dir->entrys_first_cluster_no_high_16 << 16) + dir->entrys_first_cluster_no_low_16;
                retrieved_paths[(*idx)].size = dir->size_bytes;
                retrieved_paths[(*idx)].is_folder = 0x10 & dir->attr;

                memcpy((unsigned long) &retrieved_paths[(*idx)++].name,(unsigned long) &filename_buffer[filename_idx + 1], MAX_FILENAME_LEN - 1 - filename_idx);
                filename_idx = MAX_FILENAME_LEN - 2;
                memzero((unsigned long) filename_buffer, MAX_FILENAME_LEN);
                filename_buffer[MAX_FILENAME_LEN - 1] = '\0';
                // printf("%s\n", dir->name);
            }
        }
        emmc_read_block(sector_num++, data);
    }
    if ( table_value >= 0x0FFFFFF8) {
        // printf("Read last cluster\n");
        return 0;
    } else {
        // printf("Reading next cluster in chain\n");
        return parse_cluster(table_value, retrieved_paths, idx);
    }
}


/**
 * @brief Finds our fat32 partition from MBR
 * @return -1 on failure
 * @return address on success
 */
long get_fat32_sector() {
    // We will read first sector and check first and 2nd mbr entry
    // We will likely already see that first one is 

    uint8_t data[512];
    emmc_read_block(0, data);

    mbr_entry_t* entries = (mbr_entry_t*)(data + 0x1BE);
    printf("Found following partitions: \n");
    print_partition_info(&entries[0]);
    print_partition_info(&entries[1]);

    for (int i = 0; i < 2; i++) {
        if (entries[i].partition_type == FAT32_FS) {
            fat_sector = entries[i].lba_partition_start;
            return entries[i].lba_partition_start;
        }
    }

    return -1;
}

void print_partition_info(mbr_entry_t* entry) {
    printf("Partition Info:\n");
    printf("  Bootable:       %s\n", entry->bootable == 0x80 ? "Yes" : "No");
    const char* partition_type;
    switch (entry->partition_type) {
        case FAT32_FS:
            partition_type = "FAT32";
            break;
        case EXT4_FS:
            partition_type = "EXT4";
            break;
        default:
            partition_type = "UNKWN";
            break;
    }
    printf("  Partition Type: 0x%02X (%s)\n", entry->partition_type, partition_type);
    printf("  Start LBA:      %u\n", entry->lba_partition_start);
    printf("  Sectors:        %u\n", entry->no_sectors);
    printf("  Size (MB):      %u\n", (entry->no_sectors / 2048)); // Assuming 512-byte sectors
}