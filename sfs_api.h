
#define MAXFILENAME 20

void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);


typedef struct super_block {
int magic;
int block_size;
int fs_size;
int inode_table_len;
int root_dir_inode;
} super_block_t;


typedef struct inode { 
	 int link_cnt;
	 int id;
		int size;
	 int data_ptrs[12];
	 int indirect_ptr; 
} inode_t;


typedef struct dir_entry { 
	char name[MAXFILENAME+1];
	int inode_idx;
} dir_entry_t;


typedef struct fd_table { 
	int inode_idx;
	int rd_write_ptr;
} fd_table_t;

