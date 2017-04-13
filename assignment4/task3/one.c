#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#define MAJOR_NUMBER 61
 
long long int length = 1;
size_t pos_buf = 0;
/* forward declaration */
int onebyte_open(struct inode *inode, struct file *filep);
int onebyte_release(struct inode *inode, struct file *filep);
ssize_t onebyte_read(struct file *filep, char *buf, size_t
count, loff_t *f_pos);
ssize_t onebyte_write(struct file *filep, const char *buf,
size_t count, loff_t *f_pos);
loff_t onebyte_lseek(struct file *file, loff_t offset, int whence);
static void onebyte_exit(void);
/* definition of file_operation structure */
struct file_operations onebyte_fops = {
     read:     onebyte_read,
     write:    onebyte_write,
     open:     onebyte_open,
     release: onebyte_release,
     llseek: onebyte_lseek
};
char *onebyte_data = NULL;
int onebyte_open(struct inode *inode, struct file *filep)
{
     return 0; // always successful
}
int onebyte_release(struct inode *inode, struct file *filep)
{
     return 0; // always successful
}
loff_t onebyte_lseek(struct file *filep, loff_t offset, int whence) {
    int off = (int)offset;
    loff_t pos_new;
    printk(KERN_ALERT "offset = %lld\n", offset);
    if (whence == 0) {
        // whence == SEEK_SET
        pos_new = offset;
    } else if (whence == 1) {
        // whence == SEEK_CUR
        pos_new = filep->f_pos + offset;
    } else if (whence == 2) {
        // whence == SEEK_END
        pos_new = length + off;
    } else {
        // Error.
        pos_new = -1;
    }
    if (pos_new < 0) pos_new = 0;
    if (pos_new > length) pos_new = length;
    filep->f_pos = pos_new;
    return pos_new;
}
ssize_t onebyte_read(struct file *filep, char *buf, size_t
count, loff_t *f_pos)
{
    int t = 0;
    if (onebyte_data != NULL) { 
    	if (count + (*f_pos) <= length) {
      	    copy_to_user(buf, onebyte_data + (int)(*f_pos), count);
            (*f_pos) += count;
            return count;
        } else if ((*f_pos) < length) {
            t = (int)(length - (int)(*f_pos));
            copy_to_user(buf, onebyte_data + (int)(*f_pos), t); 
            (*f_pos) = length;
            return t;
    	} else {
	        return 0;
	    }
    } else {
        return 0;
    }
}
ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
    int t;
    if (buf != NULL) {
        if (count > 4 * 1024 * 1024) count = 4 * 1024 * 1024;
        if ((*f_pos) + count <= 4 * 1024 * 1024) {
            copy_from_user(onebyte_data + (int)(*f_pos), buf, count);
            (*f_pos) += count;
            length = (*f_pos) > length ? (*f_pos) : length;
            printk(KERN_ALERT "bytes writen = %lu\n", count);
            return count;
        } else if ((*f_pos) < 4 * 1024 * 1024) {
            t = (int)(4 * 1024 * 1024 - (int)(*f_pos));
            copy_from_user(onebyte_data + (int)(*f_pos), buf, t); 
            (*f_pos) = 4 * 1024 * 1024;
            length = (*f_pos) > length ? (*f_pos) : length;
            printk(KERN_ALERT "bytes writen = %d\n", t);
            return t;
        } else {
            printk(KERN_ALERT "ENTERED");
            return -ENOSPC;
        } 
    } else {
        return -EFAULT;
    }
}
static int onebyte_init(void)
{
     int result;
     // register the device
     result = register_chrdev(MAJOR_NUMBER, "one",
&onebyte_fops);
     if (result < 0) {
         return result;
     }
     // allocate one byte of memory for storage
     // kmalloc is just like malloc, the second parameter is
// the type of memory to be allocated.
     // To release the memory allocated by kmalloc, use kfree.
     onebyte_data = kmalloc(sizeof(char) * 1024 * 1024 * 4, GFP_KERNEL);
     if (!onebyte_data) {
          onebyte_exit();
          // cannot allocate memory
          // return no memory error, negative signify a failure
         return -ENOMEM;
     }        
     // initialize the value to be X
     *onebyte_data = 'X';
     printk(KERN_ALERT "This is a 4MB device module\n");
     return 0;
}
static void onebyte_exit(void)
{
     // if the pointer is pointing to something
     if (onebyte_data) {
          // free the memory and assign the pointer to NULL
          kfree(onebyte_data);
          onebyte_data = NULL;
     }         // unregister the device
     unregister_chrdev(MAJOR_NUMBER, "one");
     printk(KERN_ALERT "4MB device module is unloaded\n");
}
MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);
