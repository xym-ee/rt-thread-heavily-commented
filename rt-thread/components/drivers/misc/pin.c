/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-01-20     Bernard      the first version
 * 2021-02-06     Meco Man     fix RT_ENOSYS code in negative
 */

#include <drivers/pin.h>
#include <finsh.h>

/*
 * 驱动框架层的抽象，write、read、control
 *
 */


static struct rt_device_pin _hw_pin;

/*  rt_device_t 里的read，通用read方法调用此函数
    _hw_pin.parent.read         = _pin_read;  
    此函数内部调用子类 pin 里的 ops 读方法
    */
static rt_size_t _pin_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_device_pin_status *pin_status;
    struct rt_device_pin *dev_pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(dev_pin != RT_NULL);

    pin_status = (struct rt_device_pin_status *) buffer;
    if (pin_status == RT_NULL || size != sizeof(*pin_status)) return 0;

    pin_status->status = dev_pin->ops->pin_read(dev, pin_status->pin);
    return size;
}

/* 同read，未修改局部变量名 */
static rt_size_t _pin_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_device_pin_status *status;
    struct rt_device_pin *pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(pin != RT_NULL);

    status = (struct rt_device_pin_status *) buffer;
        
    if (status == RT_NULL || size != sizeof(*status)) return 0;

    pin->ops->pin_write(dev, (rt_base_t)status->pin, (rt_base_t)status->status);

    return size;
}

static rt_err_t _pin_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_device_pin_mode *mode;
    struct rt_device_pin *pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(pin != RT_NULL);

    mode = (struct rt_device_pin_mode *) args;
    if (mode == RT_NULL) return -RT_ERROR;

    pin->ops->pin_mode(dev, (rt_base_t)mode->pin, (rt_base_t)mode->mode);

    return 0;
}

int rt_device_pin_register(const char *name, const struct rt_pin_ops *ops, void *user_data)
{
    /* pin注册函数，struct rt_device;的数据 */
    _hw_pin.parent.type         = RT_Device_Class_Miscellaneous;
    _hw_pin.parent.rx_indicate  = RT_NULL;
    _hw_pin.parent.tx_complete  = RT_NULL;

    /* struct rt_device; 里的通用设备操作方法，对接 rt_device_xx 方法 */
    _hw_pin.parent.init         = RT_NULL;
    _hw_pin.parent.open         = RT_NULL;
    _hw_pin.parent.close        = RT_NULL;
    _hw_pin.parent.read         = _pin_read;
    _hw_pin.parent.write        = _pin_write;
    _hw_pin.parent.control      = _pin_control;
    _hw_pin.parent.user_data    = user_data;
    
    /* pin设备专用操作方法 */
    _hw_pin.ops                 = ops;

    /* 调用通用 device 注册方法，注册字符设备 */
    rt_device_register(&_hw_pin.parent, name, RT_DEVICE_FLAG_RDWR);

    return 0;
}


/* RT-Thread Hardware PIN APIs */
void rt_pin_mode(rt_base_t pin, rt_base_t mode)
{
    RT_ASSERT(_hw_pin.ops != RT_NULL);
    _hw_pin.ops->pin_mode(&_hw_pin.parent, pin, mode);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pin_mode, pinMode, set hardware pin mode);

void rt_pin_write(rt_base_t pin, rt_base_t value)
{
    RT_ASSERT(_hw_pin.ops != RT_NULL);
    _hw_pin.ops->pin_write(&_hw_pin.parent, pin, value);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pin_write, pinWrite, write value to hardware pin);

int rt_pin_read(rt_base_t pin)
{
    RT_ASSERT(_hw_pin.ops != RT_NULL);
    return _hw_pin.ops->pin_read(&_hw_pin.parent, pin);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pin_read, pinRead, read status from hardware pin);

rt_base_t rt_pin_get(const char *name)
{
    RT_ASSERT(_hw_pin.ops != RT_NULL);
    RT_ASSERT(name[0] == 'P');

    if(_hw_pin.ops->pin_get == RT_NULL)
    {
        return -RT_ENOSYS;
    }

    return _hw_pin.ops->pin_get(name);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pin_get, pinGet, get pin number from hardware pin);




