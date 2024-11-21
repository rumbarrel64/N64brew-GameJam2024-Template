# 1 - Specify Assets you want (like pngs files)
ASSETS_LIST += \
	filesystem/FinishLineFrenzy/sphere.t3dm \
	filesystem/FinishLineFrenzy/flatplane.t3dm 

# t3d flags
$(ASSETS)/sphere.t3dm: T3DM_FLAGS = --base-scale=1
$(ASSETS)/dsphere.t3dm: T3DM_FLAGS = --base-scale=1