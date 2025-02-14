#
# Copyright 2023-2024 embeddedTS
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += ts9370.o
obj-y += ../../../board/freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += lpddr4x-16gb-3733.o lpddr4x-alliance-8gb-3733.o
endif
obj-y += ../tsimx93-common/
obj-y += ../ts-common/
obj-y += parse_straps.o
obj-y += tsfpga.o
obj-y += fpga_bootloader.o
