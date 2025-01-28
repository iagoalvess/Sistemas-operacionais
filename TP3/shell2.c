#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2.h"

#define EXT2_S_IFDIR 0x4000
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block - 1) * block_size)
#define EXT2_BLOCK_SIZE 1024
#define NAMELEN 255

unsigned char *disk;

void read_superblock(FILE *img)
{
  struct ext2_super_block super_block;

  fseek(img, EXT2_SUPERBLOCK_OFFSET, SEEK_SET);
  fread(&super_block, sizeof(struct ext2_super_block), 1, img);

  printf("\nSuperblock Information:\n");
  printf("--------------------------------------\n");
  printf("Total Inodes: %u\n", super_block.s_inodes_count);
  printf("Total Blocks: %u\n", super_block.s_blocks_count);
  printf("Free Blocks: %u\n", super_block.s_free_blocks_count);
  printf("Free Inodes: %u\n", super_block.s_free_inodes_count);
  printf("Block Size: %u\n", 1024 << super_block.s_log_block_size);
  printf("Magic: 0x%x\n", super_block.s_magic);
}

void print_dir_contents(int fd, struct ext2_inode *inode, int i, int block_size)
{
  if ((inode->i_mode & 0xF000) != EXT2_S_IFDIR && !S_ISDIR(inode->i_mode))
  {
    return; // Não é um diretório
  }

  struct ext2_dir_entry_2 *entry;
  unsigned int size;
  void *block;

  if ((block = malloc(block_size)) == NULL)
  {
    fprintf(stderr, "Not enough Memory.\n");
    close(fd);
    exit(1);
  }

  lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
  read(fd, block, block_size);

  size = 0;                                 /* keep track of the bytes read */
  entry = (struct ext2_dir_entry_2 *)block; /* first entry in the directory */
  while ((size < inode->i_size) && entry->inode)
  {
    char file_name[NAMELEN + 1];
    memcpy(file_name, entry->name, entry->name_len);
    file_name[entry->name_len] = 0; /* append null char to the file name */
    printf("%10u %s\n", entry->inode, file_name);
    entry = (void *)entry + entry->rec_len; /* move to the next entry */
    size += entry->rec_len;
  }
  free(block);
}

void read_inode(int fd, int inode_no, const struct ext2_group_desc *block_group, struct ext2_inode *inode, int block_size)
{
  lseek(fd, BLOCK_OFFSET(block_group->bg_inode_table) + (inode_no - 1) * sizeof(struct ext2_inode), SEEK_SET);
  read(fd, inode, sizeof(struct ext2_inode));
}

void list_directories(const char *image_path)
{
  int inode_num = 0;
  struct ext2_super_block sb;

  int fd = open(image_path, O_RDWR);
  if (fd < 0)
  {
    perror("Failed to open image");
    return;
  }

  disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  lseek(fd, BASE_OFFSET, SEEK_SET);
  read(fd, &sb, sizeof(struct ext2_super_block));

  unsigned int block_size = 1024 << sb.s_log_block_size;
  unsigned int group = 1 + (sb.s_blocks_count - 1) / sb.s_blocks_per_group;
  unsigned int group_offset;
  unsigned int descr_list_size = group * sizeof(struct ext2_group_desc);

  struct ext2_group_desc gd;
  struct ext2_inode inode;

  for (int l = 0; l < group; l++)
  {
    group_offset = l * sizeof(struct ext2_group_desc);

    lseek(fd, BASE_OFFSET + group_offset + block_size, SEEK_SET);
    read(fd, &gd, sizeof(struct ext2_group_desc));

    int i_bmap_address = block_size * gd.bg_inode_bitmap;

    unsigned char bitmap[block_size];

    lseek(fd, i_bmap_address, SEEK_SET);
    read(fd, &bitmap, block_size);

    for (int i = 2; i < (sb.s_inodes_per_group); i++)
    {
      read_inode(fd, i, &gd, &inode, block_size);

      if ((bitmap[i / 8] >> (i % 8) & 1))
      {
        print_dir_contents(fd, &inode, i, block_size);
      }
    }
  }

  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s disk_image\n", argv[0]);
    return 1;
  }

  FILE *img = fopen(argv[1], "rb");
  if (!img)
  {
    perror("Failed to open image");
    return 1;
  }

  int k = 0;
  while (k == 0)
  {
    printf("\n1. Read Superblock (sb)\n");
    printf("2. List Directories (ls)\n");
    printf("5. Exit\n");
    printf("\n> ");
    int option;

    if (scanf("%d", &option) != 1)
    {
      printf("Invalid input. Please enter a number.\n");
      while (getchar() != '\n')
        ;
      continue;
    }

    switch (option)
    {
    case 1:
      read_superblock(img);
      break;
    case 2:
      list_directories(argv[1]);
      break;
    case 5:
      printf("Bye!\n");
      k = 1;
      break;
    default:
      printf("Invalid Option!\n");
    }
  }

  fclose(img);
  return 0;
}