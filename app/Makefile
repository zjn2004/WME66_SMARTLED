#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug
USER = smartled
#USER = lexyairpurifier

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	user    \
	driver  \
	user/ledctl/src \
        user/user_uart/src

ifeq ($(USER),smartled)
    SUBDIRS += user/user_smartled/src
else ifeq ($(USER),lexyairpurifier)
    SUBDIRS += user/user_lexyairpurifier/src
endif

endif # } PDIR

LDDIR = $(SDK_PATH)/ld

CCFLAGS += -Os -D_PLATFORM_ESPRESSIF_8266__  -lalink -lssl -lcrypto

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

COMPONENTS_eagle.app.v6 = \
	user/libuser.a    \
	driver/libdriver.a \
	user/ledctl/src/libuser_ledctl.a \
	user/user_uart/src/libuser_useruart.a

ifeq ($(USER),smartled)
    COMPONENTS_eagle.app.v6 += user/user_smartled/src/libuser_smartled.a
else ifeq ($(USER),lexyairpurifier)
    COMPONENTS_eagle.app.v6 += user/user_lexyairpurifier/src/libuser_lexyairpurifier.a
endif


LINKFLAGS_eagle.app.v6 = \
	-L$(SDK_PATH)/lib        \
	-Wl,--gc-sections   \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lcirom \
	-lgcc					\
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-lwpa	\
	-lmain	\
	-lfreertos	\
	-llwip	\
	-lssl	\
	-ljson  \
	-lsmartconfig	\
	-lspiffs	\
	$(DEP_LIBS_eagle.app.v6)					\
	-lcrypto \
	-lalink \
	-laws \
	-Wl,--end-group

DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) -I $(PDIR)include
INCLUDES += -I $(PDIR)user/user_uart/include
INCLUDES += -I $(PDIR)user/ledctl/include
ifeq ($(USER),smartled)
INCLUDES += -I $(PDIR)user/user_smartled/include
else ifeq ($(USER),lexyairpurifier)
INCLUDES += -I $(PDIR)user/user_lexyairpurifier/include
endif

sinclude $(SDK_PATH)/Makefile

.PHONY: FORCE
FORCE:

