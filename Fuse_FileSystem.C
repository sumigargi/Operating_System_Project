#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#define DEFAULT_BLOCKSIZE   4096
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#include "log.h"

/* Super Block Initialization */
/* void* init(struct fuse_conn_info *conn)
Initialize the filesystem. This function can often be left unimplemented, but it can be a handy way to perform one-time setup such as allocating variable-sized data structures or initializing a new filesystem. The fuse_conn_info structure gives information about what features are supported by FUSE, and can be used to request certain capabilities (see below for more information). The return value of this function is available to all file operations in the private_data field of fuse_context. It is also passed as a parameter to the destroy() method.
static void fs_init(struct fuse_conn_info *conn) */

{
CreateFiles();
Struct SuperBlock *SB = malloc (sizeof(struct SuperBlock ));
SB-> CreationTime = 1376483073;
SB-> Mounted =50;
SB-> deviceId = 20;
SB-> freeStart =1
SB-> freeEnd=25;
SB-> root=26
SB-> maxBlocks = 10000;



/*  To write the SuperBlock Info into fusedata.0 file */
fptr= fopen("/fusedata/fusedata.0", "w");
if(fptr!=NULL)
{
fwrite(SB, sizeof(struct SuperBlock),1, fptr);
fclose(fptr);
}
for(int i=1; i<26; i++)
{
	for(int j=0; j<400; j++)
	{
	if(i==1 && j <28)
	{
	freeblockarr[i][j]=1;
	}
else
{
freeblockarr[i][j]=0;
}
}
String freefilename= strcat(prefix, x);
fileptr= fopen(freefilename, "w");
if(fileptr!=NULL)
{
	fwrite(freeblockarr,1, strlen(freeblockarr), fileptr);
fclose(fileptr);
}
}
static struct super_operations SB_ops = {
	SB_read_inode,  /* read inode */
	NULL,			/* write inode */
	NULL,			/* write superblock */
	SB_statfs,		/* stat filesystem */
        };
/* SB_read_inode: Called from iget, it only traverses the allocated SB_inode_info's and initializes the inode from the data found there.  It does not allocate or deallocate anything. */

static void SB_read_inode(struct inode *inode)
{
        /* Our task should be extremely simple here. We only have tolook up the infomation somebody else (smb_iget) put into the inode tree. The address of this information is the            inode->i_ino. Just to make sure everything went well, we
check it's there. */

 if (SB_INOP(inode)->finfo.attr & aDIR)
 inode->i_mode = SMB_SERVER(inode)->m.dir_mode;
        
else
                
inode->i_mode = SMB_SERVER(inode)->m.file_mode;

 struct SB_inode_info *inode_info = (struct smb_inode_info *)(inode->i_ino);

 	 {  
	  inode->linkcnt   = 2;
        inode->i_uid     = 1;
        inode->i_gid     = 1;
        inode->i_size    = 0;
        inode->i_blksize = 1024;
        inode->i_size    = 0;
	   inode->i_mode = 16877;

        if ((inode->i_blksize != 0) && (inode->i_size != 0))
                inode->i_blocks =
                        (inode->i_size - 1) / inode->i_blksize + 1;
        else
        inode->i_blocks = 0;
        inode->i_mtime = 1323630836;
        inode->i_ctime = 1323630836;
        inode->i_atime = 1323630836;

}

static struct fuse_op ops = {
.getattr= fu_getattr,
.readdir= fu_readdir,
.open= fu_open,
.read= fu_read
};
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
/ Report errors to logfile and give -errno to caller
static int fu_error(char *str)
{
    int restat = -errno;
    
    log_msg("    ERROR %s: %s\n", str, strerror(errno));
    
    return restat;
}
static void fu_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, fu_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths //will break here

    log_msg("    fu_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
	    fu_DATA->rootdir, path, fpath);
}
/* Get file attributes. getattr()
 Similar to stat().  The 'st_dev' and 'st_blksize' fields are
  ignored.  The 'st_ino' field is ignored except if the 'use_ino'  mount option is given.
 */
int fu_getattr(const char *path, struct stat *statbuf)
{
    int restat = 0;
    char fpath[PATH_MAX];
memset(stbuf, 0, sizeof(struct stat));
if(strcmp(path, "/")== 0)
{
	stbuf-> st_mode= S_IFDIR |16877 ;
	stbuf-> st_linkcnt =2;
} else if(strcmp(path, PATH_MAX) == 0)
{
	stbuf-> st_mode= S_IFREG | 16877;
	stbuf-> st_linkcnt=4;
}
    else
	restat= -ENOENT;  // Path doesn't exist 
    log_msg("\nfu_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
    fu_fullpath(fpath, path);
    
    restat = lstat(fpath, statbuf);
    if (restat != 0)
	restat = fu_error("fu_getattr lstat");
    
    log_stat(statbuf);
    
    return restat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must //be one
// less than the size passed to fu_readlink()
// fu_readlink() code by Bernardo F Costa (thanks!)
int fu_readlink(const char *path, char *link, size_t size)
{
    int restat = 0;
    char fpath[PATH_MAX];
    
    log_msg("fu_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
	  path, link, size);
    fu_fullpath(fpath, path);
    
    retstat = readlink(fpath, link, size - 1);
    if (restat < 0)
	restat =fu_error("fu_readlink readlink");
    else  {
	link[restat] = '\0';
	restat = 0;
    }
    
    return restat;
}

/*Create a file node
  There is no create() operation, mknod() will be called for
  creation of all non-directory, non-symlink nodes.
 */

int fu_mknod(const char *path, mode_t mode, dev_t dev)
{
    int restat = 0;
    char fpath[PATH_MAX];
    
    log_msg("\nfu_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
	  path, mode, dev);
    fu_fullpath(fpath, path);
    
    // On Linux this could just be 'mknod(path, mode, rdev)' but //this is more portable
    if (S_ISREG(mode)) {
        restat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
	if (restat < 0)
	    restat = fu_error("fu_mknod open");
        else {
            restat = close(restat);
	    if (restat < 0)
		restat = fu_error("fu_mknod close");
	}
    } else
	if (S_ISFIFO(mode)) {
	    restat = mkfifo(fpath, mode);
	    if (restat < 0)
		restat = fu_error("fu_mknod mkfifo");
	} else {
	    restat = mknod(fpath, mode, dev);
	    if (restat < 0)
		restat = fu_error("fu_mknod mknod");
	}
    
    return restat;
}

/*Create a directory */
/* The mkdir() function shall create a new directory with name path. The file permission bits of the new directory shall be initialized from mode. These file permission bits of the mode argument shall be modified by the process' file creation mask.
int fu_mkdir(const char *path, mode_t mode) */

/* Upon successful completion, mkdir() shall mark for update the st_atime, st_ctime, and st_mtime fields of the directory. Also, the st_ctime and st_mtime fields of the directory that contains the new entry shall be marked for update.
*/ 
{
    int restat = 0;
    char fpath[PATH_MAX];
    
    log_msg("\nfu_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
    fu_fullpath(fpath, path);
    
    restat = mkdir(fpath, mode);
    if (restat < 0)
	restat = fu_error("fu_mkdir mkdir");
    
    return restat;
}

/* Remove a file */ 

/* Remove (delete) the given file, symbolic link, hard link, or special node. 
int fu_unlink(const char *path)
{
    int restat = 0;
    char fpath[PATH_MAX];
        log_msg("fu_unlink(path=\"%s\")\n",
	    path);
    fu_fullpath(fpath, path);
    
    restat = unlink(fpath);
    if (restat < 0)
	restat = fu_error("fu_unlink unlink");
    
    return restat;
}

/* Remove a directory */
/* Remove the given directory. This should succeed only if the directory is empty (except for "." and "..").  */ 
int fu_rmdir(const char *path)
{
    int restat = 0;
    char fpath[PATH_MAX];
    
    log_msg("fu_rmdir(path=\"%s\")\n",
	    path);
    fu_fullpath(fpath, path);
    
    restat = rmdir(fpath);
    if (restat < 0)
	restat = fu_error("fu_rmdir rmdir");
    
    return restat;
}

/* Read a directory */
//Read data from an open file

/* Read should return exactly the number of bytes requested except on EOF or error, otherwise the rest of the data will be substituted with zeroes. An exception to this is when the 'direct_io' mount option is specified, in which case the return value of the read system call will reflect the return value of this operation. */

int fu_read(const char * path, char *buf, size_t, fuse_fill_dir_t, off_t offset , struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path != 0)
	return -ENOENT;
len=strlen(buf);
if(offset < len) 
{
if( offset + size > len)
size= len- offset;
memcpy(buf, offset, size);
} else = 0;
return size;
}
static int fu_open(const char *path,  struct fuse_file_info *fi)
{
if(strcmp (path, fi->flags)!=0)
return -ENOENT;
if((fi->flags & 3) !=O_RDONLY)
return -EACCES;
return 0;
}

/* Return one or more directory entries (struct dirent) to the caller. This is one of the most complex FUSE functions.*/

static int fu_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
(void) offset;
(void) fi;
if(strcmp(path, "/") != 0)
return -ENOENT;
filler(buf, ".", NULL, 0);
filler(buf, ". .", NULL, 0);
return 0;
}
int fu_rename( const char *from, const char *to)
{
int res;
res= rename(from, to);
if(res==1)
return -errno;
return 0;
}

int main(int argc, char *argv[])
{
 return fuse_main(argc, argv, &ops, NULL );
}
