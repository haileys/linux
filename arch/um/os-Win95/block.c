#include "linux/blk_types.h"
#include "linux/highmem-internal.h"
#include "linux/kern_levels.h"
#include "linux/printk.h"
#include <linux/blkdev.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <wsl9x.h>
#include <wsl9x/ifsmgr.h>

#define DRIVER_NAME "wsl9x-block"
#define DRIVER_BLOCK_SIZE 512
#define IMAGE_PATH "C:\\WSL\\ROOTFS.IMG"

static int major;

struct wslbd_device {
	HFILE file;
	struct gendisk* gd;
	struct platform_device platform;
};

static struct platform_driver wsl9x_block_driver = {
	.driver = {
		.name = DRIVER_NAME,
	},
};

static void wslbd_submit_bio(struct bio* bio)
{
	printk(KERN_ERR DRIVER_NAME ": submit_bio!");

	struct wslbd_device *dev = bio->bi_bdev->bd_disk->private_data;
	struct bio_vec bv = bio_iter_iovec(bio, bio->bi_iter);
	void* kaddr = bvec_kmap_local(&bv);

	sector_t sector = bio->bi_iter.bi_sector;
	u32 offset = sector * DRIVER_BLOCK_SIZE;

	blk_opf_t opf = bio->bi_opf;

	ssize_t rc;

	enum req_op op = opf & REQ_OP_MASK;
	switch (op) {
	case REQ_OP_READ:
		rc = IFSMgr_ReadFile(dev->file, bv.bv_len, offset, kaddr);
		if (rc < 0) {
			printk(KERN_ERR DRIVER_NAME ": IFSMgr_ReadFile: len=%u offset=%u error=%d", bv.bv_len, offset, rc);
			goto err;
		}
		bio_advance_iter_single(bio, &bio->bi_iter, rc);
		goto out;
	default:
		panic("wslbd_submit_bio: unknown op: %d", op);
	}

err:
	bio_io_error(bio);
out:
	kunmap_local(kaddr);
}

static struct block_device_operations wsl9x_block_ops = {
	.submit_bio = wslbd_submit_bio,
};

static u32 block_count_rounding_up(u32 bytes)
{
	return (bytes + (DRIVER_BLOCK_SIZE - 1)) / DRIVER_BLOCK_SIZE;
}

static void wslbd_device_release(struct device *dev)
{
}

static int wslbd_device_init(struct wslbd_device* dev)
{
	int err = 0;

	struct queue_limits lim = {
		/*
		 * This is so fdisk will align partitions on 4k, because of
		 * direct_access API needing 4k alignment, returning a PFN
		 * (This is only a problem on very small devices <= 4M,
		 *  otherwise fdisk will align on 1M. Regardless this call
		 *  is harmless)
		 */
		.physical_block_size	= DRIVER_BLOCK_SIZE,
		.max_hw_discard_sectors	= UINT_MAX,
		.max_discard_segments	= 1,
		.discard_granularity	= DRIVER_BLOCK_SIZE,
		.features		= BLK_FEAT_SYNCHRONOUS,
	};

	HFILE file = IFSMgr_OpenCreateFile(IFS_OPEN_READWRITE, 0, IFS_OPEN_IF_EXIST, IMAGE_PATH);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		printk(KERN_ERR DRIVER_NAME ": IFSMgr_OpenCreateFile %s: error %d", IMAGE_PATH, err);
		goto err_open;
	}

	dev->platform.dev.release = wslbd_device_release;
	err = platform_device_register(&dev->platform);
	if (err) {
		goto err_device_register;
	}

	u32 byte_size = 0;
	err = IFSMgr_GetFileSize(file, &byte_size);
	if (err) {
		printk(KERN_ERR DRIVER_NAME ": IFSMgr_GetFileSize: error %d", err);
		// fall through with size = 0
	}

	struct gendisk* gd = blk_alloc_disk(&lim, NUMA_NO_NODE);
	if (IS_ERR(gd)) {
		err = PTR_ERR(gd);
		printk(KERN_ERR DRIVER_NAME ": blk_alloc_disk: error %d", err);
		goto err_alloc_disk;
	}

	gd->major = major;
	gd->first_minor = 0;
	gd->minors = 1;
	gd->fops = &wsl9x_block_ops;
	gd->private_data = dev;
	sprintf(gd->disk_name, "%s%d", dev->platform.name, dev->platform.id);
	set_capacity(gd, block_count_rounding_up(byte_size));

	dev->gd = gd;
	dev->file = file;

	err = device_add_disk(&dev->platform.dev, gd, NULL);
	if (err) {
		printk(KERN_ERR DRIVER_NAME ": device_add_disk: error %d", err);
		goto err_add_disk;
	}

	return 0;

err_add_disk:
	put_disk(gd);
err_alloc_disk:
	platform_device_unregister(&dev->platform);
err_device_register:
	IFSMgr_CloseFile(file);
err_open:
	return err;
}

static struct wslbd_device dev;

static int __init wsl9x_block_init(void)
{
	int rc = register_blkdev(major, DRIVER_NAME);
	if (rc < 0) {
		printk(KERN_ERR DRIVER_NAME ": register_blkdev: major %d, error %d\n", major, rc);
		goto err;
	}

	major = rc;

	rc = platform_driver_register(&wsl9x_block_driver);
	if (rc) {
		printk(KERN_ERR DRIVER_NAME ": platform_driver_register: error %d\n", rc);
		goto err_register_driver;
	}

	dev.platform.name = "wslbd";
	dev.platform.id = 0;
	rc = wslbd_device_init(&dev);
	if (rc) {
		printk(KERN_ERR DRIVER_NAME ": init_device: error %d\n", rc);
		goto err_init_device;
	}

	return 0;

err_init_device:
	platform_driver_unregister(&wsl9x_block_driver);
err_register_driver:
	unregister_blkdev(major, DRIVER_NAME);
err:
	return rc;
}

__initcall(wsl9x_block_init);
