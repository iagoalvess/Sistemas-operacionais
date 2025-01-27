#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/stat.h>
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

void print_dir_contents(int fd,struct ext2_inode *inode, int block_size, struct ext2_group_desc block_group, struct ext2_super_block super_block){
           
         if ((inode->i_mode & 0xF000) != EXT2_S_IFDIR) {
            return;
         }
        
        struct ext2_dir_entry_2 *entry;
        unsigned int size;
        void *block;

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




int main(){
   
   
	struct ext2_super_block super_block;
  struct ext2_group_desc block_group;
  struct ext2_inode inode;
   
   
int fd = open("fs-0x00dcc605-ext2-10240.img", O_RDWR);
   
  if(lseek(fd, BASE_OFFSET, SEEK_SET) < 0) {
    fprintf(stderr,"unable to seek\n");
    exit(0);
  }

  /* read super-block */

  lseek(fd, BASE_OFFSET, SEEK_SET);
  read(fd, &super_block, sizeof(super_block));

  int block_size = 1024 << super_block.s_log_block_size;

  /* read group descriptor */

  lseek(fd, BASE_OFFSET + block_size, SEEK_SET);
  read(fd, &block_group, sizeof(block_group));

 
   
  int index = (2) % super_block.s_inodes_per_group;

  lseek(fd, BLOCK_OFFSET(block_group.bg_inode_table)+index*sizeof(struct ext2_inode),
    SEEK_SET);
  read(fd, &inode, sizeof(struct ext2_inode));

 print_dir_contents(fd, &inode, block_size, block_group, super_block);      
			
    return 0;
}

