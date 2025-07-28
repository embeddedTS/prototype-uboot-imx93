#
# Copyright 2023-2024 embeddedTS
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += ts4300.o
obj-y += ../../../board/freescale/common/

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += lpddr4x-8gb.o
endif

obj-y += ../ts-common-baseboards/bbdetect.o
obj-y += ../ts-common-baseboards/ts_extension_boards.o
obj-y += ../ts-common/
obj-y += parse_straps.o
