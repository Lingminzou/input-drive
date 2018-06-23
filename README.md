
## 一、前言

这个东西我做出来已经过去很久了，一直躺在我的电脑里面，今天把它拿出来分享，少了刚开始做出来时的那份激情突然不知道怎么来写。

废话少说，先上最终的效果视频：

然而这有什么用呢。。。。哈哈哈。。。

理解掌握它可以丰富系统的输入外设类型，让输入设备不仅限于传统的鼠标和键盘。

空鼠就是一个很好的例子，空鼠拿在手上在空中晃动就能控制指针的移动，像是拿着指挥棒指挥指针一样。

然后你可以发挥你的奇思妙想创造各种奇异的外设来控制我们的系统，是不是一件很有趣的事情，哈哈哈。。。

## 二、从输入设备说起

（当然这里讨论的是基于 Linux 系统的输入设备，其他 OS 不在本文讨论范围）

输入外设在 Linux 中是接入到输入子系统（input subsystem）中，它从上到下由三层实现，分别为：

 - 输入子系统事件处理层（EventHandler）；
 - 输入子系统核心层（InputCore）；
 - 输入子系统设备驱动层（Driver）。

事件处理层：它是用户编程的接口（设备节点），并处理驱动层提交的数据处理。

核心层：它为设备驱动层提供了规范和接口。让设备驱动层只要关心如何驱动硬件并获得硬件数据（例如按下的按键数据），然后调用核心层提供的接口，核心层会自动把数据提交给事件处理层。

设备驱动层：主要实现对硬件设备的读写访问，中断设置，并把硬件产生的事件转换为核心层定义的规范并由核心层提交给事件处理层。

以上通常情况下我们能做的工作都是在设备驱动层，如给系统增加一个输入外设等。

下面引用一张图来更好的理解它们。

![](http://wx2.sinaimg.cn/mw690/9e169b75gy1fskz2fp4njj20fk0cw74r.jpg)

通常一个输入设备驱动会经历分配输入设备、注册输入设备、提交输入事件三个步骤，对应的系统函数如下：

	// 分配
	struct input_dev *input_allocate_device(void);

	// 注册
	int input_register_device(struct input_dev *dev);

	// 提交
	void input_event(struct input_dev *dev,
		 unsigned int type, unsigned int code, int value);

注：分配和注册通常在模块加载初始化函数中完成，事件提交通常在设备驱动的中断函数中完成。

理解一个输入设备驱动抓住上面三个点就好了。

## 三、如何接入手机触摸屏到系统

对 Linux 输入子系统有一定了解了，那下面我们开始进入正题吧。

要控制指针实际上就是要给系统上报指针的位置（X,Y 坐标）。

这个可以是相对坐标（如鼠标上报的），也可以是绝对坐标（如触摸屏上报的）。

我们这里选鼠标吧，向系统注册一个鼠标设备，但实际上这个鼠标设备的数据是来着于手机触摸屏的，完美，哈哈哈。。。

那手机触摸屏的数据怎么传给我们注册的这个鼠标设备呢？

这里我们加入一个中间人，一个 Linux 用户空间的 app，我们利用网络把触摸数据传给这个 app，

然后由这个 app 再写给我们的鼠标设备，之后鼠标设备再上报给系统，完美

很显然我们的手机上也需要一个 app 来转换触摸数据为相对坐标变动发送给 Linux 端的 app。

所以整个系统会是像下面这样：

![](http://wx3.sinaimg.cn/mw690/9e169b75gy1fsl2gil1lxj20s50e7dg7.jpg)

## 四、mouse driver 实现

mouse driver 需要实现如下几个任务：

 - 把自己注册为鼠标设备；
 - 提供读写节点给用户空间 app 写入数据；
 - 转换 app 写入的数据通过事件上报接口上报给系统。

首先我们来看下怎么把自己注册为鼠标设备，代码如下：

	/* Allocate an input device data structure */ 
	vms_input_dev = input_allocate_device();  /* 第一步分配 */
	
	if (!vms_input_dev) 
	{ 
		perr("Bad input_allocate_device()\n"); return ENOMEM;
	}
	
	/* Basics */
	vms_input_dev->evbit[0] |= BIT(EV_KEY) | BIT(EV_REL); /* 告诉系统自己有上报 key，和 相对事件的能力 */

	/* Relative 'X' movement */
	set_bit(REL_X, vms_input_dev->relbit); /* 说明自己的具体能力，上报 x，y */
	/* Relative 'Y' movement */
	set_bit(REL_Y, vms_input_dev->relbit);

	set_bit(BTN_LEFT, vms_input_dev->keybit);  /* 说明自己的具体能力，上报左右按键与中键 */
	set_bit(BTN_RIGHT, vms_input_dev->keybit);
	set_bit(BTN_MIDDLE, vms_input_dev->keybit);	

    vms_input_dev->name = "vms-zoulm"; /* 一些设备信息，注意有些系统信息不完全将不能被成功识别 */
	vms_input_dev->id.bustype = 0xAA;    
	vms_input_dev->id.vendor = 0xDEAD;
	vms_input_dev->id.product = 0xBEEF;
	vms_input_dev->id.version = 0x0102;

	/* Register with the input subsystem */ 
	input_register_device(vms_input_dev);  /* 第二步注册*/

提供读写节点给用户空间 app 使用，得到 /dev/vms0 这个节点

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

定义和用户空间 app 交换的数据结构

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

上报从用户空间 app 写入的数据

	/*汇报情况*/
	input_report_key(vms_input_dev, BTN_LEFT, *(char*)pdata & 0x01);

	input_report_key(vms_input_dev, BTN_RIGHT, *(char*)pdata & 0x02);

	input_report_key(vms_input_dev, BTN_MIDDLE, *(char*)pdata & 0x04);

	input_report_rel(vms_input_dev, REL_X, pdata->rel_x);

	input_report_rel(vms_input_dev, REL_Y, pdata->rel_y);

	input_report_rel(vms_input_dev, REL_WHEEL, pdata->rel_wheel);
	
	/*同步更新输入*/
	input_sync(vms_input_dev);

到这里 mouse driver 的关键点都完成了。

## 五、Linux app 与 Android app 实现

首先 Linux app 和 Android app 的通信我们选用 udp 进行传输，主要基于以下两点考虑：

1. udp 简单，传输效率高
2. 触摸数据允许丢失，数据丢失对触摸影响不大

Liunx app 这边就不展示了，就是一个普通的 udp 接收程序，在指定的端口号上收到数据后写到设备节点。

Android app 里面主要做两件事：

1. 转换触摸屏的绝对坐标数据为相对坐标数据，并把点击转换为单击，三点触摸转换为右击然后打包
2. 通过 upd 把打包好的数据发送出去

这里需要注意的是 Activity 里面的事件处理是在 UI 线程处理的，所以在事件处理回掉中操作网络接口并不明智，所以这里 upd 发送任务单独开了线程，然后数据通过 Handle 传送过去。

## 六、后记

到这里我想你应该大致了解这整个过程，有没自己自定义一个输入设备的想法呢，哈哈。。。

比如自定义一个游戏手柄，可参考我多年以前的一个作品，如下：

https://v.youku.com/v_show/id_XNTQ4MDIzOTU2.html?spm=a2hzp.8244740.0.0

其全部实现我上传至了 github 上，如下地址：

https://github.com/Lingminzou/input-drive

此代码在 Ubuntu 12.04 上测试通过，但目前发现在 Ubuntu 16.04 上有问题，控制的时候鼠标指针像是被限制住了一样，目前我还没解决，后续若有解决，我再更新一下。

扫码关注我了解更多。

![](http://wx1.sinaimg.cn/large/9e169b75gy1fqcisgsbd7j2076076q3e.jpg)