#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

#define BUFFER_SIZE 1024

#define MAJOR_NUMBER 240
char *device_buffer;

static int count_open = 0;
static int count_close = 0; 

MODULE_AUTHOR("Cal Brynestad");
MODULE_LICENSE("GPL");

/* Define device_buffer and other global data structures you will need here */

ssize_t pa2_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want
	to be read from the device file*/
	/* length is the length of the userspace buffer, or how many bytes are already in it*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the 
	userspace buffer *buffer */

	/* offset is the current position in the device buffer */
	/* length is the number of bytes the user inputs in test program that they want to read */
	

	int end_of_buf = *offset + length;
	if (end_of_buf > BUFFER_SIZE) {
		length = BUFFER_SIZE - *offset;
	}

	if (*offset < 0) {
		printk("Can't read before beginning of file");
		return 0;
	}

	/* buffer is destination and device_buffer is source. For example first read, offset is 0 so we read from the beginning of device buffer, and then from where we left off on next read because we add offset */
	copy_to_user(buffer, device_buffer + *offset, length);
	*offset = *offset + length;

	printk("Number of bytes read: %zu\n", length);
	return length;
}

ssize_t pa2_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to
	be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the 
	userspace buffer *buffer */

	/* offset is offset of device buffer
	length is length of data string to be written */

	int end_of_buf = *offset + length;
	if (end_of_buf > BUFFER_SIZE) {
		length = BUFFER_SIZE - *offset;
	}

	if (*offset < 0) {
		printk("Can't write to where the file doesn't exist");
		return 0;
	}

	/* device_buffer is destination and buffer is source. */
	copy_from_user(device_buffer + *offset, buffer, length);
	*offset = *offset + length;

	printk("Number of bytes written: %zu\n", length);
	//printk("%s\n", device_buffer);

	return length;
}

int pa2_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number 
	of times this device has been opened until now*/
	printk("PA2 char driver is opened");
	count_open += 1;
	printk("Number of times device has been opened: %d\n", count_open);
	

	return 0;
}

int pa2_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number 
	of times this device has been closed until now*/
	printk("PA2 char driver is closed");
	count_close += 1;
	printk("Number of times device has been closed until now: %d\n", count_close);

	return 0;
}

loff_t pa2_char_driver_seek (struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */
	loff_t dev_pointer = 0;

	if (whence == 0){
		dev_pointer = offset;
	}

	if (whence == 1){
		dev_pointer = pfile->f_pos + offset;
	}

	if (whence == 2){
		dev_pointer = BUFFER_SIZE - 1 + offset;
	}

	if (dev_pointer < 0 || dev_pointer > BUFFER_SIZE - 1){
		printk("ERROR: Attempting to seek before the beginning or beyond the end of the device buffer");
		return -1;
	}

	pfile->f_pos = dev_pointer;

	return dev_pointer;
}

struct file_operations pa2_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	/* add the function pointers to point to the corresponding file operations. 
	look at the file fs.h in the linux souce code*/

	.open   = pa2_char_driver_open,       // int my_open  (struct inode *, struct file *);
	.release = pa2_char_driver_close,      // int my_close (struct inode *, struct file *);
	.read    = pa2_char_driver_read,       // ssize_t my_read  (struct file *, char __user *, size_t, loff_t *);
	.write   = pa2_char_driver_write,      // ssize_t my_write (struct file *, const char __user *, size_t, loff_t *);
	.llseek  = pa2_char_driver_seek        // loff_t  my_seek  (struct file *, loff_t, int);
};

static int pa2_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	/* register the device */
	register_chrdev(MAJOR_NUMBER, "simple_character_device", &pa2_char_driver_file_operations);
	printk(KERN_ALERT "Init function is called for PA2 char driver\n");
	device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	memset(device_buffer,0,BUFFER_SIZE);

	return 0;
}

static void pa2_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the register_chrdev() function. */
	unregister_chrdev(MAJOR_NUMBER, "simple_character_device");
	printk(KERN_ALERT "Exit function is called for PA2 char driver\n");
	kfree(device_buffer);
}

/* add module_init and module_exit to point to the corresponding init and exit 
function*/

module_init(pa2_char_driver_init);

module_exit(pa2_char_driver_exit);
