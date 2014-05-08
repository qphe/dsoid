LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE    := dsoidneon
LOCAL_ARM_NEON 			:= true
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := -fexceptions -DHAVE_NEON=1 -ftree-vectorize -fsingle-precision-constant -fprefetch-loop-arrays -fvariable-expansion-in-unroller -mfloat-abi=softfp -mfpu=neon -marm -march=armv7-a -mtune=cortex-a9
LOCAL_LDLIBS :=  -llog -lGLESv1_CM -ldl -lz

### Add all source file names to be inclded in lib separated by a whitespace
LOCAL_SRC_FILES := utils/ConvertUTF.c utils/ConvertUTF.h utils/datetime.h utils/datetime.cpp utils/guid.cpp utils/guid.h \
utils/emufat.cpp utils/emufat.h utils/emufat_types.h\
utils/md5.cpp utils/md5.h utils/valuearray.h utils/xstring.cpp utils/xstring.h \
utils/decrypt/crc.cpp utils/decrypt/crc.h utils/decrypt/decrypt.cpp \
utils/decrypt/decrypt.h utils/decrypt/header.cpp utils/decrypt/header.h \
utils/task.cpp utils/task.h \
utils/vfat.h utils/vfat.cpp \
utils/dlditool.cpp \
utils/libfat/bit_ops.h \
utils/libfat/cache.cpp \
utils/libfat/cache.h \
utils/libfat/common.h \
utils/libfat/directory.cpp \
utils/libfat/directory.h \
utils/libfat/disc.cpp \
utils/libfat/disc.h \
utils/libfat/disc_io.h \
utils/libfat/fat.h \
utils/libfat/fatdir.cpp \
utils/libfat/fatdir.h \
utils/libfat/fatfile.cpp \
utils/libfat/fatfile.h \
utils/libfat/filetime.cpp \
utils/libfat/filetime.h \
utils/libfat/file_allocation_table.cpp \
utils/libfat/file_allocation_table.h \
utils/libfat/libfat.cpp \
utils/libfat/libfat_pc.h \
utils/libfat/libfat_public_api.cpp \
utils/libfat/libfat_public_api.h \
utils/libfat/lock.cpp \
utils/libfat/lock.h \
utils/libfat/mem_allocate.h \
utils/libfat/partition.cpp \
utils/libfat/partition.h \
utils/tinyxml/tinystr.cpp \
utils/tinyxml/tinystr.h \
utils/tinyxml/tinyxml.cpp \
utils/tinyxml/tinyxml.h \
utils/tinyxml/tinyxmlerror.cpp \
utils/tinyxml/tinyxmlparser.cpp \
addons/slot2_mpcf.cpp addons/slot2_paddle.cpp addons/slot2_gbagame.cpp addons/slot2_none.cpp addons/slot2_rumblepak.cpp addons/slot2_guitarGrip.cpp addons/slot2_expMemory.cpp addons/slot2_piano.cpp addons/slot1_none.cpp addons/slot1_r4.cpp addons/slot1_retail.cpp addons/slot1_retail_nand.cpp \
metaspu/metaspu.h\
metaspu/metaspu.cpp\
dynarec/arm_codegen.h\
dynarec/arm_dpimacros.h\
dynarec/arm_emit.h\
dynarec/arm_stub.S\
dynarec/cpu_threaded.cpp\
dynarec/cpu.h\
dynarec/cpu.cpp\
dynarec/dynarec_linker.h\
dynarec/dynarec_linker.cpp\
dynarec/warm.h\
dynarec/warm.cpp\
addons.h\
addons.cpp\
arm_instructions.h\
arm_instructions.cpp\
armcpu.h\
armcpu.cpp\
bios.h\
bios.cpp\
bits.h\
cheatSystem.h\
cheatSystem.cpp\
common.h\
common.cpp\
cp15.h\
cp15.cpp\
debug.h\
debug.cpp\
Disassembler.h\
Disassembler.cpp\
driver.h\
driver.cpp\
emufile_types.h\
emufile.h\
emufile.cpp\
fat.h\
FIFO.h\
FIFO.cpp\
firmware.h\
firmware.cpp\
fs.h\
fs-linux.cpp\
gfx3d.h\
gfx3d.cpp\
GPU_osd.h\
GPU_osd_stub.cpp\
GPU.h\
GPU.cpp\
main.cpp\
matrix.h\
matrix.cpp\
mic.h\
mic.cpp\
mc.h\
mc.cpp\
mem.h\
MMU.h\
MMU_timing.h\
MMU.cpp\
NDSSystem.h\
NDSSystem.cpp\
movie.h\
movie.cpp\
PACKED_END.h\
PACKED.h\
path.h\
path.cpp\
rasterize.h\
rasterize.cpp\
readwrite.h\
readwrite.cpp\
registers.h\
render3D.h\
render3D.cpp\
ROMReader.h\
ROMReader.cpp\
rtc.h\
rtc.cpp\
saves.h\
saves.cpp\
shaders.h\
slot1.h\
slot1.cpp\
sndsdl.h\
sndsdl.cpp\
SPU.h\
SPU.cpp\
texcache.h\
texcache.cpp\
thumb_instructions.h\
thumb_instructions.cpp\
types.h\
version.h\
version.cpp\
wifi.h\
wifi.cpp\

include $(BUILD_SHARED_LIBRARY)
