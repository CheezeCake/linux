#ifndef __NVKM_DEVICE_RSX_H__
#define __NVKM_DEVICE_RSX_H__

#include <asm/ps3.h>

#include <core/device.h>

struct nvkm_device_rsx {
	struct nvkm_device device;
	struct ps3_system_bus_device *sbdev;
	struct {
		u64 lpar;
		u64 size;
	} bar[3];
};

int nvkm_device_rsx_new(struct ps3_system_bus_device *sbdev,
			const char *cfg, const char *dbg,
			bool detect, bool mmio, u64 subdev_mask,
			struct nvkm_device **pdevice);

#endif
