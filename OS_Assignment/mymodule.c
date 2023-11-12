/********//*
*  \file       mymodule.c
*
*  \details    Simple driver to complete the OS Assignment
*
*  \author     Dhwanish
*
* *********/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/unistd.h>
#include <linux/wait.h>
#include <linux/kthread.h>

/*
** Function Prototypes
*/
static int      __init mymodule_init(void);
static void     __exit mymodule_exit(void);
static int      mymodule_open(struct inode *inode, struct file *file);
static int      mymodule_release(struct inode *inode, struct file *file);
static ssize_t  mymodule_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  mymodule_write(struct file *filp, const char *buf, size_t len, loff_t * off);

// defining parameters
char *kernel_version;
int timer = 0;
volatile unsigned long start_time = 0;
int read_first = 0;
int write_second = 0;
volatile unsigned long read_time;
volatile unsigned long write_time;
char username[10];

int wait_queue_flag = 0;
static struct task_struct *wait_thread;
DECLARE_WAIT_QUEUE_HEAD(wait_queue_mymodule);

dev_t dev = 0;
char device_buffer[100];

module_param(timer, int, S_IRUSR|S_IWUSR);
module_param(kernel_version, charp, S_IRUSR|S_IWUSR);

static struct cdev mymodule_dev;
static struct class *mymodule_class;

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = mymodule_read,
    .write      = mymodule_write,
    .open       = mymodule_open,
    .release    = mymodule_release,
};

static int mymodule_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Driver Open Function Called...!!!\n");
    return 0;
}

static int mymodule_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Driver Release Function Called...!!!\n");
    return 0;
}

static ssize_t mymodule_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver Read Function Called...!!!\n");
    if (*off >= sizeof(device_buffer)) {
        return 0;
    }
    size_t bytes_to_read = min(len, sizeof(device_buffer) - *off);
    if (copy_to_user(buf, device_buffer + *off, bytes_to_read) != 0) {
        return -EFAULT;
    }
    *off += bytes_to_read;
    wait_queue_flag = 1;
    wake_up_interruptible(&wait_queue_mymodule);
    return bytes_to_read;
}

static ssize_t mymodule_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver Write Function Called...!!!\n");
    if (len > sizeof(device_buffer)) {
        return -EINVAL;
    }
    if (copy_from_user(device_buffer + *off, buf, len) != 0) {
        return -EFAULT;
    }
    wait_queue_flag = 2;
    wake_up_interruptible(&wait_queue_mymodule);
    printk(KERN_INFO "device_buffer = %s\n", device_buffer);
    int i = 0;
    while (i < 10 && device_buffer[i] != '\0') {
        username[i] = device_buffer[i];
        i++;
    }
    return len;
}

/*
** Thread function
*/
static int wait_function(void *unused)
{
    while(3) {
        pr_info("Waiting For Event...\n");
        wait_event_interruptible(wait_queue_mymodule, wait_queue_flag != 0 );
        if(wait_queue_flag == 3) {
            pr_info("Event Came From Exit Function\n");
            return 0;
        } else if (wait_queue_flag == 1) {
            pr_info("Event Came From Read Function - %d\n", ++read_first);
            read_time = jiffies;
        } else if (wait_queue_flag == 2 && read_first == 1) {
            write_time = jiffies;
            pr_info("Event Came From Write Function - %d\n", ++write_second);
        }
        wait_queue_flag = 0;
    }
    do_exit(0);
    return 0;
}
 
/*
** Module init function
*/
static int __init mymodule_init(void)
{
    // if the kernel version is not same as the version of compilatio, insmod should fail
    if (kernel_version[0] != '5' || kernel_version[1] != '.' || kernel_version[2] != '1' || kernel_version[3] != '5') {
    	printk(KERN_ERR "Kernel version is not same as the version of compilation\n");
    	return -1;
    }

    // Allocating Major number
    if((alloc_chrdev_region(&dev, 0, 1, "Mymodule")) < 0){
        printk(KERN_ERR "Cannot allocate major number for Mymodule\n");
        return -1;
    }

    // Creating cdev structure
    cdev_init(&mymodule_dev, &fops);

    // Adding character device to the system
    if((cdev_add(&mymodule_dev, dev, 1)) < 0){
        printk(KERN_ERR "Cannot add the device to the system\n");
        goto r_class;
    }

    // Creating struct class
    if (IS_ERR(mymodule_class = class_create(THIS_MODULE, "mymodule_class"))) {
        printk(KERN_ERR "Cannot create the struct class for Mymodule\n");
        goto r_device;
    }

    // Creating device
    if (IS_ERR(device_create(mymodule_class, NULL, dev, NULL, "mymodule_device"))) {
        printk(KERN_ERR "Cannot create the Device 1\n");
        goto r_class;
    }
    // Create the kernel thread with name 'mythread'
    wait_thread = kthread_create(wait_function, NULL, "WaitThread");
    if (wait_thread) {
        printk(KERN_INFO "Thread Created successfully\n");
        wake_up_process(wait_thread);
    } else {
        printk(KERN_ERR "Thread creation failed\n");
    }
    start_time = jiffies;
    printk(KERN_INFO "Major = %d Minor = %d\n",MAJOR(dev), MINOR(dev));
    printk(KERN_INFO "value = %d\n", timer);
    printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
    return 0;
r_device:
    class_destroy(mymodule_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

/*
** Module Exit function
*/
static void __exit mymodule_exit(void)
{
    wait_queue_flag = 3;
    wake_up_interruptible(&wait_queue_mymodule);
    kthread_stop(wait_thread);
    // printing current time
    printk(KERN_INFO "Current time is %ld\n", jiffies_to_clock_t(jiffies));
    printk(KERN_INFO "Read time is %ld\n", read_time);
    printk(KERN_INFO "Write time is %ld\n", write_time);
    device_destroy(mymodule_class, dev);
    class_destroy(mymodule_class);
    cdev_del(&mymodule_dev);
    unregister_chrdev_region(dev, 1);
    unsigned long timer_value = msecs_to_jiffies(timer * 1000);
    if (read_first == 1 && write_second == 1 && time_after(start_time + timer_value, write_time) && time_after(write_time, read_time)) {
        printk(KERN_INFO "Successfully completed the actions within time , username = %s\n, ", username);
    } else {
        printk(KERN_INFO "Failed to complete the tasks withing the time\n");
    }
    printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}
 
module_init(mymodule_init);
module_exit(mymodule_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dhwanish <dhwani333sh@gmail.com>");
MODULE_DESCRIPTION("A simple driver to demonstrate passing arguments to the module");
MODULE_VERSION("1.0");