#include "cmd_reset.h"

#include <stdio.h>
#include <string.h>
#include <usbbluetooth.h>

#define BUFFSIZE 64

static usbbluetooth_status_t reset_dev(usbbluetooth_device_t *dev)
{
    printf("Opening %04x:%04x @ 0x%p\n", dev->vendor_id, dev->product_id, dev);

    usbbluetooth_status_t r = usbbluetooth_open(dev);
    if (r != USBBLUETOOTH_STATUS_OK)
    {
        fprintf(stderr, "Could not open device (r=%d, s=%s)\n", (int)r, usbbluetooth_status_name(r));
        goto exit;
    }

    // Send encoded HCI reset
    r = usbbluetooth_write(dev, (uint8_t *)"\x01\x03\x0c\x00", 4);
    if (r != USBBLUETOOTH_STATUS_OK)
    {
        fprintf(stderr, "Could not send command (r=%d, s=%s)\n", (int)r, usbbluetooth_status_name(r));
        goto exit;
    }

    uint8_t buf[BUFFSIZE];
    memset(buf, 0, BUFFSIZE);
    uint16_t bufsize = BUFFSIZE;
    r = usbbluetooth_read(dev, buf, &bufsize);
    if (r != USBBLUETOOTH_STATUS_OK)
    {
        fprintf(stderr, "Could not read command response (r=%d, s=%s)\n", (int)r, usbbluetooth_status_name(r));
        goto exit;
    }

    if (buf[6] != 0)
    {
        fprintf(stderr, "Command response status not OK! (status=%d)\n", buf[6]);
        r = USBBLUETOOTH_STATUS_ERR_UNK;
        goto exit;
    }

    printf("Device reset OK!\n");

exit:
    usbbluetooth_close(dev);
    return r;
}

static void reset_devs(usbbluetooth_device_t **devs)
{
    usbbluetooth_device_t *dev;
    for (int i = 0; (dev = devs[i]) != NULL; i++)
        reset_dev(dev);
}

int cmd_reset()
{
    int ret = 0;
    usbbluetooth_device_t **devs = NULL;

    usbbluetooth_status_t r = usbbluetooth_init();
    if (r < 0)
    {
        fprintf(stderr, "Could not initialize usbbluetooth (r=%d, s=%s)\n", (int)r, usbbluetooth_status_name(r));
        ret = -1;
        goto exit;
    }

    r = usbbluetooth_get_device_list(&devs);
    if (r < 0)
    {
        fprintf(stderr, "Could get a device list (r=%d, s=%s)\n", (int)r, usbbluetooth_status_name(r));
        ret = -1;
        goto exit;
    }

    reset_devs(devs);

exit:
    usbbluetooth_free_device_list(&devs);
    usbbluetooth_exit();

    return ret;
}