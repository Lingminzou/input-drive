/*
 * input devic 
 *
 */
#include <linux/fs.h> 
#include <asm/uaccess.h> 
#include <linux/pci.h> 
#include <linux/input.h> 
#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/cdev.h>  
#include <linux/device.h> 

/* debug message --------------------------*/

#define pinfo(fmt, arg...)   printk("vms info: " fmt, ##arg)

#define pdebug(fmt, arg...)  //printk("vms debug: " fmt, ##arg)

#define pwarn(fmt, arg...)   printk("vms warn: " fmt, ##arg)

#define perr(fmt, arg...)    printk("vms err: " fmt, ##arg)


#define VMS_MAJOR  1  /*主设备号*/
#define VMS_MINOR  0  /*次设备号*/
#define VMS_COUNT  1  /*设备个数*/

#define CHECK_CODE 0xAA55 

typedef struct 
{
	unsigned char bit_left:1;
	unsigned char bit_right:1;
	unsigned char bit_middle:1;
	unsigned char bit3:1;
	unsigned char bit4:1;
	unsigned char bit5:1;
	unsigned char bit6:1;
	unsigned char bit7:1;
	int rel_x;
	int rel_y;
	int rel_wheel;
}mouse_report_data;

typedef struct
{
	unsigned int code;

	mouse_report_data data;
}udata_t;

static dev_t dev = 0;     /*设备号*/

static int vms_major = 0; /*存放主设备号*/
static int vms_minor = 0; /*存放次设备号*/

static struct cdev vms_cdev;  

static struct input_dev *vms_input_dev = NULL;

struct class *dev_class = NULL;   
struct device *dev_device = NULL; 

/*报告鼠标事件*/
int virtual_mouse_report(mouse_report_data* pdata)
{
	/*汇报情况*/
	//input_report_key(vms_input_dev, BTN_LEFT, pdata->bit_left);

	//input_report_key(vms_input_dev, BTN_RIGHT, pdata->bit_right);

	//input_report_key(vms_input_dev, BTN_MIDDLE, pdata->bit_middle);

	input_report_key(vms_input_dev, BTN_LEFT, *(char*)pdata & 0x01);

	input_report_key(vms_input_dev, BTN_RIGHT, *(char*)pdata & 0x02);

	input_report_key(vms_input_dev, BTN_MIDDLE, *(char*)pdata & 0x04);

	input_report_rel(vms_input_dev, REL_X, pdata->rel_x);

	input_report_rel(vms_input_dev, REL_Y, pdata->rel_y);

	input_report_rel(vms_input_dev, REL_WHEEL, pdata->rel_wheel);
	
	/*同步更新输入*/
	input_sync(vms_input_dev);

	return 0;
}

int vms_open(struct inode* inode, struct file *filp)  
{  
    pinfo("enter vms_open!\n"); 
    
    return 0;  
}  

ssize_t vms_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)  
{  
    pinfo("enter vms_read!\n");  
    
    return 0;  
}  
  
ssize_t vms_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)  
{
	int ret = 0;

	unsigned int *p = NULL;

	udata_t *udata_p = NULL;

	mouse_report_data *data_p = NULL;

	char data_buff[(sizeof(udata_t) * 2)] = {0};

	if(copy_from_user(data_buff, buf, count))
	{
		ret = -EFAULT;
	}

	p = (unsigned int*)data_buff;

	if(0xAA55 != *p)  /*对口号*/
	{
		pinfo("write err!\n");
		ret = -EFAULT;
	}

	udata_p = (udata_t*)data_buff;

	data_p = &(udata_p->data);

	pdebug("data_p->bit_left:%d\n", data_p->bit_left);
	pdebug("data_p->bit_middle:%d\n", data_p->bit_middle);
	pdebug("data_p->bit_right:%d\n", data_p->bit_right);
	pdebug("data_p->rel_wheel:%d\n", data_p->rel_wheel);

	pdebug("data_p->rel_x:%d\n", data_p->rel_x);
	pdebug("data_p->rel_y:%d\n", data_p->rel_y);

	ret = virtual_mouse_report(data_p);

    pdebug("write succeed!\n");  

    return 0;  
}    
  
int vms_release(struct inode *inode, struct file *filp)  
{  
    pinfo("enter vms_release!\n");  
    return 0;  
}  
  
struct file_operations vms_fops = 
{  
    .owner = THIS_MODULE,  
    .open = vms_open,  
    .read = vms_read,  
    .write = vms_write,  
    .release = vms_release,
}; 

/*注册输入设备*/
int virtual_mouse_register(void)
{
	/* Allocate an input device data structure */ 
	vms_input_dev = input_allocate_device(); /*注册为输入设备驱动程序*/
	
	if (!vms_input_dev) 
	{ 
		perr("Bad input_allocate_device()\n"); return ENOMEM;
	}
	
	/* Basics */
	vms_input_dev->evbit[0] |= BIT(EV_KEY) | BIT(EV_REL);

	/* Relative 'X' movement */
	set_bit(REL_X, vms_input_dev->relbit); /*声明虚拟鼠标产生的事件的编码*/
	/* Relative 'Y' movement */
	set_bit(REL_Y, vms_input_dev->relbit);

	set_bit(BTN_LEFT, vms_input_dev->keybit);
	set_bit(BTN_RIGHT, vms_input_dev->keybit);
	set_bit(BTN_MIDDLE, vms_input_dev->keybit);	

    vms_input_dev->name = "vms-zoulm";
	vms_input_dev->id.bustype = 0xAA;
	vms_input_dev->id.vendor = 0xDEAD;
	vms_input_dev->id.product = 0xBEEF;
	vms_input_dev->id.version = 0x0102;

	/* Register with the input subsystem */ 
	input_register_device(vms_input_dev);  /*注册*/
	
	pinfo("Virtual Mouse Driver Initialized.\r\n");

	return 0;
}	

/*模块加载初始化函数*/
int virtual_mouse_init(void)
{
	int ret = 0;  

	ret = virtual_mouse_register();

	if(ret != 0x00)
	{
		perr("virtual_mouse_register fail %d\r\n", ret);

		return ret;
	}

    if(vms_major)
    {  
        dev = MKDEV(VMS_MAJOR, VMS_MINOR);//生成设备号  
        //注册设备号;1、要注册的起始设备号2、连续注册的设备号个数3、名字  
        ret = register_chrdev_region(dev, VMS_COUNT, "vms");  
    }
    else
    {  
    
        // 动态分配设备号  
        ret = alloc_chrdev_region(&dev, vms_minor, VMS_COUNT, "vms");  
    }  
      
    if(ret < 0)
    {  
        perr("register_chrdev_region failed!\n");  
        goto failure_register_chrdev;  
    }
    
    vms_major = MAJOR(dev); /*得到主设备号*/
    
    pinfo("vms_major = %d\r\n", vms_major);  
  
    /*初始化cdev*/  
    cdev_init(&vms_cdev, &vms_fops);  
    /*添加cdev到内核*/  
    ret = cdev_add(&vms_cdev, dev, VMS_COUNT); 
    if(ret < 0)
    {  
        perr("cdev_add failed!\r\n");  
        goto failure_cdev_add;  
    }  
	
    /*自动创建设备节点文件*/  
    dev_class = class_create(THIS_MODULE, "vms_class"); /*1.注册设备类  /sys/class/vms_class的文件夹*/ 
    if(IS_ERR(dev_class)){  
        printk("class_create failed!\r\n");  
        ret = PTR_ERR("dev_class");  
        goto failure_class_create;  
    }

    /*2.注册设备  /sys/class/vms_class/vms0   /dev/vms0*/
    dev_device = device_create(dev_class, NULL, dev, NULL, "vms%d", vms_minor);  
    if(IS_ERR(dev_device)){  
        printk("device_create failed!\n");  
        ret = PTR_ERR(dev_device);  
        goto failure_device_create;  
    }
    
    return 0; 
    
failure_device_create:  
    class_destroy(dev_class);  
    
failure_class_create:  
    cdev_del(&vms_cdev);
		
failure_cdev_add:  
    unregister_chrdev_region(dev, VMS_COUNT);  
    
failure_register_chrdev:  
    return ret;
	
	return 0;
}

/*模块卸载函数*/
void virtual_mouse_exit(void)
{
    pinfo("virtual mouse exit!!\n");

	/* Unregister from the input subsystem */ 
	input_unregister_device(vms_input_dev); 

	//从内核中删除设备  
    device_destroy(dev_class, dev);  
    
    //从内核中删除设备类  
    class_destroy(dev_class);  
    
    //从内核中删除cdev  
    cdev_del(&vms_cdev);  
    
    //注销设备号  
    unregister_chrdev_region(dev, 1);  
	
	return;
}


MODULE_AUTHOR("zoulm 2016/11/11");
MODULE_LICENSE("GPL");

module_init(virtual_mouse_init); 
module_exit(virtual_mouse_exit);

