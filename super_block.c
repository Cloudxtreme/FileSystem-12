#include "disk_emu.h"
#include <stdio.h> 
#include <stdlib.h>

void setup_super_block(int BLOCKSIZE, int NUM_BLOCKS, int INODE_LEN)
{
	*super_block = malloc(BLOCKSIZE); 
	super_block[0] = "MagicNumber"; 
	super_block[1] = BLOCKSIZE; 
	super_block[2] = NUM_BLOCKS;
	super_block[3] = INODE_LEN;
	super_block[4] =  


}
