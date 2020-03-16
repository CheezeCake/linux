#include "nouveau_rsx.h"

#define DEVICE_NAME "rsxdrm"

static int rsx_probe(struct ps3_system_bus_device *sbdev)
{
	struct nvkm_device *device = NULL;
	struct drm_device *drm;
	int ret;

	pr_err(" -> %s()", __func__);

	ret = ps3_open_hv_device(sbdev);
	if (ret) {
		dev_dbg(&sbdev->core, "%s:ps3_open_hv_device failed\n",
			__func__);
		goto fail_open;
	}

	ret = ps3_dma_region_create(sbdev->d_region);
	if (ret) {
		dev_dbg(&sbdev->core, "%s:ps3_dma_region_create failed(%d)\n",
			__func__, ret);
		BUG_ON("check region type");
		goto fail_dma_region;
	}

	drm = nouveau_rsx_create(sbdev, &device);
	if (IS_ERR(drm)) {
		ret = PTR_ERR(drm);
		goto fail_rsx_create;
	}

	ret = drm_dev_register(drm, 0);
	if (ret < 0) {
		drm_dev_put(drm);
		goto fail_drm_register;
	}

	pr_err(" <- %s()", __func__);
	return 0;

fail_drm_register:
fail_rsx_create:
	ps3_dma_region_free(sbdev->d_region);
fail_dma_region:
	ps3_close_hv_device(sbdev);
fail_open:
	pr_err(" <- %s() ret=%d", __func__, ret);
	return ret;
}

static int rsx_shutdown(struct ps3_system_bus_device *sbdev)
{
	struct drm_device *dev = ps3_system_bus_get_drvdata(sbdev);

	nouveau_drm_device_remove(dev);
	ps3_dma_region_free(sbdev->d_region);
	ps3_close_hv_device(sbdev);

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
