#include "filesys.h"
#include "system_call.h"

static filesys file_sys;
static dentry_t file_dentry;
static dentry_t directory_dentry;

/*
 * file_system_init
 *  DESC: initializes structs associated with the filesystem
 *  INPUT: start_addr - ptr to the start of the filesystem (magic ptr given to us)
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: initializes file_sys, file_dentry, directory_dentry
*/
void file_system_init(void* start_addr){
    /* initialize file_sys struct */
    file_sys.boot_block_pointer = (boot_block*)start_addr;
    file_sys.inode_pointer = (inode_t*)(start_addr) + 1;
    file_sys.data_block_pointer = (data_block*)(file_sys.inode_pointer) + (file_sys.boot_block_pointer -> num_inodes);

    /* init file_dentry and directory_dentry */
    file_dentry.name[0] = '\0';
    file_dentry.inode_num = -1;
    file_dentry.type = -1;

    directory_dentry.name[0] = '\0';
    directory_dentry.inode_num = -1;
    directory_dentry.type = -1;
}



/*
 * open_file
 *  DESC: loads file info into the static file_dentry struct
 *        Basically an API call to read_dentry_by_name
 *  INPUT: filename - name of the file to open
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: none
*/
int32_t open_file(const uint8_t* filename, int32_t fd){
    int error = read_dentry_by_name(filename, &(curr_pcb->fd_table[fd].fd_dentry));
    if (error) return -1;
    return 0;
}

/*
 * open_file_execute
 *  DESC: loads file info into the static file_dentry struct
 *        Basically an API call to read_dentry_by_name
 *  INPUT: filename - name of the file to open
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: none
*/
int32_t open_file_execute(const uint8_t* filename) {
    int error = read_dentry_by_name(filename, &file_dentry);
    if (error) return -1;

    return 0;
}

/*
 * close_file
 *  DESC: clears the file_dentry struct - basically opposite of open
 *  INPUT: fd - index in the file descriptor table
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success
 *  SIDE EFFECTS: none
*/
int32_t close_file(int32_t fd){
    *(curr_pcb->fd_table[fd].fd_dentry.name) = '\0';
    curr_pcb->fd_table[fd].fd_dentry.type = -1;
    curr_pcb->fd_table[fd].fd_dentry.inode_num = -1;
    return 0;
}

/*
 * read_file
 *  DESC: reads data in a file, basically an API call to read_data
 *  INPUTS: fd - index in the file descriptor table
 *          buf - a buffer to write the data to
 *          nbytes - number of bytes to read
 *  OUTPUT: an int for error checking
 *  RETURN VALUE: 0 for success, -1 for error
 *  SIDE EFFECTS: offset to read from in file changes
*/
int32_t read_file(int32_t fd, void* buf, int32_t nbytes){
    int result;
    int file_size = (file_sys.inode_pointer+curr_pcb->fd_table[fd].fd_dentry.inode_num)->length;
    if (curr_pcb->fd_table[fd].offset == file_size) return 0;
    result = read_data(curr_pcb->fd_table[fd].fd_dentry.inode_num, curr_pcb->fd_table[fd].offset , buf, nbytes);
    if (result == -1) {
        return -1;
    } else if (result == -2) {
        return -1;
    }
    curr_pcb->fd_table[fd].offset = curr_pcb->fd_table[fd].offset + result;
    return result;
}

/*
 * read_file_execute
 *  DESC: reads data in a file, basically an API call to read_data
 *        (use this one for starting a new process)
 *  INPUTS: buf - a buffer to write the data to
 *          nbytes - number of bytes to read
 *  OUTPUT: an int for error checking
 *  RETURN VALUE: 0 for success, -1 for error
 *  SIDE EFFECTS: offset to read from in file changes
*/
int32_t read_file_execute(void* buf, int32_t nbytes) {
    int result;
    result = read_data(file_dentry.inode_num, 0, buf, nbytes);
    if (result < 0) return -1;
    return 0;
}



/*
 * write_file
 *  DESC: right now this does nothing (read-only filesystem)
 *  INPUTS: fd - index in file descriptor array
 *          buf - a buffer
 *          nbytes - number of bytes
 *  OUTPUTS: an int for error checking
 *  RETURN VALUE: -1 because we can't write to a file
 *  SIDE EFFECTS: none
*/
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*
 * open_dir
 *  DESC: loads directory info into the static directory_dentry struct
 *        Basically an API call to read_dentry_by_name
 *  INPUT: filename - name of the directory to open
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: none
*/
int32_t open_dir(const uint8_t* filename, int32_t fd){
    int result = read_dentry_by_name(filename, &(curr_pcb->fd_table[fd].fd_dentry));
    if (result) return -1;
    return 0;
}

/*
 * close_dir
 *  DESC: clears the directory_dentry struct - basically opposite of open
 *  INPUT: fd - index in the file descriptor table
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success
 *  SIDE EFFECTS: none
*/
int32_t close_dir(int32_t fd){
    *(curr_pcb->fd_table[fd].fd_dentry.name) = '\0';
    curr_pcb->fd_table[fd].fd_dentry.type = -1;
    curr_pcb->fd_table[fd].fd_dentry.inode_num = -1;
    return 0;
}

/*
 * read_dir
 *  DESC: this returns the name of a file
 *  INPUTS: fd - index in the file descriptor table
 *          buf - a buffer to write into
 *          nbytes - number of bytes to read
 *  OUTPUT: an int for errors and sizeof name
 *  RETURN VALUE: 0 for done, -1 for error, other for the length of name returned in buf
 *  SIDE EFFECTS: none
*/
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes){
    // basically make a buffer one bigger than max size so it's a null terminated string
    uint8_t name[NAME_LENGTH+1];
    memset((void*)name, (int32_t)'\0', NAME_LENGTH+1);
    uint32_t total_dentries = file_sys.boot_block_pointer->num_dentries;

    // use the offset as an offset into the directory instead of the filesys, jank but whatever
    // if this happens then we are done
    if (curr_pcb->fd_table[fd].offset > total_dentries) {
        return 0;
    }
    
    (void)read_dentry_by_index(curr_pcb->fd_table[fd].offset, &(curr_pcb->fd_table[fd].fd_dentry));

    // copy to name because it's automatically null temrinated (by hardcoding lol)
    memcpy(name, (curr_pcb->fd_table[fd].fd_dentry).name, NAME_LENGTH);

    // copy the NULL TERMINATED STRING into buf
    strcpy((int8_t*)buf, (int8_t*)name);
    
    // go to next file
    curr_pcb->fd_table[fd].offset++;

    return strlen((int8_t*)name);
}

/*
 * write_dir
 *  DESC: right now this does nothing (read-only filesystem)
 *  INPUTS: fd - index in file descriptor array
 *          buf - a buffer
 *          nbytes - number of bytes
 *  OUTPUTS: an int for error checking
 *  RETURN VALUE: -1 because we can't write to a file
 *  SIDE EFFECTS: none
*/
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


/* dentry/data operations */

/*
 * read_dentry_by_name
 *  DESC: given a name, this fn finds the matching dentry
 *        and copies that dentry to a pointer passed in
 *  INPUTS: fname - file name
 *          dentry - pointer to an empty dentry to populate
 *  OUTPUT: an int indicating success/failure
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: populates the dentry via the ptr passed in
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int i;
    int target_index;
    int same_string_flag;

    // check invalid pointers
    if(fname == NULL || dentry == NULL){
        return -1;
    }

    // check if name is too long
    if(strlen((int8_t*)fname) > NAME_LENGTH){
        return -1;
    }
    

    target_index = -1;
    // iterate through the dentries and compare the strings to see if it matches
    for(i = 0; i < DENTRIES_NUM; i++){
        if(strlen((int8_t*)fname) > strlen((int8_t*)file_sys.boot_block_pointer -> dentries[i].name)){
            same_string_flag = strncmp((int8_t*)fname, (int8_t*)file_sys.boot_block_pointer -> dentries[i].name, strlen((int8_t*)fname));
        }else if (strlen((int8_t*)fname) == NAME_LENGTH){
            same_string_flag = strncmp((int8_t*)fname, (int8_t*)file_sys.boot_block_pointer -> dentries[i].name, strlen((int8_t*)fname));
        }else{
            same_string_flag = strncmp((int8_t*)fname, (int8_t*)file_sys.boot_block_pointer -> dentries[i].name, strlen((int8_t*)file_sys.boot_block_pointer -> dentries[i].name));
        }
        if(same_string_flag == 0){
            target_index = i;
            break;
        }
    }
    if(same_string_flag != 0){
        return -1;
    }
    if(target_index == -1){
        return -1;
    }
    
    // populate the dentry passed in 
    // just note that this will copy the entirety of the string (32 chars) to the new dentry whether or not they are all used
    memcpy(dentry->name, file_sys.boot_block_pointer->dentries[target_index].name, sizeof(dentry->name));
    dentry -> type = (file_sys.boot_block_pointer -> dentries[target_index]).type;
    dentry -> inode_num = (file_sys.boot_block_pointer -> dentries[target_index]).inode_num;

    return 0;

}

/*
 * read_dentry_by_index
 *  DESC: given an index, this fn finds the matching dentry
 *        and copies that dentry to a pointer passed in
 *  INPUTS: index - an index in the boot block (NOT INODE NUMBER)
 *          dentry - pointer to an empty dentry to populate
 *  OUTPUT: an int indicating success/failure
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: populates the dentry via the ptr passed in
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    // check invalid pointer
    if(dentry == NULL) return -1;

    // check if name is too long
    if(index > DENTRIES_NUM || index < 0) return -1;

    // populate the dentry passed in 
    // just note that this will copy the entirety of the string (32 chars) to the new dentry whether or not they are all used
    memcpy(dentry->name, file_sys.boot_block_pointer->dentries[index].name, sizeof(dentry->name));
    dentry -> type = (file_sys.boot_block_pointer -> dentries[index]).type;
    dentry -> inode_num = (file_sys.boot_block_pointer -> dentries[index]).inode_num;

    return 0;
}

/*
 * read_data
 *  DESC: This copies file data to a buffer, but starts at a given offset
 *  INPUTS: inode - the inode of the file
 *          offset - the offset (in bytes in respect to the start of the file) to begin reading from
 *          buf - a string buffer to populate
 *          length - how many bytes to read
 *  OUTPUT: an int indicating success/failure
 *  RETURN VAL: number of bytes read for success, -1/-2 for error
 *  SIDE EFFECTS: populates the string buffer passed in
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

    int32_t num_total_inodes = file_sys.boot_block_pointer->num_inodes;    // total inodes in file system
    inode_t* file_inode = file_sys.inode_pointer+inode;                     // current file's inode

    int32_t data_blk_idx_global;                            // index of data block in the entire file system
    int32_t data_blk_idx_inode = offset / BLOCK_SIZE;       // index of the curr data blk in the inode
    int32_t offset_in_blk = offset % BLOCK_SIZE;            // offset in the curr data blk (where to start reading)
    data_block* data_blk_ptr;                  // ptr to the current data block
    int32_t offset_in_new_blk = 0;            // used for accessing multiple pages for a file
    int32_t i;                                // loop index

    /* check if inode is valid*/
    if (inode >= num_total_inodes || inode < 0) return -2;

    /* check if input length is greater than length of file */
    if((file_inode->length  <= data_blk_idx_inode) && inode != 0) {
            return -1;
    }
    
    // iterate through length amout of bytes
    for(i = 0; i < length; i++){
        if (offset + i >= file_inode->length) {
            return i;
        }
        
        /* get the global index of the data block in the file system */
        data_blk_idx_global = file_inode->data_blocks[data_blk_idx_inode];

        /* get a pointer to the data block */
        data_blk_ptr = file_sys.data_block_pointer + data_blk_idx_global;

        // these two need to be in the for loop to handle the case where we access multiple files

        // this gives you the which data block, it is in a form of int, a index to 
        buf[i] = data_blk_ptr->data[offset_in_blk + i + offset_in_new_blk];

        if(offset_in_blk + i + offset_in_new_blk == BLOCK_SIZE-1){
            data_blk_idx_inode++;
            // make offset_in_data_block + i = 0
            offset_in_new_blk -= BLOCK_SIZE; // subtract 4096 because i increments after this line, need to be ==0
        }
    }
    return i;
}

