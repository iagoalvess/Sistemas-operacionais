#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "ext2.h"
#define EXT2_S_IFDIR 0x4000  // Diretório
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block - 1) * block_size)
#define EXT2_BLOCK_SIZE  1024
#define BASE_OFFSET 1024
#define NAMELEN     255


unsigned char *disk;
void print_dir_contents(int fd,struct ext2_inode *inode, int i, int block_size){

        if ((inode->i_mode & 0xF000) != EXT2_S_IFDIR && !S_ISDIR(inode->i_mode)) {
            return; // Não é um diretório
        }

        struct ext2_dir_entry_2 *entry;
        unsigned int size;
        void *block;
    
        char *tmpprefix;      


        if ((block = malloc(block_size)) == NULL) {
        fprintf(stderr, "Not enough Memory.\n");
        close(fd);
        exit(1);
        }

        lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
        read(fd, block, block_size);

        size = 0;                                            /* keep track of the bytes read */
        entry = (struct ext2_dir_entry_2 *) block;           /* first entry in the directory */
        while((size < inode->i_size) && entry->inode) {
            char file_name[NAMELEN+1];
            memcpy(file_name, entry->name, entry->name_len);
            file_name[entry->name_len] = 0;              /* append null char to the file name */
            printf("%10u %s\n", entry->inode, file_name);
            entry = (void*) entry + entry->rec_len;      /* move to the next entry */
            size += entry->rec_len;
        }
}


void read_inode(int fd, int inode_no, const struct ext2_group_desc *block_group, struct ext2_inode *inode, int block_size)
{
  lseek(fd, BLOCK_OFFSET(block_group->bg_inode_table)+(inode_no-1)*sizeof(struct ext2_inode),
    SEEK_SET);
  read(fd, inode, sizeof(struct ext2_inode));

}

int main(){
   
   
	int inode_num = 0;
    struct ext2_super_block sb;
   
    int fd = open("fs-0x00dcc605-ext2.img", O_RDWR);
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lseek(fd, BASE_OFFSET, SEEK_SET);
    read(fd, &sb, sizeof(struct ext2_super_block));
   
    unsigned int block_size = 1024 << sb.s_log_block_size;
    unsigned int group = 1 + (sb.s_blocks_count-1) / sb.s_blocks_per_group;
    unsigned int group_offset;
    unsigned int descr_list_size = group * sizeof(struct ext2_group_desc);
    
    struct ext2_group_desc gd;
    //struct ext2_inode inode; 
    int l;
    printf("numero de grupos %d\n\n", group);
    
    for(l=0; l<group; l++){
        printf("grupo %d-----------------------", l);
		inode_num = 0;
		//printf("\n\n------------------ GROUP %d -------------------\n\n", l);

        group_offset =  l *sizeof(struct ext2_group_desc);

        lseek(fd, BASE_OFFSET + group_offset + block_size, SEEK_SET);
        read(fd, &gd, sizeof(struct ext2_group_desc));

        int i_bmap_address = block_size * gd.bg_inode_bitmap;

        unsigned char bitmap[block_size];

        lseek(fd, i_bmap_address, SEEK_SET);
        read(fd, &bitmap, block_size); 

       
	    int n = 0;
        struct ext2_inode inode; 

        for(int i = 2; i<(sb.s_inodes_per_group); i++){
			
            read_inode(fd, i, &gd, &inode, block_size);   

			//if(inode.i_mode > 0) printf("inode %d imode %d bitmap %d\n", inode_num+1, inode.i_mode, (bitmap[i/8] >> (i%8) & 1));
            
			if((bitmap[i/8] >> (i%8) & 1)){
                print_dir_contents(fd, &inode, i, block_size);
                 
			}
			        
        }
		
    }
    return 0;
}

