# Temporary makefile to build all capri boards for testing. Can be deleted later.

BOARDS := \
bcm11130_ray \
bcm11130_ray_jffs2 \
bcm11140_ray \
bcm11140_tablet \
bcm11140_li_tablet \
bcm11351_ray \
bcm11351_tablet \
bcm11351_li_tablet \


all: ${BOARDS}

${BOARDS}: 
	make -j8 $@ O=obj/$@

clean:
	rm -rf obj
