#include <stdio.h>
#include "fs.h"

int main() {
    superblock sb;
    init_superblock(&sb);

    printf("Total blocks: %u\n", sb.total_blocks);
    printf("Inode start: %u\n", sb.inode_start);
    printf("Data start: %u\n", sb.data_start);

    return 0;
}
