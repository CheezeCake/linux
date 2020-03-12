#include "nouveau_rsx.h"

#define DEVICE_NAME "rsxdrm"

static int rsx_probe(struct ps3_system_bus_device *sbdev)
{
	struct nvkm_device *device = NULL;
	struct drm_device *drm;
	int ret;

	pr_err(" -> %s()", __func__);

	drm = nouveau_rsx_create(sbdev, &device);
	if (IS_ERR(drm))
		return PTR_ERR(drm);

	pr_err("%s(): before drm_dev_register", __func__);
	ret = drm_dev_register(drm, 0);
	if (ret < 0) {
		drm_dev_put(drm);
		return ret;
	}

	pr_err(" <- %s()", __func__);
	return 0;
}

static int rsx_shutdown(struct ps3_system_bus_device *dev)
{
	return 0;
}

// XXX: const?
struct ps3_system_bus_driver nouveau_rsx_driver = {
	.match_id	= PS3_MATCH_ID_GPU,
	.match_sub_id	= PS3_MATCH_SUB_ID_GPU_DRM,
	.core.name	= DEVICE_NAME,
	.core.owner	= THIS_MODULE,
	.probe		= rsx_probe,
	.remove		= rsx_shutdown,
	.shutdown	= rsx_shutdown,
};
