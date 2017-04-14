#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */ 
#define SCULL_IOC_MAGIC 'k'
#define SCULL_IOC_MAXNR 14
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_WRITE _IOW(SCULL_IOC_MAGIC, 2, char*)
#define SCULL_READ _IOR(SCULL_IOC_MAGIC, 3, char*)
#define SCULL_EDIT _IOWR(SCULL_IOC_MAGIC, 4, char*)
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
long ioctl_example(struct file *filp, unsigned int cmd, unsigned long arg); 
static void onebyte_exit(void);
/* definition of file_operation structure */
struct file_operations onebyte_fops = {
     read:     onebyte_read,
     write:    onebyte_write,
     open:     onebyte_open,
     release: onebyte_release,
     llseek: onebyte_lseek,
     unlocked_ioctl: ioctl_example
};
char *onebyte_data = NULL;
char *dev_meg = NULL;
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

long ioctl_example(struct file *filp, unsigned int cmd, unsigned long arg) {
    int err = 0, tmp, i;
    int retval = 0;
    /*
    * extract the type and number bitfields, and don't decode
    * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok() */
    if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;
    /*
    * the direction is a bitmask, and VERIFY_WRITE catches R/W * transfers. `Type' is user‐oriented, while
    * access_ok is kernel‐oriented, so the concept of "read" and * "write" is reversed
    */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;
    char *buf = (char *)arg;
    switch(cmd) {
        case SCULL_HELLO:
            printk(KERN_WARNING "hello\n");
            break;
        case SCULL_WRITE: 
            // the msg has fixed length of 10.
            copy_from_user(dev_meg, buf, 10); 
            retval = 10;
            break;
        case SCULL_READ: 
            // the msg has fixed length of 10.
            copy_to_user(buf, dev_meg, 10); 
            retval = 10;
            break;
        case SCULL_EDIT: 
            // the msg has fixed length of 10.
            printk(KERN_WARNING "Old dev_meg: %s\n", dev_meg);
            char temp[11];
            copy_from_user(temp, buf, 10);
            copy_to_user(buf, dev_meg, 10); 
            for (i = 0; i < 10; i++) dev_meg[i] = temp[i];
            printk(KERN_WARNING "New dev_meg: %s\n", dev_meg);
            retval = 10;
            break;
        default: /* redundant, as cmd was checked against MAXNR */ return -ENOTTY;
    }
    return retval;
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
     dev_meg = kmalloc(sizeof(char) * 10, GFP_KERNEL);
     if (!onebyte_data || !dev_meg) {
          onebyte_exit();
          // cannot allocate memory
          // return no memory error, negative signify a failure
         return -ENOMEM;
     }        
     dev_meg[10] = 0;
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
