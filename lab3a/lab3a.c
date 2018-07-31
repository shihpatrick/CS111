#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "ext2_fs.h"

#define OFFSET_SB 1024
#define SIZE_SB 2048
#define SIZE_TD 32
#define SIZE_I 128

/* variables used across all func */
int image_fd;
int num_group;
__u32 size_block;

/* declare struct for elements of fs */
struct ext2_super_block super_block;
struct ext2_group_desc *group_desc;
struct ext2_inode inode;
struct ext2_dir_entry dir_entry;

/* store values from pread() */
__u8 buf_8;
__u16 buf_16;
__u32 buf_32;
__s32 sbuf_32;

void read_super_block(){
  int rc;
  if (rc = pread(image_fd, &super_block, EXT2_MIN_BLOCK_SIZE, EXT2_MIN_BLOCK_SIZE) < EXT2_MIN_BLOCK_SIZE){
    fprintf(stderr, "Failure to read super block");
    exit(2);
  }

  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", super_block.s_blocks_count, super_block.s_inodes_count, 
	1024 << super_block.s_log_block_size, super_block.s_inode_size,
	super_block.s_blocks_per_group, super_block.s_inodes_per_group, super_block.s_first_ino);
}

void read_group(){
  int struct_size = sizeof(struct ext2_group_desc);
  group_desc = malloc(num_group*struct_size);
 
  int i;
  for(i = 0; i < num_group; i++){
    pread(image_fd, &group_desc[i], struct_size, EXT2_MIN_BLOCK_SIZE + sizeof(struct ext2_super_block) + i*struct_size);

    fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, super_block.s_blocks_count, 
      super_block.s_inodes_per_group,group_desc[i].bg_free_blocks_count,
      group_desc[i].bg_free_inodes_count, group_desc[i].bg_block_bitmap,
      group_desc[i].bg_inode_bitmap, group_desc[i].bg_inode_table);
  }
}

void read_free_blocks(){
  int i;
  for(i = 0; i < num_group; i++){
    int j;
    for(j = 0; j < size_block; j++){
      pread(image_fd, &buf_8, 1, group_desc[i].bg_block_bitmap*size_block+j);
      int k;
      for (k = 0; k < 8; k++){
	int val = buf_8 & (1 << k);
	if (val == 0){
	  fprintf(stdout, "BFREE,%d\n", j*8+k+1+(i*size_block));
        }
      }
    }
  }
}

void read_free_inodes(){
  int i;
  for(i = 0; i < num_group; i++){
    int j;
    for(j = 0; j < size_block; j++){
      pread(image_fd, &buf_8, 1, group_desc[i].bg_inode_bitmap*size_block+j);
      int k;
      for (k = 0; k < 8; k++){
	int val = buf_8 & (1 << k);
	if (val == 0){
	  fprintf(stdout, "IFREE,%d\n", j*8+k+1+(i*super_block.s_inodes_per_group));
        }
      }
    }
  }
}

void read_inode_summary(){
  /*variables for fprintf */
  __u32 mode;
  __u32 num_blocks;
  int inode_num = 1;
  
  char file_type;
  int i;
  for (i = 0; i < num_group; i++){
    int j;
  
    for(j = 0; j < size_block; j++){
      pread(image_fd, &buf_8, 1, group_desc[i].bg_inode_bitmap*size_block+j);
      int8_t mask = 1;
      int k;
      for (k = 0; k < 8; k++){
	 int val = buf_8 & (1 << k);
	 if (val != 0 && (j*8+k) < super_block.s_inodes_per_group){
	  pread(image_fd, &inode, sizeof(struct ext2_inode), (k+8*j)*sizeof(struct ext2_inode) + size_block*group_desc[i].bg_inode_table); 

	  //mode
	  mode = inode.i_mode & ((1 << 12)-1);

	  if (mode != 0 && inode.i_links_count != 0){

	    //file type
	    if(inode.i_mode & 0x8000)
	      file_type = 'f';
	    else if (inode.i_mode & 0xA000)
	      file_type = 's';
	    else if (inode.i_mode & 0x4000)
	      file_type = 'd';
	    else
	      file_type = '?';

	    //time of last I-node change
	    char c_time_str[32];
	    time_t c_time = (time_t) inode.i_ctime;
	    strftime(c_time_str, 32, "%m/%d/%y %H:%M:%S", gmtime(&c_time));

	    //modification time
	    char m_time_str[32];
	    time_t m_time = (time_t) inode.i_mtime;
	    strftime(m_time_str, 32, "%m/%d/%y %H:%M:%S", gmtime(&m_time));
	
	    //time of last access
  	    char a_time_str[32];
	    time_t a_time = (time_t) inode.i_atime;
	    strftime(a_time_str, 32, "%m/%d/%y %H:%M:%S", gmtime(&a_time));
	  
	    //num of blocks
	    num_blocks = inode.i_blocks/(1 << super_block.s_log_block_size);
	  
	    fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d,", inode_num, file_type,
	      mode, inode.i_uid, inode.i_gid, inode.i_links_count,
	      c_time_str, m_time_str, a_time_str, inode.i_size, num_blocks);
	    int m;
	    for (m = 0; m < EXT2_N_BLOCKS; m++){
		if (m == EXT2_TIND_BLOCK)
		  fprintf(stdout, "%d\n", inode.i_block[m]);
		else
		  fprintf(stdout, "%d,", inode.i_block[m]);
	    }
	  }
        }
      inode_num++;
      }
    }
  } 
}

void read_dir_summary(){
  int inode_num = 1;
  int i;
  __u32 address;
  __u32 dir_addr;
  int offset = 0;

  for (i = 0; i < num_group; i++){
    //if (i == num_group-1; i++
    int j;
    for (j = 0; j < size_block; j++){
      pread(image_fd, &buf_8, 1, group_desc[i].bg_inode_bitmap*size_block+j);
      int8_t mask = 1;
      int k;

      for (k = 0; k < 8; k++){
	int val = buf_8 & (1 << k);
	
	if (val != 0 && (j*8+k) < super_block.s_inodes_per_group){
    	
	  pread(image_fd, &inode, sizeof(struct ext2_inode), (k+8*j)*sizeof(struct ext2_inode) + size_block*group_desc[i].bg_inode_table); 
	 
	  if(inode.i_mode & 0x4000){
	    int m;
	    for (m = 0; m < 12; m++){
	      offset = 0;
	      address = inode.i_block[m];
	      dir_addr = address * size_block;
	
	      if (address == 0)
		continue;
	      while(dir_addr < size_block*address + size_block){
	        pread(image_fd, &dir_entry, sizeof(struct ext2_dir_entry), dir_addr);
	        if (dir_entry.inode != 0){
	          fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_num, offset, dir_entry.inode,
		    dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
	      	}
	 	 offset += dir_entry.rec_len;
		 dir_addr += dir_entry.rec_len;
	      }
	    }
    	    //indirect
	    address = inode.i_block[12];
	    offset = 0;
	    if (address != 0){
	      for(m = 0; m < size_block/4; m++){
		dir_addr = size_block*address+m*4;
		__u32 dir_addr_2;
		pread(image_fd, &dir_addr_2, 4, dir_addr);
		if (dir_addr_2 != 0){
		  dir_addr = dir_addr_2 * size_block;
		  while (dir_addr < size_block + dir_addr_2*size_block){
		    pread(image_fd, &dir_entry, sizeof(struct ext2_dir_entry), dir_addr);
		    if (dir_entry.inode != 0){
	              fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,\'%s\'\n", inode_num, offset, dir_entry.inode,
		        dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
		     
	            }
 		   offset += dir_entry.rec_len;
		   dir_addr += dir_entry.rec_len;
	          }
		  offset = 0;
		}
	      }	
	    }

	   //double indirect
	   address = inode.i_block[13];
	   offset = 0;
    	    if (address != 0){
	      for(m = 0; m < size_block/4; m++){
		dir_addr = size_block*address+m*4;
		__u32 dir_addr_2;
		pread(image_fd, &dir_addr_2, 4, dir_addr);
		if (dir_addr_2 != 0){
		  __u32 dir_addr_3;
		  int n;
		  for (n = 0; n < size_block/4; n++){
		    offset = 0;
		    pread(image_fd, &dir_addr_3, 4, dir_addr_2 * size_block + n*4);
		    if (dir_addr_3 != 0){
		      dir_addr = dir_addr_3 * size_block;
		      while (dir_addr < size_block + dir_addr_3*size_block){
		   	pread(image_fd, &dir_entry, sizeof(struct ext2_dir_entry), dir_addr);
		    	if (dir_entry.inode != 0){
	             	  fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_num, offset, dir_entry.inode,
		            dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
		      	  
	                }
		      offset += dir_entry.rec_len;
		      dir_addr += dir_entry.rec_len;
		      }
		    }
		  }
		}
	      }	
	    }
	    //triple indirect
	    address = inode.i_block[14];

	    offset = 0;
    	    if (address != 0){
	      for(m = 0; m < size_block/4; m++){
		dir_addr = size_block*address+m*4;
		__u32 dir_addr_2;
		pread(image_fd, &dir_addr_2, 4, dir_addr);
		if (dir_addr_2 != 0){
		  __u32 dir_addr_3;
		  int n;
		  for (n = 0; n < size_block/4; n++){
		    pread(image_fd, &dir_addr_3, 4, dir_addr_2 * size_block + (n*4));
		    if (dir_addr_3 != 0){
		      __u32 dir_addr_4;
		      int o;
		      for (o = 0; o < size_block/4; o++){
			offset = 0;
			pread(image_fd, &dir_addr_4, 4, dir_addr_3 * size_block + (o*4));
			if (dir_addr_4 != 0){
 		          dir_addr = dir_addr_4 * size_block;
		   	  while (dir_addr < size_block + dir_addr_4*size_block){
		   	    pread(image_fd, &dir_entry, sizeof(struct ext2_dir_entry), dir_addr);
		    	    if (dir_entry.inode != 0){
	             	      fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_num, offset, dir_entry.inode,
		               dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
		      	    
	                    }
 			  offset += dir_entry.rec_len;
		   	  dir_addr += dir_entry.rec_len;
	          	  }
		        }
		      }
		    }
		  }
	        }
	      }
            }  
	  }
	}
      inode_num++;
     }  
    }
  }
}


void process_indirects(int inode_num, int levels, __u32 block_num){
  if (levels == 0)
    return;

  __u32 *curr = (__u32*) malloc(size_block);
  __u32 base = block_num*size_block;
  pread(image_fd, curr, size_block, base);
  
  int i;
  for (i = 0; i < size_block; i += 4){
    if (curr[i/4] == 0)
	break;
    __u32 offset = 12;
    if (levels >= 2)
      offset += 256;
    if (levels == 3)
      offset += 256*256;

    offset += i/4;
    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num,
	levels, offset, block_num, curr[i/4]);    
  }
 
  for (i = 0; i < size_block; i+=4){
    if(curr[i/4])
      process_indirects(inode_num, levels - 1, curr[i/4]);
  }

}

void indirect_output(){
  int inode_num = 1;
  int i;
  __u32 address;
  __u32 dir_addr;
  __u32 num_blocks;
  int offset = 0;
  for (i = 0; i < num_group; i++){
    int j;
    for (j = 0; j < size_block; j++){
      pread(image_fd, &buf_8, 1, group_desc[i].bg_inode_bitmap*size_block+j);

      int k;
      for (k = 0; k < 8; k++){
	int val = buf_8 & (1 << k);

	if (val != 0 && (j*8+k) < super_block.s_inodes_per_group){
	  pread(image_fd, &inode, sizeof(struct ext2_inode), (k+8*j)*sizeof(struct ext2_inode) + size_block*group_desc[i].bg_inode_table); 

	  num_blocks = inode.i_blocks/ (1 << super_block.s_log_block_size);

	  if (num_blocks > 10){
	    int l; 
	    int levels = 1; 
	    for (l = EXT2_IND_BLOCK; l < EXT2_N_BLOCKS; l++){
	      if (inode.i_block[l]){  
	        process_indirects(inode_num, levels, inode.i_block[l]);
		levels++;
  	      }
	    }
	  }
        }
      inode_num++;
      }
    }
  }
}

int main(int argc, char **argv){
  if (argc != 2){
    fprintf(stderr, "Invalid argument. Correct Usage: ./lab3a *.img");
    exit(1);
  }

  image_fd = open(argv[1], O_RDONLY);
   
  read_super_block();

  size_block = 1024 << super_block.s_log_block_size; //conversion
  num_group = super_block.s_blocks_count/super_block.s_blocks_per_group;
  if (super_block.s_blocks_count%super_block.s_blocks_per_group)
    num_group++;

  read_group();
  read_free_blocks();
  read_free_inodes();
  read_inode_summary();
  read_dir_summary();
  indirect_output();

  exit(0);
}
