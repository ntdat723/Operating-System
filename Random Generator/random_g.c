#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/random.h>


static dev_t first;																					//dev_t type is used to hold device number (both major and minor number)
static struct cdev c_dev;
static struct class *cl;
static int randomNumber;																			//created to hold the random generated value, as its name suggests

static int random_open(struct inode *i, struct file *f)												//one of the desired operations for character device file. T
{
	printk(KERN_INFO "Opened.\n");
	return 0;
}



static int random_close(struct inode *i, struct file *f)											
{
	printk(KERN_INFO "Closed.\n");
	return 0;
}


//Read operation, calling by cat command. In this case, it will return an infinite sequence of random integer from 0 to 9.
static ssize_t random_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	char afterCast;																					//a variable of type char to hold the value of the random-generated number
	get_random_bytes(&randomNumber, 1);																//get_random_bytes() returns the requested number of random bytes and stores them in the buffer
	while (randomNumber < 0 || randomNumber >= 10)													//while loop to check if the random-generated value is in range from 0 to 9
		get_random_bytes(&randomNumber, 1);																//if not, keep performing get_random_bytes() until it is
	afterCast = randomNumber + '0';																	//Casting an integer to char type
	if (copy_to_user(buf, &afterCast, 1))															//copy_to_user() copies data from kernel space to user space. Return 0 on success
		return -EFAULT;
	else
		return 1;
}


//Write operation, calling by echo command. In this case, it just simply prints to kernel log because we don't initialize any interactions with it.
static ssize_t random_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: write()\n");
	return len;
}


//This structure holds pointers to functions defined by the driver that perform various operations. In this case, there are basic ones like read, release (close), write, open.
static struct file_operations pugs_fops = 
{
	.owner = THIS_MODULE,
	.open = random_open,
	.release = random_close,
	.read = random_read,
	.write = random_write
};


static int __init random_init(void)
{
	printk(KERN_INFO "The random number driver has been registered successfully.\n");
	
	//Register a range of character device number for MyRandom character device. Return a negative number if failed.
	if (alloc_chrdev_region(&first, 0, 1, "MyRandom") < 0)
		return -1;
	
	//Create a device class with the name of random_gen
	if ((cl = class_create(THIS_MODULE, "random_gen")) == NULL)
	{
		unregister_chrdev_region(first, 1);																			//unregister a range of device numbers
		return -1;
	}

	//Create the myrandomgen device and register it with the assigned major number
	if (device_create(cl, NULL, first, NULL, "myrandomgen")
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	//Initialize a character device structure with the mentioned set of operations
	cdev_init(&c_dev, &pugs_fops);
	
	//cdev_add() adds the character device to the system
	if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	return 0;
}

static void __exit random_exit(void)
{
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "The driver has been successfully unregistered!\n");
}

module_init(random_init);																		//Determine that random_init() will be the first one to be called right after installing the module
module_exit(random_exit);																		//Determine that random_exit() will be the last one to be called right before uninstalling the module

MODULE_LICENSE("GPL");
MODULE_AUTHOR("18125126");
MODULE_DESCRIPTION("Random character Driver");
