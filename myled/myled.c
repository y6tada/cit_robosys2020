/*
 * myled.c
 * Device driver to send digital signal to sub microcontroller
 *
 * Version: 0.0.1
 *
 * Copyright (C) 2020 Yusuke Tada all right reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/hrtimer.h>

// Raspberry pi 4
#define RPI_REG_BASE (0xFE200000)

MODULE_AUTHOR("Yusuke Tada and Ryuichi Ueda");
MODULE_DESCRIPTION("Driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

typedef enum  {
    published,
    edited
} publish_status_t;

typedef enum {
    on,
    off,
    pulse
} gpio_status_t;

static publish_status_t publish_status = published;
static dev_t dev;
static struct cdev cdv;
static struct class *cls         = NULL;
static volatile u32 *gpio_base     = NULL;
/* Timer */
static struct hrtimer timer;

static enum hrtimer_restart timer_handler(struct hrtimer *_timer)
{
    gpio_base[10] = 1 << 25;
    return HRTIMER_NORESTART;
}

static void timer_set_handler(unsigned long time_s, unsigned long time_ns)
{
    hrtimer_init(&timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    timer.function = timer_handler;
    hrtimer_start(&timer, ktime_set(time_s, time_ns), HRTIMER_MODE_REL);
}

void gpio_set(gpio_status_t _order)
{
    publish_status = edited;
    switch (_order) {
        case on:
        gpio_base[7] = 1 << 25;
        break;

        case off:
        gpio_base[10] = 1 << 25;
        break;

        case pulse:
        gpio_base[7] = 1 << 25;
        timer_set_handler(0, 200000000);
        break;

        default:
        publish_status = published;
        break;
    }
}

static ssize_t mcu_sigsend(struct file* filp, const char* buf, size_t count, loff_t *pos)
{
    char c;
    if(copy_from_user(&c, buf, sizeof(char))) {
        return -EFAULT;
    }

    // printk(KERN_INFO "recieve %c\n", c);

    switch (c) {
        case '0':
        gpio_set(off);
        break;

        case '1':
        gpio_set(on);
        break;

        case 'p':
        gpio_set(pulse);
        break;

        default:
        break;
    }

    return 1;
}

static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t *pos)
{
    int size = 0;
    const char msg_of_read[] = {0xF0, 0x9F, 0x8D, 0xA3 ,0x0A};

    if (publish_status == edited) {
        if (copy_to_user(buf + size, (const char *)msg_of_read, sizeof(msg_of_read))) {
            printk(KERN_ERR "msg_of_read: copy_to_user failed.\n");
            return -EFAULT;
        }
        publish_status = published;
        printk(KERN_INFO "status changed to published.\n");
    }

    size += sizeof(msg_of_read);
    return size;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = mcu_sigsend,
    .read  = sushi_read
};

static int __init init_mod(void)
{
    int retval;
    retval = alloc_chrdev_region(&dev, 0, 1, "myled");
    if (retval < 0) {
        printk(KERN_ERR "alloc_chrdev_region failed.\n");
        return retval;
    }
    printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));

    cdev_init(&cdv, &led_fops);
    retval = cdev_add(&cdv, dev, 1);
    if (retval < 0) {
        printk(KERN_ERR "cdev_add failed. major:%d, manor:%d\n", MAJOR(dev), MINOR(dev));
    }

    cls = class_create(THIS_MODULE, "myled");
    if (IS_ERR(cls)) {
        printk(KERN_ERR, "class create failed.");
        return PTR_ERR(cls);
    }

    device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));
    
    gpio_base = ioremap(RPI_REG_BASE, 0xA0);

    const u32 led       = 25;
    const u32 index     = led / 10;
    const u32 shift     = (led%10)*3;
    const u32 mask      = ~(0x7 << shift);
    gpio_base[index]    = (gpio_base[index] & mask) | (0x1 << shift);

    return 0;
}

static void __exit cleanup_mod(void)
{
    cdev_del(&cdv);
    device_destroy(cls, dev);
    class_destroy(cls);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);

