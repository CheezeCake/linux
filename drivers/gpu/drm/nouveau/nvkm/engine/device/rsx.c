#include <linux/kernel.h>
#include <asm/lv1call.h>

#include <core/rsx.h>
#include "priv.h"

/* MMIO */
#define RSX_BAR0_ADDR	0x28000000000ULL
#define RSX_BAR0_SIZE	(32 * 1024 * 1024)

/* VRAM */
#define RSX_BAR1_ADDR	0x28080000000ULL
#define RSX_BAR1_SIZE	(256 * 1024 * 1024)

/* PRAMIN */
#define RSX_BAR2_ADDR	0x28002000000ULL
#define RSX_BAR2_SIZE	(1 * 1024 * 1024)

static struct nvkm_device_rsx *
nvkm_device_rsx(struct nvkm_device *device)
{
	return container_of(device, struct nvkm_device_rsx, device);
}

static void unmap_bars(const struct nvkm_device_rsx *rdev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rdev->bar); i++) {
		if (rdev->bar[i].lpar)
			lv1_undocumented_function_115(rdev->bar[i].lpar);
	}
}

static void *
nvkm_device_rsx_dtor(struct nvkm_device *device)
{
	struct nvkm_device_rsx *rdev = nvkm_device_rsx(device);
	unmap_bars(rdev);
	return rdev;
}

static resource_size_t
nvkm_device_rsx_resource_addr(struct nvkm_device *device, unsigned bar)
{
	struct nvkm_device_rsx *rdev = nvkm_device_rsx(device);
	resource_size_t ret = (bar < ARRAY_SIZE(rdev->bar)) ? rdev->bar[bar].lpar : 0;
	pr_err("%s():%d bar=%u, ret=0x%08llx\n", __func__, __LINE__, bar, ret);
	return ret;
}

static resource_size_t
nvkm_device_rsx_resource_size(struct nvkm_device *device, unsigned bar)
{
	struct nvkm_device_rsx *rdev = nvkm_device_rsx(device);
	return (bar < ARRAY_SIZE(rdev->bar)) ? rdev->bar[bar].size : 0;
}

static const struct nvkm_device_func
nvkm_device_rsx_func = {
	.rsx = nvkm_device_rsx,
	.dtor = nvkm_device_rsx_dtor,
	/* .preinit = nvkm_device_rsx_preinit, */
	/* .fini = nvkm_device_rsx_fini, */
	.resource_addr = nvkm_device_rsx_resource_addr,
	.resource_size = nvkm_device_rsx_resource_size,
	.cpu_coherent = false, /* XXX: true for cell? */
};

static int map_bar(struct nvkm_device_rsx *rdev, unsigned bar,
		   u64 addr, u64 size)
{
	if (bar >= ARRAY_SIZE(rdev->bar))
		return -EINVAL;
	if (lv1_undocumented_function_114(addr, PAGE_SHIFT, size,
					  &rdev->bar[bar].lpar))
		return -EFAULT;
	rdev->bar[bar].size = size;

	return 0;
}

int nvkm_device_rsx_new(struct ps3_system_bus_device *sbdev,
			const char *cfg, const char *dbg,
			bool detect, bool mmio, u64 subdev_mask,
			struct nvkm_device **pdevice)
{
	struct nvkm_device_rsx *rdev;
	int ret;

	pr_err(" -> %s()", __func__);

	if (!(rdev = kzalloc(sizeof(*rdev), GFP_KERNEL)))
		return -ENOMEM;

	rdev->sbdev = sbdev;
	ret = map_bar(rdev, 0, RSX_BAR0_ADDR, RSX_BAR0_SIZE);
	if (ret)
		goto free;
	ret = map_bar(rdev, 1, RSX_BAR1_ADDR, RSX_BAR1_SIZE);
	if (ret)
		goto unmap_bar;
	ret = map_bar(rdev, 2, RSX_BAR2_ADDR, RSX_BAR2_SIZE);
	if (ret)
		goto unmap_bar;

	ret = nvkm_device_ctor(&nvkm_device_rsx_func, NULL, &sbdev->core,
			       NVKM_DEVICE_RSX, sbdev->dev_id, NULL,
			       cfg, dbg, detect, mmio, subdev_mask,
			       &rdev->device);
	if (ret)
		goto unmap_bar;

	*pdevice = &rdev->device;

	pr_err(" <- %s()", __func__);
	return 0;

unmap_bar:
	unmap_bars(rdev);
free:
	kfree(rdev);
	return ret;
}
