#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "ext2.h"

// Estrutura global para armazenar o estado atual
struct fs_state
{
  int fd;
  struct ext2_super_block sb;
  int block_size;
  unsigned int cwd_inode; // Inode do diretório atual
  char cwd_path[MAX_PATH_LEN];
} g_state;

struct ext2_inode read_inode(unsigned int inode_num);
int find_entry_in_dir(struct ext2_inode *dir_inode, const char *name, unsigned int *found_inode);
int resolve_path(const char *path, unsigned int *result_inode);
void ext2_find(const char *path, const char *prefix);
void _ext2_find_recursive(struct ext2_inode *inode, const char *path, const char *prefix);
void normalize_path(char *path);

// Função auxiliar para normalização de caminho
void normalize_path(char *path)
{
  char copy[MAX_PATH_LEN];
  strncpy(copy, path, MAX_PATH_LEN - 1);
  copy[MAX_PATH_LEN - 1] = '\0';

  char *components[MAX_PATH_LEN];
  int depth = 0;
  char *saveptr;
  char *token = strtok_r(copy, "/", &saveptr);

  while (token != NULL)
  {
    if (strcmp(token, "..") == 0)
    {
      if (depth > 0)
        depth--;
    }
    else if (strcmp(token, ".") != 0 && strlen(token) > 0)
    {
      if (depth < MAX_PATH_LEN)
      {
        components[depth++] = token;
      }
    }
    token = strtok_r(NULL, "/", &saveptr);
  }

  path[0] = '\0';
  strcat(path, "/");

  for (int i = 0; i < depth; i++)
  {
    if (i > 0)
      strcat(path, "/");
    strcat(path, components[i]);
  }

  if (depth == 0)
    strcat(path, "/");
}

struct ext2_inode read_inode(unsigned int inode_num)
{
  if (inode_num == 0)
  {
    fprintf(stderr, "read_inode: inode 0 inválido\n");
    exit(1);
  }

  unsigned int group = (inode_num - 1) / g_state.sb.s_inodes_per_group;
  unsigned int index = (inode_num - 1) % g_state.sb.s_inodes_per_group;

  off_t group_desc_offset = BASE_OFFSET + g_state.block_size + (group * sizeof(struct ext2_group_desc));
  struct ext2_group_desc group_desc;

  lseek(g_state.fd, group_desc_offset, SEEK_SET);
  if (read(g_state.fd, &group_desc, sizeof(group_desc)) != sizeof(group_desc))
  {
    fprintf(stderr, "Erro ao ler descritor de grupo: %s\n", strerror(errno));
    exit(1);
  }

  off_t inode_offset = BLOCK_OFFSET(group_desc.bg_inode_table) + (index * g_state.sb.s_inode_size);

  struct ext2_inode inode;
  lseek(g_state.fd, inode_offset, SEEK_SET);
  if (read(g_state.fd, &inode, sizeof(inode)) != sizeof(inode))
  {
    fprintf(stderr, "Erro ao ler inode %u: %s\n", inode_num, strerror(errno));
    exit(1);
  }

  return inode;
}

int find_entry_in_dir(struct ext2_inode *dir_inode, const char *name, unsigned int *found_inode)
{
  void *block = malloc(g_state.block_size);
  int ret = -ENOENT;

  for (int block_idx = 0; block_idx < EXT2_NDIR_BLOCKS; block_idx++)
  {
    if (dir_inode->i_block[block_idx] == 0)
      break;

    lseek(g_state.fd, BLOCK_OFFSET(dir_inode->i_block[block_idx]), SEEK_SET);
    if (read(g_state.fd, block, g_state.block_size) != g_state.block_size)
    {
      fprintf(stderr, "Erro ao ler bloco %u: %s\n", dir_inode->i_block[block_idx], strerror(errno));
      continue;
    }

    struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)block;
    unsigned int pos = 0;

    while (pos < g_state.block_size && entry->rec_len > 0)
    {
      if (entry->inode == 0)
      {
        pos += entry->rec_len;
        entry = (void *)entry + entry->rec_len;
        continue;
      }

      char entry_name[NAMELEN + 1];
      memcpy(entry_name, entry->name, entry->name_len);
      entry_name[entry->name_len] = '\0';

      if (strcmp(entry_name, name) == 0)
      {
        *found_inode = entry->inode;
        ret = 0;
        goto out;
      }

      pos += entry->rec_len;
      entry = (void *)entry + entry->rec_len;
    }
  }

out:
  free(block);
  return ret;
}

int resolve_path(const char *path, unsigned int *result_inode)
{
  char *path_copy = strdup(path);
  char *component;
  unsigned int current_inode;
  char *normalized_path[MAX_PATH_LEN];
  int depth = 0;

  if (path[0] == '/')
  {
    current_inode = 2;
  }
  else
  {
    current_inode = g_state.cwd_inode;
  }

  component = strtok(path_copy, "/");
  while (component != NULL)
  {
    if (strlen(component) == 0)
    {
      component = strtok(NULL, "/");
      continue;
    }

    if (strcmp(component, ".") == 0)
    {
      component = strtok(NULL, "/");
      continue;
    }

    if (strcmp(component, "..") == 0)
    {
      struct ext2_inode current = read_inode(current_inode);
      unsigned int parent_inode;

      if (find_entry_in_dir(&current, "..", &parent_inode) != 0)
      {
        free(path_copy);
        return -1;
      }

      current_inode = parent_inode;
      component = strtok(NULL, "/");
      continue;
    }

    struct ext2_inode current = read_inode(current_inode);
    unsigned int child_inode;

    if (find_entry_in_dir(&current, component, &child_inode) != 0)
    {
      free(path_copy);
      return -1;
    }

    current_inode = child_inode;
    component = strtok(NULL, "/");
  }

  *result_inode = current_inode;
  free(path_copy);
  return 0;
}

void ext2_cd(const char *path)
{
  unsigned int target_inode;
  char abs_path[MAX_PATH_LEN] = {0};
  char old_path[MAX_PATH_LEN] = {0};

  strncpy(old_path, g_state.cwd_path, MAX_PATH_LEN - 1);

  if (path == NULL || strlen(path) == 0)
  {
    path = "/";
  }

  if (path[0] == '/')
  {
    strncpy(abs_path, path, MAX_PATH_LEN - 1);
  }
  else
  {
    if (strcmp(g_state.cwd_path, "/") != 0)
    {
      strncat(abs_path, g_state.cwd_path, MAX_PATH_LEN - 1);
      strncat(abs_path, "/", MAX_PATH_LEN - strlen(abs_path) - 1);
    }
    strncat(abs_path, path, MAX_PATH_LEN - strlen(abs_path) - 1);
  }

  abs_path[MAX_PATH_LEN - 1] = '\0';

  if (resolve_path(abs_path, &target_inode) != 0)
  {
    strncpy(g_state.cwd_path, old_path, MAX_PATH_LEN - 1);
    fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
    return;
  }

  struct ext2_inode inode = read_inode(target_inode);
  if ((inode.i_mode & 0xF000) != EXT2_S_IFDIR)
  {
    strncpy(g_state.cwd_path, old_path, MAX_PATH_LEN - 1);
    fprintf(stderr, "cd: %s: Não é um diretório\n", path);
    return;
  }

  strncpy(g_state.cwd_path, abs_path, MAX_PATH_LEN - 1);
  normalize_path(g_state.cwd_path);
  g_state.cwd_inode = target_inode;
}

void ext2_ls()
{
  struct ext2_inode cwd_inode = read_inode(g_state.cwd_inode);
  void *block = malloc(g_state.block_size);

  // printf("Conteúdo de %s:\n", g_state.cwd_path);

  for (int block_idx = 0; block_idx < EXT2_NDIR_BLOCKS; block_idx++)
  {
    if (cwd_inode.i_block[block_idx] == 0)
      break;

    lseek(g_state.fd, BLOCK_OFFSET(cwd_inode.i_block[block_idx]), SEEK_SET);
    if (read(g_state.fd, block, g_state.block_size) != g_state.block_size)
    {
      fprintf(stderr, "Erro ao ler bloco: %s\n", strerror(errno));
      continue;
    }

    struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)block;
    unsigned int pos = 0;

    while (pos < g_state.block_size && entry->rec_len > 0)
    {
      if (entry->inode == 0)
      {
        pos += entry->rec_len;
        entry = (void *)entry + entry->rec_len;
        continue;
      }

      char name[NAMELEN + 1];
      memcpy(name, entry->name, entry->name_len);
      name[entry->name_len] = '\0';

      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      {
        pos += entry->rec_len;
        entry = (void *)entry + entry->rec_len;
        continue;
      }

      struct ext2_inode inode = read_inode(entry->inode);
      printf("%10u %s%s   ",
             entry->inode,
             name,
             (inode.i_mode & 0xF000) == EXT2_S_IFDIR ? "" : "");

      pos += entry->rec_len;
      entry = (void *)entry + entry->rec_len;
    }
  }

  printf("\n");
  free(block);
}

void ext2_find(const char *input_path, const char *prefix)
{
  unsigned int target_inode;
  char abs_path[MAX_PATH_LEN] = {0};
  struct ext2_inode inode;

  if (strlen(g_state.cwd_path) + strlen(input_path) + 2 > MAX_PATH_LEN)
  {
    fprintf(stderr, "find: Caminho muito longo\n");
    return;
  }

  if (input_path[0] == '/')
  {
    strncpy(abs_path, input_path, MAX_PATH_LEN - 1);
  }
  else
  {
    strncpy(abs_path, g_state.cwd_path, MAX_PATH_LEN - 1);
    if (abs_path[strlen(abs_path) - 1] != '/')
    {
      strncat(abs_path, "/", MAX_PATH_LEN - strlen(abs_path) - 1);
    }
    strncat(abs_path, input_path, MAX_PATH_LEN - strlen(abs_path) - 1);
  }

  if (resolve_path(abs_path, &target_inode) != 0)
  {
    fprintf(stderr, "find: %s: %s\n", abs_path, strerror(errno));
    return;
  }

  inode = read_inode(target_inode);
  _ext2_find_recursive(&inode, abs_path, prefix);
}

void _ext2_find_recursive(struct ext2_inode *inode, const char *path, const char *prefix)
{
  if ((inode->i_mode & 0xF000) != EXT2_S_IFDIR)
    return;

  void *block = malloc(g_state.block_size);
  int first = 1;

  for (int block_idx = 0; block_idx < EXT2_NDIR_BLOCKS; block_idx++)
  {
    if (inode->i_block[block_idx] == 0)
      break;

    lseek(g_state.fd, BLOCK_OFFSET(inode->i_block[block_idx]), SEEK_SET);
    if (read(g_state.fd, block, g_state.block_size) != g_state.block_size)
    {
      fprintf(stderr, "Erro ao ler bloco %u: %s\n", inode->i_block[block_idx], strerror(errno));
      continue;
    }

    struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)block;
    unsigned int pos = 0;
    int count = 0;

    while (pos < g_state.block_size && entry->rec_len > 0)
    {
      if (entry->inode != 0 &&
          strncmp(entry->name, ".", entry->name_len) != 0 &&
          strncmp(entry->name, "..", entry->name_len) != 0)
      {
        count++;
      }
      pos += entry->rec_len;
      entry = (void *)entry + entry->rec_len;
    }

    pos = 0;
    entry = (struct ext2_dir_entry_2 *)block;
    int index = 0;

    while (pos < g_state.block_size && entry->rec_len > 0)
    {
      if (entry->inode == 0)
      {
        pos += entry->rec_len;
        entry = (void *)entry + entry->rec_len;
        continue;
      }

      char name[NAMELEN + 1];
      memcpy(name, entry->name, entry->name_len);
      name[entry->name_len] = '\0';

      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      {
        pos += entry->rec_len;
        entry = (void *)entry + entry->rec_len;
        continue;
      }

      char full_path[MAX_PATH_LEN];
      snprintf(full_path, MAX_PATH_LEN, "%s/%s", path, name);

      printf("%s%s── %s", prefix, (index == count - 1) ? "└" : "├", name);

      struct ext2_inode child_inode = read_inode(entry->inode);
      if ((child_inode.i_mode & 0xF000) == EXT2_S_IFDIR)
      {
        printf("/");
      }
      printf("\n");

      if ((child_inode.i_mode & 0xF000) == EXT2_S_IFDIR)
      {
        char new_prefix[MAX_PATH_LEN];
        snprintf(new_prefix, MAX_PATH_LEN, "%s%s   ",
                 prefix,
                 (index == count - 1) ? " " : "│");
        _ext2_find_recursive(&child_inode, full_path, new_prefix);
      }

      index++;
      pos += entry->rec_len;
      entry = (void *)entry + entry->rec_len;
    }
  }

  free(block);
}

void ext2_shell()
{
  char line[256];

  printf("Bem-vindo ao shell EXT2! Comandos disponíveis:\n");
  printf("cd <diretório> - Muda para o diretório especificado.\n");
  printf("ls - Lista os arquivos e diretórios do diretório atual.\n");
  printf("find <diretório> - Encontra um arquivo ou diretório. \n");
  printf("exit - Encerra o shell.\n\n");

  while (1)
  {
    printf("%s $ ", g_state.cwd_path);
    if (!fgets(line, sizeof(line), stdin))
      break;

    line[strcspn(line, "\n")] = 0;
    char *cmd = strtok(line, " ");
    char *arg = strtok(NULL, "");

    if (cmd == NULL)
      continue;

    if (strcmp(cmd, "cd") == 0)
    {
      ext2_cd(arg ? arg : "/");
    }
    else if (strcmp(cmd, "ls") == 0)
    {
      ext2_ls();
    }
    else if (strcmp(cmd, "find") == 0)
    {
      printf("\n");
      if (arg)
      {
        ext2_find(arg, "");
      }
      else
      {
        ext2_find(g_state.cwd_path, "");
      }
      printf("\n");
    }
    else if (strcmp(cmd, "exit") == 0)
    {
      break;
    }
    else
    {
      printf("Comandos disponíveis: cd, ls, find, exit\n");
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Uso: %s <imagem.ext2>\n", argv[0]);
    return 1;
  }

  g_state.fd = open(argv[1], O_RDONLY);
  if (g_state.fd < 0)
  {
    perror("Falha ao abrir a imagem");
    return 1;
  }

  lseek(g_state.fd, BASE_OFFSET, SEEK_SET);
  if (read(g_state.fd, &g_state.sb, sizeof(g_state.sb)) != sizeof(g_state.sb))
  {
    perror("Falha ao ler o superbloco");
    close(g_state.fd);
    return 1;
  }

  g_state.block_size = 1024 << g_state.sb.s_log_block_size;
  g_state.cwd_inode = 2;
  strcpy(g_state.cwd_path, "/");

  ext2_shell();

  close(g_state.fd);
  return 0;
}