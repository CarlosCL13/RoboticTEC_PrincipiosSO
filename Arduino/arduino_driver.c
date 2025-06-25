#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/usb/cdc.h>
#include <linux/completion.h>

#define DRIVER_AUTHOR "Meibel Ceciliano, Carlos Contreras, Ludwin Ramos"
#define DRIVER_DESC   "Driver para comunicación con Arduino Uno R3 usando USB CDC ACM"

#define DRIVER_NAME "Arduino_usb_driver"
#define DEVICE_NAME "arduino_driver"
#define USB_CLASS_DATA 0x0A


static struct usb_device *arduino_dev;
static unsigned char out_ep;
static int out_ep_max_packet_size;
static int device_registered = 0;

static struct completion write_done;

static void write_urb_complete(struct urb *urb){
    complete(&write_done);
}

static int arduino_open(struct inode *inode, struct file *file) {
    pr_info(DRIVER_NAME ": arduino_open llamado\n");
    return 0;
}

// Función de escritura del dispositivo (archivo de caracter en /dev/ que se usa para enviar datos al Arduino)
static ssize_t arduino_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {

    static struct urb *write_urb;
    static unsigned char *write_buffer;
    int retval;
    int len;

    if (!arduino_dev || !out_ep)
        return -ENODEV;

    if (count > 64)  // limita tamaño (puede ajustarse pero para un arduino típico 64 bytes)
        count = 64;

    write_buffer = kmalloc(count, GFP_KERNEL);
    if (!write_buffer)
        return -ENOMEM;

    if (copy_from_user(write_buffer, buf, count)) {
        kfree(write_buffer);
        return -EFAULT;
    }

    write_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!write_urb) {
        kfree(write_buffer);
        return -ENOMEM;
    }

    init_completion(&write_done);

    usb_fill_bulk_urb(write_urb,
                      arduino_dev,
                      usb_sndbulkpipe(arduino_dev, out_ep),
                      write_buffer,
                      count,
                      write_urb_complete,
                      NULL);

    retval = usb_submit_urb(write_urb, GFP_KERNEL);
    if (retval) {
        pr_err("arduino_usb_driver: usb_submit_urb error %d\n", retval);
        usb_free_urb(write_urb);
        kfree(write_buffer);
        return retval;
    }

    // Esperar a que se complete o hasta timeout (3 seg)
    retval = wait_for_completion_timeout(&write_done, msecs_to_jiffies(3000));
    if (retval == 0) {
        pr_err("arduino_usb_driver: timeout en urb_write\n");
        usb_kill_urb(write_urb);
        usb_free_urb(write_urb);
        kfree(write_buffer);
        return -ETIMEDOUT;
    }

    len = write_urb->actual_length;

    usb_free_urb(write_urb);
    kfree(write_buffer);

    pr_info("arduino_usb_driver: datos enviados, len=%d\n", len);

    return len;
}

// Definición de las operaciones del dispositivo
static const struct file_operations arduino_fops = {
    .owner = THIS_MODULE,
    .open = arduino_open,
    .write = arduino_write,
    .llseek = no_llseek,
};

static struct miscdevice arduino_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &arduino_fops,
    .mode = 0666,
};

// Identificadores USB para interfaz CDC ACM
static const struct usb_device_id arduino_table[] = {
    { USB_INTERFACE_INFO(USB_CLASS_COMM, USB_CDC_SUBCLASS_ACM, USB_CDC_ACM_PROTO_AT_V25TER) },
    { USB_INTERFACE_INFO(USB_CLASS_DATA, 0x00, 0x00) },
    { }
};

// Registrar la tabla de dispositivos USB (permite que arduino_probe se dispare cuando un dispositivo coincida)
MODULE_DEVICE_TABLE(usb, arduino_table);

// Encuentra el endpoint OUT de la interfaz de datos
static int find_out_endpoint(struct usb_interface *interface) {
    struct usb_host_interface *iface_desc = interface->cur_altsetting;
    int i;
    for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
        struct usb_endpoint_descriptor *ep = &iface_desc->endpoint[i].desc;
        if (usb_endpoint_is_bulk_out(ep)) {
            out_ep = ep->bEndpointAddress;
            out_ep_max_packet_size = usb_endpoint_maxp(ep);
            pr_info(DRIVER_NAME ": Endpoint OUT encontrado: 0x%02x\n", out_ep);
            return 0;
        }
    }
    return -ENODEV;
}

// Función de probe del controlador USB: se llama cuando se conecta un dispositivo que coincide con la tabla
static int arduino_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_device *dev = interface_to_usbdev(interface);
    u8 iface_class = interface->cur_altsetting->desc.bInterfaceClass; 
    int ret;

    pr_info(DRIVER_NAME ": probe ejecutado para clase 0x%02x\n", iface_class);

    if (iface_class == USB_CLASS_COMM) {
        struct usb_cdc_line_coding line = {
            .dwDTERate   = cpu_to_le32(9600),
            .bCharFormat = 0,
            .bParityType = 0,
            .bDataBits   = 8,
        };

        int iface_num = interface->cur_altsetting->desc.bInterfaceNumber;

        // Configura velocidad y control
        ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                              USB_CDC_REQ_SET_LINE_CODING,
                              USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                              0, iface_num, &line, sizeof(line), 1000);

        if (ret < 0) {
            pr_err(DRIVER_NAME ": SET_LINE_CODING falló: %d\n", ret);
            return ret;
        }

        ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                              USB_CDC_REQ_SET_CONTROL_LINE_STATE,
                              USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                              3, iface_num, NULL, 0, 1000);

        if (ret < 0) {
            pr_err(DRIVER_NAME ": SET_CONTROL_LINE_STATE falló: %d\n", ret);
            return ret;
        }

        pr_info(DRIVER_NAME ": Interfaz de control configurada\n");
        return 0;
    }

    if (iface_class == USB_CLASS_DATA) {
        ret = find_out_endpoint(interface);
        if (ret)
            return ret;

        arduino_dev = dev;

        if (!device_registered) {
            arduino_miscdev.fops = &arduino_fops;
            ret = misc_register(&arduino_miscdev);
            if (ret) {
                pr_err(DRIVER_NAME ": Error al registrar miscdevice\n");
                return ret;
            }
            device_registered = 1;
        }

        pr_info(DRIVER_NAME ": Interfaz de datos activa, dispositivo listo\n");
        return 0;
    }

    return -ENODEV;
}

// Función de desconexión del controlador USB: se llama cuando el dispositivo se desconecta
static void arduino_disconnect(struct usb_interface *interface) {
    pr_info(DRIVER_NAME ": Dispositivo desconectado\n");
    if (device_registered) {
        misc_deregister(&arduino_miscdev);
        device_registered = 0;
    }
    arduino_dev = NULL;
    out_ep = 0;
    out_ep_max_packet_size = 0;
}

// Estructura del controlador USB
static struct usb_driver arduino_driver = {
    .name = DRIVER_NAME,
    .id_table = arduino_table,
    .probe = arduino_probe,
    .disconnect = arduino_disconnect,
};

module_usb_driver(arduino_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
