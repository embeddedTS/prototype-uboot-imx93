#
# Copyright 2023-2024 embeddedTS
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += ts4300.o
obj-y += ../../../board/freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += lpddr4x-alliance-8gb-3733.o
obj-y += lpddr4x-alliance-8gb-3200.o
endif

obj-y += ../tsimx93-common/
obj-y += ../ts-common-baseboards/bbdetect.o
obj-y += ../ts-common-baseboards/ts_extension_boards.o
obj-y += ../ts-common/
obj-y += parse_straps.o
