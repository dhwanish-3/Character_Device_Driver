/***************************************************************************//**
*  \file       file_operations.c
*
*  \details    Simple Linux device driver (File Operations)
*
*  \author     Dhwanish
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dhwanish_cdev;

/*
** Function Prototypes
*/
static int      __init dhwanish_driver_init(void);
static void     __exit dhwanish_driver_exit(void);
static int      dhwanish_open(struct inode *inode, struct file *file);
static int      dhwanish_release(struct inode *inode, struct file *file);
static ssize_t  dhwanish_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  dhwanish_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = dhwanish_read,
    .write      = dhwanish_write,
    .open       = dhwanish_open,
    .release    = dhwanish_release,
};

/*
** This function will be called when we open the Device file
*/
static int dhwanish_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int dhwanish_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t dhwanish_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Driver Read Function Called...!!!\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t dhwanish_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Driver Write Function Called...!!!\n");
        return len;
}

/*
** Module Init function
*/
static int __init dhwanish_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "Dhwanish")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&dhwanish_cdev,&fops);

        /*Adding character device to the system*/
        if((cdev_add(&dhwanish_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }

        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"dhwanish_class"))){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }

        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"dhwanish_device"))){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
      return 0;

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

/*
** Module exit function
*/
static void __exit dhwanish_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&dhwanish_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}

module_init(dhwanish_driver_init);
module_exit(dhwanish_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dhwanish <dhwani333sh@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (File Operations)");
MODULE_VERSION("1.3");