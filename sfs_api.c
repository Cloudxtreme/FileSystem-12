#include "sfs_api.h"
#include "disk_emu.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define DISK_FILE "sfs_disk.disk"
#define BLOCK_SIZE 512
#define MAX_BLOCKS 1024
#define INODE_BLOCK_NUM 10
#define MAX_INODES 105 
#define MAX_FILES 104

super_block_t sb; 
dir_entry_t root_dir[MAX_INODES];

inode_t inode_table[MAX_INODES];
fd_table_t fd_table[MAX_FILES];

unsigned short free_blocks[MAX_BLOCKS];

char file_data[] = "abcdefghijklmnopqrstuvwxyz";

void init_superblock(){
        // init the superblock
        sb.magic = 1234;
        sb.block_size = BLOCK_SIZE;
        sb.fs_size = MAX_BLOCKS*BLOCK_SIZE;
        sb.inode_table_len = MAX_INODES;
        sb.root_dir_inode = 0;
}

void add_root_dir_inode(){

        //first entry in the inode table is the root
        inode_table[0].link_cnt = 1;
        inode_table[0].id = 0;
        inode_table[0].size = 45;
        inode_table[0].data_ptrs[0] = 2; //root dir is stored in the 3rd block
}

void init_inode_table()
{
	for(int i=1; i<MAX_INODES; i++)
	{
		inode_table[i].id = -1;  //set all ids to -1, so that we can track available inodes 
	}
}
void init_inode(int index)
{
	inode_table[index].id = index; 
    inode_table[index].link_cnt = 0; //num_blocks
    inode_table[index].size = 0;
    for (int i = 0; i< 12; i++)
    {
    	inode_table[index].data_ptrs[i] = -1; 
    }
    inode_table[index].indirect_ptr = -1; 
}

void init_root_dir() 
{
	for (int i = 0; i<MAX_INODES; i++)
	{
		root_dir[i].inode_idx = -1;
	}
}

void write_root_dir()
{
	 write_blocks(2,1, &root_dir);
}

void init_fd_table()
{
	for (int i = 0; i < MAX_FILES; i++)
	{
		fd_table[i].inode_idx = -1; 
	}
}

void zero_everything()
{
        bzero(&sb, sizeof(super_block_t));
        bzero(&fd_table[0], sizeof(fd_table_t)*MAX_FILES);
        bzero(&inode_table[0], sizeof(inode_t)*MAX_INODES);
        bzero(&root_dir, sizeof(dir_entry_t));
        bzero(&free_blocks[0], sizeof(int)*MAX_BLOCKS);
}

void mksfs(int fresh) {
	//Implement mksfs here	
	if (fresh == 1) {

                //begin
        printf("Initalizing sfs\n");
		init_fresh_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
        zero_everything();

        		init_root_dir();
        		init_fd_table();
		// write superblock to the first block
                printf("Writing superblocks\n");
                init_superblock();
				write_blocks(0, 1, &sb);
	
		
		// write the inode table to the 2nd block
                printf("Writing inode table\n");
                add_root_dir_inode();
                init_inode_table(); 
				write_blocks(1, 1, &inode_table);

        // write root directory data to the 3rd block
                printf("Writing root dir\n");
                write_blocks(2,1, &root_dir);
                
        //mark blocks as used
                printf("Writing free blocks\n");
				free_blocks[0] = 1; //superblock
				free_blocks[1] = 1; //inode table
				free_blocks[2] = 1; //root dir data
				free_blocks[MAX_BLOCKS-1] = 1; //free blocks

        // write the free blocks to the disk
				write_blocks(MAX_BLOCKS - 1, 1, &free_blocks);
				printf("writing blocks\n");

	} else {

		init_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
                // pull back data from disk to mem
	}

	return;
}

int sfs_getnextfilename(char *fname) {

		printf("Calling sfs get next file\n");
		int i =0; 
		for (i = 0; i < MAX_INODES; i++)
		{
			if (strcmp(root_dir[i].name, fname) == 0)
			{
				strcpy(fname, root_dir[i].name);
			}
		}
	return 0;
}


int sfs_getfilesize(const char* path) {

	//Implement sfs_getfilesize here	
	int inode_idx; 
	for (int i =0; i<MAX_INODES; i++)
	{
		if (strcmp(root_dir[i].name, path)==0)
		{
			inode_idx = root_dir[i].inode_idx; 
		}
	}
    inode_t inode = inode_table[inode_idx];
    return inode.size;
}

int create_file(char *name) 
{
	printf("Creating file %s\n", name);
	
	int index; 
	for (int i =0; i<MAX_INODES; i++)
	{
		if (inode_table[i].id == -1)
		{
			index = i;
			break;  // we have found the next available free inode table entry 
		} 
	}
	init_inode(index);

    for (int i = 0; i< MAX_INODES; i++)
    {
    	if (root_dir[i].inode_idx == -1)
    	{
    		index = i; 
    		break; 
    	}
    }
    strncpy(root_dir[index].name, name, MAXFILENAME); 
    root_dir[index].inode_idx = index; 
    write_root_dir();
 	return index; 
}

int sfs_fopen(char *name) {

	for (int i = 0; i<MAX_FILES; i++)
	{
		if (fd_table[i].inode_idx != -1 )
		{
			printf("Name stored in root directory %s \n", root_dir[fd_table[i].inode_idx].name);
			printf("Name passed in %s\n", name);
			if (strcmp(root_dir[fd_table[i].inode_idx].name, name) == 0)
			{
				printf("File %s is already open\n", name);
				return -1; 
			}
		}
	}
	int exists = 0;
	int index;
	for (int i = 0; i<MAX_FILES; i++)
	{
		if (root_dir[i].inode_idx != -1) 
		{
			if (strcmp(root_dir[i].name, name) == 0) 
			{
				
			index = i;
			exists = 1;  
			}
		}
	}
	if (!exists)
	{
		index = create_file(name); 
	}	
    printf("Opening %s\n", name);
    int fd_ind; 
    for (int i = 0; i<MAX_FILES; i++)
    {
    	if (fd_table[i].inode_idx == -1)
    	{
    		fd_ind = i; 
    		break;
    	}   
    }
	//set the fd table
	fd_table[fd_ind].inode_idx = index; 
    fd_table[fd_ind].rd_write_ptr = 0;
    printf("Updated fd table at index %d to %d\n", fd_ind, index);
	return fd_ind;
}

int sfs_fclose(int fileID){

		//Implement sfs_fclose here
		if (fd_table[fileID].inode_idx !=-1)
		{	
        	fd_table[fileID].inode_idx = -1;
        	fd_table[fileID].rd_write_ptr = -1;
        	return 0; 
        }
    printf("This file %d has already been closed\n", fileID );
	return -1;
}

int get_next_inode_pointer_block(inode_t crt_inode,  int file_block) 
{
	int disk_block;
	int* indirect_buffer = malloc(BLOCK_SIZE); 
	if (file_block < 12)
	{	
		disk_block  = crt_inode.data_ptrs[file_block]; 
	}
	else
	{	
		//get the indirect pointer block 
		read_blocks(crt_inode.indirect_ptr, 1, indirect_buffer); 
		int i = file_block - 12; 
		disk_block = indirect_buffer[i * sizeof(int)]; 
	}
	free(indirect_buffer);
	return disk_block; 
}

int sfs_fread(int fileID, char *buf, int length){

	//Implement sfs_fread here	
        int r_len = length;
        int inode_idx = fd_table[fileID].inode_idx;
        int crt_pos = fd_table[fileID].rd_write_ptr;
        char buffer[BLOCK_SIZE];
        inode_t crt_inode = inode_table[inode_idx];
        int block_idx; 
        int offset = crt_pos % 512; 
        int num_blocks_to_read = (length + offset) /BLOCK_SIZE; 
        int start_block = get_next_inode_pointer_block(crt_inode, crt_pos/BLOCK_SIZE);
        //read start block
        read_blocks(start_block, 1, buffer);
        char first_buffer [BLOCK_SIZE-offset]; 
        
        //trying to read beyond the file
        if (crt_pos + length > crt_inode.size)
        {
            length = crt_inode.size - crt_pos;
            printf("Trying to read beyond the file\n");
            return -1;
        }
        
        for (int i = offset; i< BLOCK_SIZE; i++)
        {
        	first_buffer[i] = buffer[i];
        } 
        if (BLOCK_SIZE > length)
        {
        	memcpy(buf, first_buffer, length);
        }
    	else 
    	{
    		memcpy(buf, first_buffer, BLOCK_SIZE);
    		length = length - (BLOCK_SIZE-offset);
    		int j = 0; 
    		while (length > 0)
    		{ 
    			for (int i = crt_pos/BLOCK_SIZE; i < num_blocks_to_read; i++)
	    		{
	    			block_idx = get_next_inode_pointer_block(crt_inode, i); 
	    			read_blocks(block_idx, 1, buffer); 
	    			if (BLOCK_SIZE> length) 
	    			{
	    				memcpy(buf+(BLOCK_SIZE-offset)+(j*BLOCK_SIZE), 1, buffer);
	    				j++;
	    			}
	    			else 
	    			{
	    				memcpy((buf+(BLOCK_SIZE-offset)+((j-1)*BLOCK_SIZE)+length), 1, buffer);
	    			}
	    		}
	    		length = length - BLOCK_SIZE;
	    	}
	    	return r_len;
    	}

	return 0;
}
int get_next_free_block()
{
	int next_block;
	for(int i = 0; i < MAX_BLOCKS; i++)
	{
		if (free_blocks[i] == 0) 
		{
			next_block = i;
			break;  
		} 		
	}
	return next_block; 
}

int allocate_next_block(inode_t crt_inode)
{
	int inode_block;
	char* indirect_buffer = malloc(BLOCK_SIZE); 
	//int num_blocks = extra_bytes / BLOCK_SIZE + 1; 
	int index = 0; 
	for (int i = 0; i < 12; i++)
	{
		if (crt_inode.data_ptrs[i] == -1)
		{
			index = i;
			break;  	  
		}
	}
	int next_block = get_next_free_block();
	//set the block you are going to write to to be used in the free_block map
	free_blocks[next_block] = 1;
	if (index < 12) 
	{
		crt_inode.data_ptrs[index] = next_block; 
	}
	else 
	{
		//initialize the indirect pointer
		if (crt_inode.indirect_ptr == -1)
		{	
			inode_block = get_next_free_block();
			free_blocks[inode_block] = 1;
			crt_inode.indirect_ptr = inode_block;
			memcpy(indirect_buffer, &next_block, sizeof(int)); 
		}
		else
		{
			inode_block = crt_inode.indirect_ptr;
			read_blocks(inode_block, 1, indirect_buffer);
			for (int i = 0; i < BLOCK_SIZE; i+= sizeof(int))
			{
				if(indirect_buffer[i] == 0)
				{
					indirect_buffer += i; 
					memcpy(indirect_buffer, &next_block, sizeof(int)); 
				}
			} 
		}
		write_blocks(inode_block, 1, indirect_buffer);
	}
	return next_block;  
}


int sfs_fwrite(int fileID, const char *buf, int length)
{
	//determine the block we should start writing to
	inode_t crt_inode = inode_table[fd_table[fileID].inode_idx];
	int rd_write_ptr = fd_table[fileID].rd_write_ptr;
	int file_block = rd_write_ptr / BLOCK_SIZE;
	int disk_block = get_next_inode_pointer_block(crt_inode, file_block);
	char* block_buffer = malloc(BLOCK_SIZE);
	int num_blocks = 1;  
	read_blocks(disk_block, num_blocks, block_buffer);
	//create a new buffer with the saved file data from before the read-write pointer, and the new data to append
	int offset = rd_write_ptr % BLOCK_SIZE;
	char buffer[length + offset];
	int buf_counter = 0; 
	for (int i = 0; i< offset; i++)
	{
		buffer[i] = block_buffer[i]; 
	}
	for(int i = offset; i < offset + length; i++)
	{
		buffer[i] = buf[buf_counter];
		buf_counter++;
	} 
	buf_counter = 0; 
	int num_blocks_in_file = crt_inode.size/BLOCK_SIZE + 1;
	int num_blocks_taken = rd_write_ptr/BLOCK_SIZE;
	int num_blocks_available = num_blocks_in_file - num_blocks_taken;  
	int j = 0; 
		while(buf_counter < length + offset)
		{ 
			char* write_buf; 
			for (int i = buf_counter; i < BLOCK_SIZE && i < length + offset; i++)
			{
				write_buf[i] = buffer[i]; 
			}
			if (buf_counter != 0)
			{ 
				if (j < num_blocks_available)
				{
					disk_block = get_next_inode_pointer_block(crt_inode, file_block + 1); 
				}
				else
				{
					disk_block = allocate_next_block(crt_inode); 
				} 
			}
			write_blocks(disk_block, 1, &write_buf);
			buf_counter += BLOCK_SIZE;
			j++;   
		}
		crt_inode.size = rd_write_ptr + length; 
		fd_table[fileID].rd_write_ptr = rd_write_ptr + length;
		free(block_buffer);
		return 0; 
} 

int sfs_fseek(int fileID, int loc)
{        
        //should check if loc is a valid length
        if (loc > inode_table[fd_table[fileID].inode_idx].size)
        {
        	printf("The seek location is greater than the filesize\n");
        	return -1;
        }
        if (loc < 0)
        {
        	printf("The seek location is not a valid file location\n");
        	return -1; 
        }
        fd_table[fileID].rd_write_ptr = loc;
		return 0;
}
void free_block_map(int block)
{
	free_blocks[block] = 0; 
}

void clear_data(int inode_idx)
{
	int block; 
	char * buf = malloc(BLOCK_SIZE);
	inode_t crt_inode = inode_table[inode_idx]; 
	int filesize = crt_inode.size;
	for(int i = 0; i < filesize; i += 512)
	{
		block = get_next_inode_pointer_block(crt_inode, i/512);
		free_block_map(block); 
		write_blocks(block, 1, &buf);
	}
	if (filesize/512 >= 12)
	{
		write_blocks(crt_inode.indirect_ptr, 1, &buf);
		free_block_map(crt_inode.indirect_ptr); 
	}
}

void clear_inodes(int index)
{
	inode_table[index].id = -1;
	inode_table[index].link_cnt = 0; //num_blocks
    inode_table[index].size = 0;
    for (int i = 0; i< 12; i++)
    {
    	inode_table[index].data_ptrs[i] = -1; 
    }
    inode_table[index].indirect_ptr = -1; 
}

void clear_dir_entry(int index)
{
	for(int i = 0; i < MAX_INODES; i++)
	{
		if (root_dir[i].inode_idx== index)
		{
			root_dir[i].inode_idx = -1; 
		}
	}
}


int sfs_remove(char *file) {
        
    //clear the data blocks
	int inode_idx; 
	for (int i =0; i<MAX_INODES; i++)
	{
		if (strcmp(root_dir[i].name, file)==0)
		{
			inode_idx = root_dir[i].inode_idx; 
		}
	}
	clear_data(inode_idx); 
    clear_dir_entry(inode_idx);
    clear_inodes(inode_idx);
	return 0;
}
