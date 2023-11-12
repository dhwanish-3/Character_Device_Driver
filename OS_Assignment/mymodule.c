/********//*
*  \file       mymodule.c
*
*  \details    Simple driver to complete the OS Assignment
*
*  \author     Dhwanish
*
* *********/
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>

char *kernel_version;
int timer = 0;
 
dev_t dev = 0;

module_param(timer, int, S_IRUSR|S_IWUSR);
module_param(kernel_version, charp, S_IRUSR|S_IWUSR);
 
/*
** Module init function
*/
static int __init mymodule_init(void)
{
    // check if the kernel version is same as that of the output of uname -r
    // if the kernel version is not same as system_version insmod should fail
    
    if (kernel_version[0] != '5' || kernel_version[2] != '1' || kernel_version[3] != '5') {
    	printk(KERN_ERR "Not same version");
    	return -1;
    }

    // Allocating Major number
    if((alloc_chrdev_region(&dev, 0, 1, "Mymodule")) < 0){
        printk(KERN_INFO "Cannot allocate major number for Mymodule\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    printk(KERN_INFO "value = %d  \n", timer);
    printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
    return 0;
}

/*
** Module Exit function
*/
static void __exit mymodule_exit(void)
{
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}
 
module_init(mymodule_init);
module_exit(mymodule_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dhwanish <dhwani333sh@gmail.com>");
MODULE_DESCRIPTION("A simple driver to demonstrate passing arguments to the module");
MODULE_VERSION("1.0");