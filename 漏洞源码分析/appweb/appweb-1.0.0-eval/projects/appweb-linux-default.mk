#
#   appweb-linux-default.mk -- Makefile to build Embedthis Appweb for linux
#

NAME                  := appweb
VERSION               := 1.0.0-eval
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= linux
CC                    ?= gcc
AR                    ?= ar
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

#
# Components
#
ME_COM_CGI            ?= 1
ME_COM_COMPILER       ?= 1
ME_COM_DIR            ?= 0
ME_COM_EJS            ?= 0
ME_COM_ESP            ?= 1
ME_COM_FAST           ?= 1
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_MBEDTLS        ?= 0
ME_COM_MDB            ?= 1
ME_COM_MPR            ?= 1
ME_COM_OPENSSL        ?= 1
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_PHP            ?= 0
ME_COM_PROXY          ?= 1
ME_COM_SQLITE         ?= 0
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_WATCHDOG       ?= 1


ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_ESP),1)
    ME_COM_MDB := 1
endif

#
# Settings
#
ME_AUTHOR             ?= \"Embedthis Software\"
ME_CERTS_GENDH        ?= 1
ME_COMPANY            ?= \"embedthis\"
ME_COMPAT             ?= 1
ME_COMPATIBLE         ?= \"1.0\"
ME_COMPILER_FORTIFY   ?= 1
ME_COMPILER_HAS_ATOMIC ?= 1
ME_COMPILER_HAS_ATOMIC64 ?= 1
ME_COMPILER_HAS_DOUBLE_BRACES ?= 1
ME_COMPILER_HAS_DYN_LOAD ?= 1
ME_COMPILER_HAS_LIB_EDIT ?= 0
ME_COMPILER_HAS_LIB_RT ?= 1
ME_COMPILER_HAS_MMU   ?= 1
ME_COMPILER_HAS_MTUNE ?= 1
ME_COMPILER_HAS_PAM   ?= 0
ME_COMPILER_HAS_STACK_PROTECTOR ?= 1
ME_COMPILER_HAS_SYNC  ?= 1
ME_COMPILER_HAS_SYNC64 ?= 1
ME_COMPILER_HAS_SYNC_CAS ?= 1
ME_COMPILER_HAS_UNNAMED_UNIONS ?= 1
ME_COMPILER_WARN64TO32 ?= 0
ME_COMPILER_WARN_UNUSED ?= 1
ME_CONFIG_FILE        ?= \"appweb.conf\"
ME_CONFIGURE          ?= \"me -d -q -platform linux-x64-default -configure . --with esp --with mdb --with cgi --with fast --with proxy -gen make\"
ME_CONFIGURED         ?= 1
ME_DEPRECATED_WARNINGS ?= 0
ME_DEPTH              ?= 1
ME_DESCRIPTION        ?= \"Embedthis Appweb\"
ME_ESP_CMD            ?= 1
ME_ESP_LEGACY         ?= 0
ME_ESP_MODULE         ?= 0
ME_ESP_NAME           ?= \"appweb-esp\"
ME_HTTP_BASIC         ?= 1
ME_HTTP_CACHE         ?= 1
ME_HTTP_CMD           ?= 1
ME_HTTP_DEFENSE       ?= 1
ME_HTTP_DIGEST        ?= 1
ME_HTTP_DIR           ?= 1
ME_HTTP_HTTP2         ?= 1
ME_HTTP_PAM           ?= 1
ME_HTTP_SENDFILE      ?= 1
ME_HTTP_UPLOAD        ?= 1
ME_HTTP_WEB_SOCKETS   ?= 1
ME_INTEGRATE          ?= 1
ME_MANIFEST           ?= \"installs/manifest.me\"
ME_MBEDTLS_COMPACT    ?= 1
ME_MPR_LOGGING        ?= 1
ME_MPR_OSLOG          ?= 0
ME_MPR_ROM_MOUNT      ?= \"/rom\"
ME_MPR_SSL_CACHE      ?= 512
ME_MPR_SSL_HANDSHAKES ?= 3
ME_MPR_SSL_LOG_LEVEL  ?= 6
ME_MPR_SSL_TICKET     ?= 1
ME_MPR_SSL_TIMEOUT    ?= 86400
ME_MPR_THREAD_LIMIT_BY_CORES ?= 1
ME_MPR_THREAD_STACK   ?= 0
ME_NAME               ?= \"appweb\"
ME_OPENSSL_VERSION    ?= \"1.0\"
ME_PARTS              ?= \"undefined\"
ME_PLATFORMS          ?= \"local\"
ME_PREFIXES           ?= \"install-prefixes\"
ME_ROM                ?= 0
ME_SERVER_ROOT        ?= \".\"
ME_TITLE              ?= \"Embedthis Appweb\"
ME_TUNE               ?= \"size\"
ME_VERSION            ?= \"1.0.0-eval\"
ME_WATCHDOG_NAME      ?= \"appman\"
ME_WEB_GROUP          ?= \"$(WEB_GROUP)\"
ME_WEB_USER           ?= \"$(WEB_USER)\"

CFLAGS                += -fPIC -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Wl,-z,relro,-z,now -Wl,--as-needed -Wl,--no-copy-dt-needed-entries -Wl,-z,noexecstatck -Wl,-z,noexecheap  -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_CGI=$(ME_COM_CGI) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_DIR=$(ME_COM_DIR) -DME_COM_EJS=$(ME_COM_EJS) -DME_COM_ESP=$(ME_COM_ESP) -DME_COM_FAST=$(ME_COM_FAST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_MDB=$(ME_COM_MDB) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_PHP=$(ME_COM_PHP) -DME_COM_PROXY=$(ME_COM_PROXY) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WATCHDOG=$(ME_COM_WATCHDOG) -DME_CERTS_GENDH=$(ME_CERTS_GENDH) -DME_ESP_CMD=$(ME_ESP_CMD) -DME_ESP_LEGACY=$(ME_ESP_LEGACY) -DME_ESP_MODULE=$(ME_ESP_MODULE) -DME_ESP_NAME=$(ME_ESP_NAME) -DME_HTTP_BASIC=$(ME_HTTP_BASIC) -DME_HTTP_CACHE=$(ME_HTTP_CACHE) -DME_HTTP_CMD=$(ME_HTTP_CMD) -DME_HTTP_DEFENSE=$(ME_HTTP_DEFENSE) -DME_HTTP_DIGEST=$(ME_HTTP_DIGEST) -DME_HTTP_DIR=$(ME_HTTP_DIR) -DME_HTTP_HTTP2=$(ME_HTTP_HTTP2) -DME_HTTP_PAM=$(ME_HTTP_PAM) -DME_HTTP_SENDFILE=$(ME_HTTP_SENDFILE) -DME_HTTP_UPLOAD=$(ME_HTTP_UPLOAD) -DME_HTTP_WEBSOCKETS=$(ME_HTTP_WEBSOCKETS) -DME_MBEDTLS_COMPACT=$(ME_MBEDTLS_COMPACT) -DME_MPR_ALLOC=$(ME_MPR_ALLOC) -DME_MPR_LOGGING=$(ME_MPR_LOGGING) -DME_MPR_OSLOG=$(ME_MPR_OSLOG) -DME_MPR_ROMMOUNT=$(ME_MPR_ROMMOUNT) -DME_MPR_SSL=$(ME_MPR_SSL) -DME_MPR_THREADLIMITBYCORES=$(ME_MPR_THREADLIMITBYCORES) -DME_MPR_THREADSTACK=$(ME_MPR_THREADSTACK) -DME_OPENSSL_VERSION=$(ME_OPENSSL_VERSION) -DME_WATCHDOG_NAME=$(ME_WATCHDOG_NAME) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS              += -L$(BUILD)/bin
LIBS                  += -lrt -ldl -lpthread -lm

DEBUG                 ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG=1
LDFLAGS-debug         ?= -g
DFLAGS-release        ?= 
CFLAGS-release        ?= -O2
LDFLAGS-release       ?= 
CFLAGS                += $(CFLAGS-$(DEBUG))
DFLAGS                += $(DFLAGS-$(DEBUG))
LDFLAGS               += $(LDFLAGS-$(DEBUG))

ME_ROOT_PREFIX        ?= 
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local
ME_DATA_PREFIX        ?= $(ME_ROOT_PREFIX)/
ME_STATE_PREFIX       ?= $(ME_ROOT_PREFIX)/var
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)/lib/$(NAME)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)/$(VERSION)
ME_BIN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/bin
ME_INC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/include
ME_LIB_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/lib
ME_MAN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/share/man
ME_SBIN_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local/sbin
ME_ETC_PREFIX         ?= $(ME_ROOT_PREFIX)/etc/$(NAME)
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)

WEB_USER              ?= $(shell egrep 'www-data|_www|nobody' /etc/passwd | sed 's^:.*^^' |  tail -1)
WEB_GROUP             ?= $(shell egrep 'www-data|_www|nobody|nogroup' /etc/group | sed 's^:.*^^' |  tail -1)

TARGETS               += $(BUILD)/bin/appweb
TARGETS               += $(BUILD)/bin/authpass
ifeq ($(ME_COM_ESP),1)
    TARGETS           += $(BUILD)/bin/appweb-esp
endif
ifeq ($(ME_COM_ESP),1)
    TARGETS           += $(BUILD)/.extras-modified
endif
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(BUILD)/bin/http
endif
TARGETS               += $(BUILD)/.install-roots-modified
ifeq ($(ME_COM_SQLITE),1)
    TARGETS           += $(BUILD)/bin/libsql.so
endif
TARGETS               += $(BUILD)/bin/makerom
TARGETS               += $(BUILD)/bin/server
TARGETS               += src/server/cache
TARGETS               += $(BUILD)/bin/appman


DEPEND := $(strip $(wildcard ./projects/depend.mk))
ifneq ($(DEPEND),)
include $(DEPEND)
endif

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(BUILD)/bin ] && mkdir -p $(BUILD)/bin; true
	@[ ! -x $(BUILD)/inc ] && mkdir -p $(BUILD)/inc; true
	@[ ! -x $(BUILD)/obj ] && mkdir -p $(BUILD)/obj; true
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/appweb-linux-$(PROFILE)-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/appweb-linux-$(PROFILE)-me.h >/dev/null ; then\
		cp projects/appweb-linux-$(PROFILE)-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build" ; \
			echo "   [Warning] Previous build command: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo "$(MAKEFLAGS)" >$(BUILD)/.makeflags

clean:
	rm -f "$(BUILD)/obj/appweb.o"
	rm -f "$(BUILD)/obj/authpass.o"
	rm -f "$(BUILD)/obj/cgiHandler.o"
	rm -f "$(BUILD)/obj/cgiProgram.o"
	rm -f "$(BUILD)/obj/config.o"
	rm -f "$(BUILD)/obj/convenience.o"
	rm -f "$(BUILD)/obj/esp.o"
	rm -f "$(BUILD)/obj/espHandler.o"
	rm -f "$(BUILD)/obj/espLib.o"
	rm -f "$(BUILD)/obj/fastHandler.o"
	rm -f "$(BUILD)/obj/fastProgram.o"
	rm -f "$(BUILD)/obj/http.o"
	rm -f "$(BUILD)/obj/httpLib.o"
	rm -f "$(BUILD)/obj/makerom.o"
	rm -f "$(BUILD)/obj/mpr-version.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/proxyHandler.o"
	rm -f "$(BUILD)/obj/rom.o"
	rm -f "$(BUILD)/obj/server.o"
	rm -f "$(BUILD)/obj/sqlite.o"
	rm -f "$(BUILD)/obj/sqlite3.o"
	rm -f "$(BUILD)/obj/testHandler.o"
	rm -f "$(BUILD)/obj/watchdog.o"
	rm -f "$(BUILD)/bin/appweb"
	rm -f "$(BUILD)/bin/authpass"
	rm -f "$(BUILD)/bin/appweb-esp"
	rm -f "$(BUILD)/.extras-modified"
	rm -f "$(BUILD)/bin/http"
	rm -f "$(BUILD)/.install-roots-modified"
	rm -f "$(BUILD)/bin/libappweb.so"
	rm -f "$(BUILD)/bin/libesp.so"
	rm -f "$(BUILD)/bin/libhttp.so"
	rm -f "$(BUILD)/bin/libmpr.so"
	rm -f "$(BUILD)/bin/libmpr-version.a"
	rm -f "$(BUILD)/bin/libpcre.so"
	rm -f "$(BUILD)/bin/libsql.so"
	rm -f "$(BUILD)/bin/makerom"
	rm -f "$(BUILD)/bin/server"
	rm -f "$(BUILD)/bin/appman"

clobber: clean
	rm -fr ./$(BUILD)

#
#   me.h
#

$(BUILD)/inc/me.h: $(DEPS_1)

#
#   osdep.h
#
DEPS_2 += src/osdep/osdep.h
DEPS_2 += $(BUILD)/inc/me.h

$(BUILD)/inc/osdep.h: $(DEPS_2)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_3 += src/mpr/mpr.h
DEPS_3 += $(BUILD)/inc/me.h
DEPS_3 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_3)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_4 += src/http/http.h
DEPS_4 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_4)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/http/http.h $(BUILD)/inc/http.h

#
#   appweb.h
#
DEPS_5 += src/appweb.h
DEPS_5 += $(BUILD)/inc/osdep.h
DEPS_5 += $(BUILD)/inc/mpr.h
DEPS_5 += $(BUILD)/inc/http.h

$(BUILD)/inc/appweb.h: $(DEPS_5)
	@echo '      [Copy] $(BUILD)/inc/appweb.h'
	mkdir -p "$(BUILD)/inc"
	cp src/appweb.h $(BUILD)/inc/appweb.h

#
#   config.h
#

$(BUILD)/inc/config.h: $(DEPS_6)

#
#   customize.h
#
DEPS_7 += src/customize.h

$(BUILD)/inc/customize.h: $(DEPS_7)
	@echo '      [Copy] $(BUILD)/inc/customize.h'
	mkdir -p "$(BUILD)/inc"
	cp src/customize.h $(BUILD)/inc/customize.h

#
#   esp.h
#
DEPS_8 += src/esp/esp.h
DEPS_8 += $(BUILD)/inc/me.h
DEPS_8 += $(BUILD)/inc/osdep.h
DEPS_8 += $(BUILD)/inc/http.h

$(BUILD)/inc/esp.h: $(DEPS_8)
	@echo '      [Copy] $(BUILD)/inc/esp.h'
	mkdir -p "$(BUILD)/inc"
	cp src/esp/esp.h $(BUILD)/inc/esp.h

#
#   mpr-version.h
#
DEPS_9 += src/mpr-version/mpr-version.h
DEPS_9 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/mpr-version.h: $(DEPS_9)
	@echo '      [Copy] $(BUILD)/inc/mpr-version.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr-version/mpr-version.h $(BUILD)/inc/mpr-version.h

#
#   pcre.h
#
DEPS_10 += src/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_10)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   sqlite3.h
#
DEPS_11 += src/sqlite/sqlite3.h

$(BUILD)/inc/sqlite3.h: $(DEPS_11)
	@echo '      [Copy] $(BUILD)/inc/sqlite3.h'
	mkdir -p "$(BUILD)/inc"
	cp src/sqlite/sqlite3.h $(BUILD)/inc/sqlite3.h

#
#   sqlite3rtree.h
#

$(BUILD)/inc/sqlite3rtree.h: $(DEPS_12)

#
#   windows.h
#

$(BUILD)/inc/windows.h: $(DEPS_13)

#
#   appweb.o
#
DEPS_14 += $(BUILD)/inc/appweb.h

$(BUILD)/obj/appweb.o: \
    src/server/appweb.c $(DEPS_14)
	@echo '   [Compile] $(BUILD)/obj/appweb.o'
	$(CC) -c -o $(BUILD)/obj/appweb.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/server/appweb.c

#
#   authpass.o
#
DEPS_15 += $(BUILD)/inc/appweb.h

$(BUILD)/obj/authpass.o: \
    src/utils/authpass.c $(DEPS_15)
	@echo '   [Compile] $(BUILD)/obj/authpass.o'
	$(CC) -c -o $(BUILD)/obj/authpass.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/utils/authpass.c

#
#   appweb.h
#

src/appweb.h: $(DEPS_16)

#
#   cgiHandler.o
#
DEPS_17 += src/appweb.h

$(BUILD)/obj/cgiHandler.o: \
    src/modules/cgiHandler.c $(DEPS_17)
	@echo '   [Compile] $(BUILD)/obj/cgiHandler.o'
	$(CC) -c -o $(BUILD)/obj/cgiHandler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/modules/cgiHandler.c

#
#   cgiProgram.o
#

$(BUILD)/obj/cgiProgram.o: \
    src/utils/cgiProgram.c $(DEPS_18)
	@echo '   [Compile] $(BUILD)/obj/cgiProgram.o'
	$(CC) -c -o $(BUILD)/obj/cgiProgram.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/utils/cgiProgram.c

#
#   config.o
#
DEPS_19 += src/appweb.h
DEPS_19 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/config.o: \
    src/config.c $(DEPS_19)
	@echo '   [Compile] $(BUILD)/obj/config.o'
	$(CC) -c -o $(BUILD)/obj/config.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/config.c

#
#   convenience.o
#
DEPS_20 += src/appweb.h

$(BUILD)/obj/convenience.o: \
    src/convenience.c $(DEPS_20)
	@echo '   [Compile] $(BUILD)/obj/convenience.o'
	$(CC) -c -o $(BUILD)/obj/convenience.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/convenience.c

#
#   esp.h
#

src/esp/esp.h: $(DEPS_21)

#
#   esp.o
#
DEPS_22 += src/esp/esp.h
DEPS_22 += $(BUILD)/inc/mpr-version.h

$(BUILD)/obj/esp.o: \
    src/esp/esp.c $(DEPS_22)
	@echo '   [Compile] $(BUILD)/obj/esp.o'
	$(CC) -c -o $(BUILD)/obj/esp.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/esp/esp.c

#
#   espHandler.o
#
DEPS_23 += src/appweb.h
DEPS_23 += $(BUILD)/inc/esp.h

$(BUILD)/obj/espHandler.o: \
    src/modules/espHandler.c $(DEPS_23)
	@echo '   [Compile] $(BUILD)/obj/espHandler.o'
	$(CC) -c -o $(BUILD)/obj/espHandler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/modules/espHandler.c

#
#   espLib.o
#
DEPS_24 += src/esp/esp.h
DEPS_24 += $(BUILD)/inc/pcre.h
DEPS_24 += $(BUILD)/inc/http.h

$(BUILD)/obj/espLib.o: \
    src/esp/espLib.c $(DEPS_24)
	@echo '   [Compile] $(BUILD)/obj/espLib.o'
	$(CC) -c -o $(BUILD)/obj/espLib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/esp/espLib.c

#
#   fastHandler.o
#
DEPS_25 += src/appweb.h

$(BUILD)/obj/fastHandler.o: \
    src/modules/fastHandler.c $(DEPS_25)
	@echo '   [Compile] $(BUILD)/obj/fastHandler.o'
	$(CC) -c -o $(BUILD)/obj/fastHandler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/modules/fastHandler.c

#
#   fcgiapp.h
#

../../../..$(ME_INC_PREFIX)/fcgiapp.h: $(DEPS_26)

#
#   fastProgram.o
#
DEPS_27 += ../../../..$(ME_INC_PREFIX)/fcgiapp.h

$(BUILD)/obj/fastProgram.o: \
    src/utils/fastProgram.c $(DEPS_27)
	@echo '   [Compile] $(BUILD)/obj/fastProgram.o'
	$(CC) -c -o $(BUILD)/obj/fastProgram.o $(CFLAGS) $(DFLAGS) "-I$(ME_INC_PREFIX)" src/utils/fastProgram.c

#
#   http.h
#

src/http/http.h: $(DEPS_28)

#
#   http.o
#
DEPS_29 += src/http/http.h

$(BUILD)/obj/http.o: \
    src/http/http.c $(DEPS_29)
	@echo '   [Compile] $(BUILD)/obj/http.o'
	$(CC) -c -o $(BUILD)/obj/http.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/http/http.c

#
#   httpLib.o
#
DEPS_30 += src/http/http.h
DEPS_30 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/httpLib.o: \
    src/http/httpLib.c $(DEPS_30)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/http/httpLib.c

#
#   makerom.o
#
DEPS_31 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/makerom.o: \
    src/makerom/makerom.c $(DEPS_31)
	@echo '   [Compile] $(BUILD)/obj/makerom.o'
	$(CC) -c -o $(BUILD)/obj/makerom.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/makerom/makerom.c

#
#   mpr-version.h
#

src/mpr-version/mpr-version.h: $(DEPS_32)

#
#   mpr-version.o
#
DEPS_33 += src/mpr-version/mpr-version.h
DEPS_33 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/mpr-version.o: \
    src/mpr-version/mpr-version.c $(DEPS_33)
	@echo '   [Compile] $(BUILD)/obj/mpr-version.o'
	$(CC) -c -o $(BUILD)/obj/mpr-version.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/mpr-version/mpr-version.c

#
#   mpr.h
#

src/mpr/mpr.h: $(DEPS_34)

#
#   mprLib.o
#
DEPS_35 += src/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/mpr/mprLib.c $(DEPS_35)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c -o $(BUILD)/obj/mprLib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/mpr/mprLib.c

#
#   pcre.h
#

src/pcre/pcre.h: $(DEPS_36)

#
#   pcre.o
#
DEPS_37 += $(BUILD)/inc/me.h
DEPS_37 += src/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/pcre/pcre.c $(DEPS_37)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c -o $(BUILD)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/pcre/pcre.c

#
#   proxyHandler.o
#
DEPS_38 += src/appweb.h

$(BUILD)/obj/proxyHandler.o: \
    src/modules/proxyHandler.c $(DEPS_38)
	@echo '   [Compile] $(BUILD)/obj/proxyHandler.o'
	$(CC) -c -o $(BUILD)/obj/proxyHandler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/modules/proxyHandler.c

#
#   rom.o
#
DEPS_39 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/rom.o: \
    src/rom.c $(DEPS_39)
	@echo '   [Compile] $(BUILD)/obj/rom.o'
	$(CC) -c -o $(BUILD)/obj/rom.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/rom.c

#
#   server.o
#
DEPS_40 += src/http/http.h

$(BUILD)/obj/server.o: \
    src/http/server.c $(DEPS_40)
	@echo '   [Compile] $(BUILD)/obj/server.o'
	$(CC) -c -o $(BUILD)/obj/server.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/http/server.c

#
#   sqlite3.h
#

src/sqlite/sqlite3.h: $(DEPS_41)

#
#   sqlite.o
#
DEPS_42 += $(BUILD)/inc/me.h
DEPS_42 += src/sqlite/sqlite3.h
DEPS_42 += $(BUILD)/inc/windows.h

$(BUILD)/obj/sqlite.o: \
    src/sqlite/sqlite.c $(DEPS_42)
	@echo '   [Compile] $(BUILD)/obj/sqlite.o'
	$(CC) -c -o $(BUILD)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/sqlite/sqlite.c

#
#   sqlite3.o
#
DEPS_43 += $(BUILD)/inc/me.h
DEPS_43 += src/sqlite/sqlite3.h
DEPS_43 += $(BUILD)/inc/config.h
DEPS_43 += $(BUILD)/inc/windows.h
DEPS_43 += $(BUILD)/inc/sqlite3rtree.h

$(BUILD)/obj/sqlite3.o: \
    src/sqlite/sqlite3.c $(DEPS_43)
	@echo '   [Compile] $(BUILD)/obj/sqlite3.o'
	$(CC) -c -o $(BUILD)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/sqlite/sqlite3.c

#
#   testHandler.o
#
DEPS_44 += src/appweb.h

$(BUILD)/obj/testHandler.o: \
    src/modules/testHandler.c $(DEPS_44)
	@echo '   [Compile] $(BUILD)/obj/testHandler.o'
	$(CC) -c -o $(BUILD)/obj/testHandler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/modules/testHandler.c

#
#   watchdog.o
#
DEPS_45 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/watchdog.o: \
    src/watchdog/watchdog.c $(DEPS_45)
	@echo '   [Compile] $(BUILD)/obj/watchdog.o'
	$(CC) -c -o $(BUILD)/obj/watchdog.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 $(IFLAGS) src/watchdog/watchdog.c

#
#   libmpr
#
DEPS_46 += $(BUILD)/inc/osdep.h
DEPS_46 += $(BUILD)/inc/mpr.h
DEPS_46 += $(BUILD)/obj/mprLib.o

ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_46 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_46 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_46 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_46 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_46 += -lcrypto
endif

$(BUILD)/bin/libmpr.so: $(DEPS_46)
	@echo '      [Link] $(BUILD)/bin/libmpr.so'
	$(CC) -shared -o $(BUILD)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/mprLib.o" $(LIBPATHS_46) $(LIBS_46) $(LIBS_46) $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_47 += $(BUILD)/inc/pcre.h
DEPS_47 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.so: $(DEPS_47)
	@echo '      [Link] $(BUILD)/bin/libpcre.so'
	$(CC) -shared -o $(BUILD)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_48 += $(BUILD)/bin/libmpr.so
ifeq ($(ME_COM_PCRE),1)
    DEPS_48 += $(BUILD)/bin/libpcre.so
endif
DEPS_48 += $(BUILD)/inc/http.h
DEPS_48 += $(BUILD)/obj/httpLib.o

LIBS_48 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_48 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_48 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_48 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_48 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_48 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_48 += -lpcre
endif

$(BUILD)/bin/libhttp.so: $(DEPS_48)
	@echo '      [Link] $(BUILD)/bin/libhttp.so'
	$(CC) -shared -o $(BUILD)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/httpLib.o" $(LIBPATHS_48) $(LIBS_48) $(LIBS_48) $(LIBS) 
endif

#
#   libmpr-version
#
DEPS_49 += $(BUILD)/inc/mpr-version.h
DEPS_49 += $(BUILD)/obj/mpr-version.o

$(BUILD)/bin/libmpr-version.a: $(DEPS_49)
	@echo '      [Link] $(BUILD)/bin/libmpr-version.a'
	$(AR) -cr $(BUILD)/bin/libmpr-version.a "$(BUILD)/obj/mpr-version.o"

ifeq ($(ME_COM_ESP),1)
#
#   libesp
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_50 += $(BUILD)/bin/libhttp.so
endif
DEPS_50 += $(BUILD)/bin/libmpr-version.a
ifeq ($(ME_COM_SQLITE),1)
    DEPS_50 += $(BUILD)/bin/libsql.so
endif
DEPS_50 += $(BUILD)/inc/esp.h
DEPS_50 += $(BUILD)/obj/espLib.o
ifeq ($(ME_COM_SQLITE),1)
    DEPS_50 += $(BUILD)/bin/libsql.so
endif

LIBS_50 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_50 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_50 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_50 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_50 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_50 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_50 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_50 += -lhttp
endif
LIBS_50 += -lmpr-version
ifeq ($(ME_COM_SQLITE),1)
    LIBS_50 += -lsql
endif

$(BUILD)/bin/libesp.so: $(DEPS_50)
	@echo '      [Link] $(BUILD)/bin/libesp.so'
	$(CC) -shared -o $(BUILD)/bin/libesp.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/espLib.o" $(LIBPATHS_50) $(LIBS_50) $(LIBS_50) $(LIBS) 
endif

#
#   libappweb
#
ifeq ($(ME_COM_ESP),1)
    DEPS_51 += $(BUILD)/bin/libesp.so
endif
ifeq ($(ME_COM_SQLITE),1)
    DEPS_51 += $(BUILD)/bin/libsql.so
endif
ifeq ($(ME_COM_HTTP),1)
    DEPS_51 += $(BUILD)/bin/libhttp.so
endif
DEPS_51 += $(BUILD)/bin/libmpr.so
DEPS_51 += $(BUILD)/bin/libmpr-version.a
DEPS_51 += $(BUILD)/inc/appweb.h
DEPS_51 += $(BUILD)/inc/customize.h
DEPS_51 += $(BUILD)/obj/config.o
DEPS_51 += $(BUILD)/obj/convenience.o
DEPS_51 += $(BUILD)/obj/cgiHandler.o
DEPS_51 += $(BUILD)/obj/espHandler.o
DEPS_51 += $(BUILD)/obj/fastHandler.o
DEPS_51 += $(BUILD)/obj/proxyHandler.o
DEPS_51 += $(BUILD)/obj/testHandler.o
DEPS_51 += $(BUILD)/obj/rom.o

LIBS_51 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_51 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_51 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_51 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_51 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_51 += -lhttp
endif
LIBS_51 += -lmpr-version
ifeq ($(ME_COM_ESP),1)
    LIBS_51 += -lesp
endif
ifeq ($(ME_COM_SQLITE),1)
    LIBS_51 += -lsql
endif

$(BUILD)/bin/libappweb.so: $(DEPS_51)
	@echo '      [Link] $(BUILD)/bin/libappweb.so'
	$(CC) -shared -o $(BUILD)/bin/libappweb.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/config.o" "$(BUILD)/obj/convenience.o" "$(BUILD)/obj/cgiHandler.o" "$(BUILD)/obj/espHandler.o" "$(BUILD)/obj/fastHandler.o" "$(BUILD)/obj/proxyHandler.o" "$(BUILD)/obj/testHandler.o" "$(BUILD)/obj/rom.o" $(LIBPATHS_51) $(LIBS_51) $(LIBS_51) $(LIBS) 

#
#   appweb
#
DEPS_52 += $(BUILD)/bin/libappweb.so
DEPS_52 += $(BUILD)/obj/appweb.o

LIBS_52 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_52 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_52 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_52 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_52 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_52 += -lhttp
endif
LIBS_52 += -lmpr-version
ifeq ($(ME_COM_ESP),1)
    LIBS_52 += -lesp
endif
ifeq ($(ME_COM_SQLITE),1)
    LIBS_52 += -lsql
endif
LIBS_52 += -lappweb

$(BUILD)/bin/appweb: $(DEPS_52)
	@echo '      [Link] $(BUILD)/bin/appweb'
	$(CC) -o $(BUILD)/bin/appweb $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/appweb.o" $(LIBPATHS_52) $(LIBS_52) $(LIBS_52) $(LIBS) $(LIBS) 

#
#   authpass
#
DEPS_53 += $(BUILD)/bin/libappweb.so
DEPS_53 += $(BUILD)/obj/authpass.o

LIBS_53 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_53 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_53 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_53 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_53 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_53 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_53 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_53 += -lhttp
endif
LIBS_53 += -lmpr-version
ifeq ($(ME_COM_ESP),1)
    LIBS_53 += -lesp
endif
ifeq ($(ME_COM_SQLITE),1)
    LIBS_53 += -lsql
endif
LIBS_53 += -lappweb

$(BUILD)/bin/authpass: $(DEPS_53)
	@echo '      [Link] $(BUILD)/bin/authpass'
	$(CC) -o $(BUILD)/bin/authpass $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/authpass.o" $(LIBPATHS_53) $(LIBS_53) $(LIBS_53) $(LIBS) $(LIBS) 

ifeq ($(ME_COM_ESP),1)
#
#   espcmd
#
DEPS_54 += $(BUILD)/bin/libesp.so
ifeq ($(ME_COM_SQLITE),1)
    DEPS_54 += $(BUILD)/bin/libsql.so
endif
DEPS_54 += $(BUILD)/obj/esp.o

LIBS_54 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_54 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_54 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_54 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_54 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_54 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_54 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_54 += -lhttp
endif
LIBS_54 += -lmpr-version
LIBS_54 += -lesp
ifeq ($(ME_COM_SQLITE),1)
    LIBS_54 += -lsql
endif

$(BUILD)/bin/appweb-esp: $(DEPS_54)
	@echo '      [Link] $(BUILD)/bin/appweb-esp'
	$(CC) -o $(BUILD)/bin/appweb-esp $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/esp.o" $(LIBPATHS_54) $(LIBS_54) $(LIBS_54) $(LIBS) $(LIBS) 
endif

ifeq ($(ME_COM_ESP),1)
#
#   extras
#
DEPS_55 += src/esp/esp-compile.json
DEPS_55 += src/esp/vcvars.bat

$(BUILD)/.extras-modified: $(DEPS_55)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/esp/esp-compile.json $(BUILD)/bin/esp-compile.json
	cp src/esp/vcvars.bat $(BUILD)/bin/vcvars.bat
	touch "$(BUILD)/.extras-modified"
endif

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_56 += $(BUILD)/bin/libhttp.so
DEPS_56 += $(BUILD)/obj/http.o

LIBS_56 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_56 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_56 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_56 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_56 += -lpcre
endif
LIBS_56 += -lhttp

$(BUILD)/bin/http: $(DEPS_56)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/http.o" $(LIBPATHS_56) $(LIBS_56) $(LIBS_56) $(LIBS) $(LIBS) 
endif

#
#   installPrep
#

installPrep: $(DEPS_57)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with sudo." ; \
	exit 255 ; \
	fi

#
#   install-roots
#
DEPS_58 += certs/roots.crt

$(BUILD)/.install-roots-modified: $(DEPS_58)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp certs/roots.crt $(BUILD)/bin/roots.crt
	touch "$(BUILD)/.install-roots-modified"

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_59 += $(BUILD)/inc/sqlite3.h
DEPS_59 += $(BUILD)/obj/sqlite3.o

$(BUILD)/bin/libsql.so: $(DEPS_59)
	@echo '      [Link] $(BUILD)/bin/libsql.so'
	$(CC) -shared -o $(BUILD)/bin/libsql.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/sqlite3.o" $(LIBS) 
endif

#
#   makerom
#
DEPS_60 += $(BUILD)/bin/libmpr.so
DEPS_60 += $(BUILD)/obj/makerom.o

LIBS_60 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_60 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_60 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_60 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_60 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_60 += -lcrypto
endif

$(BUILD)/bin/makerom: $(DEPS_60)
	@echo '      [Link] $(BUILD)/bin/makerom'
	$(CC) -o $(BUILD)/bin/makerom $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/makerom.o" $(LIBPATHS_60) $(LIBS_60) $(LIBS_60) $(LIBS) $(LIBS) 

#
#   server
#
ifeq ($(ME_COM_HTTP),1)
    DEPS_61 += $(BUILD)/bin/libhttp.so
endif
DEPS_61 += $(BUILD)/obj/server.o

LIBS_61 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_61 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_61 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_61 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_61 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_61 += -lcrypto
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_61 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_61 += -lhttp
endif

$(BUILD)/bin/server: $(DEPS_61)
	@echo '      [Link] $(BUILD)/bin/server'
	$(CC) -o $(BUILD)/bin/server $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/server.o" $(LIBPATHS_61) $(LIBS_61) $(LIBS_61) $(LIBS) $(LIBS) 

#
#   server-cache
#

src/server/cache: $(DEPS_62)
	( \
	cd src/server; \
	mkdir -p "cache" ; \
	)

ifeq ($(ME_COM_WATCHDOG),1)
#
#   watchdog
#
DEPS_63 += $(BUILD)/bin/libmpr.so
DEPS_63 += $(BUILD)/obj/watchdog.o

LIBS_63 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedtls
endif
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedcrypto
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_63 += -lmbedx509
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_63 += -lssl
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_63 += -lcrypto
endif

$(BUILD)/bin/appman: $(DEPS_63)
	@echo '      [Link] $(BUILD)/bin/appman'
	$(CC) -o $(BUILD)/bin/appman $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/watchdog.o" $(LIBPATHS_63) $(LIBS_63) $(LIBS_63) $(LIBS) $(LIBS) 
endif

#
#   stop
#
DEPS_64 += compile

stop: $(DEPS_64)
	@./$(BUILD)/bin/appman stop disable uninstall >/dev/null 2>&1 ; true

#
#   installBinary
#

installBinary: $(DEPS_65)
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "$(VERSION)" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	chmod 755 "$(ME_MAN_PREFIX)/man1" ; \
	mkdir -p "$(ME_LOG_PREFIX)" ; \
	chmod 755 "$(ME_LOG_PREFIX)" ; \
	[ `id -u` = 0 ] && chown $(WEB_USER):$(WEB_GROUP) "$(ME_LOG_PREFIX)"; true ; \
	mkdir -p "$(ME_CACHE_PREFIX)" ; \
	chmod 755 "$(ME_CACHE_PREFIX)" ; \
	[ `id -u` = 0 ] && chown $(WEB_USER):$(WEB_GROUP) "$(ME_CACHE_PREFIX)"; true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/appweb $(ME_VAPP_PREFIX)/bin/appweb ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/appweb" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/appweb" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/appweb" "$(ME_BIN_PREFIX)/appweb" ; \
	if [ "$(ME_COM_SSL)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp certs/roots.crt $(ME_VAPP_PREFIX)/bin/roots.crt ; \
	fi ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp src/server/mime.types $(ME_ETC_PREFIX)/mime.types ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp src/server/appweb.conf $(ME_ETC_PREFIX)/appweb.conf ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp src/server/esp.json $(ME_ETC_PREFIX)/esp.json ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp src/server/sample.conf $(ME_ETC_PREFIX)/sample.conf ; \
	echo 'set LOG_DIR "$(ME_LOG_PREFIX)"\nset CACHE_DIR "$(ME_CACHE_PREFIX)"\nDocuments "$(ME_WEB_PREFIX)"\nListen 80\n<if SSL_MODULE>\nListenSecure 443\n</if>\n' >$(ME_ETC_PREFIX)/install.conf ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libappweb.so $(ME_VAPP_PREFIX)/bin/libappweb.so ; \
	cp $(BUILD)/bin/libesp.so $(ME_VAPP_PREFIX)/bin/libesp.so ; \
	cp $(BUILD)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(BUILD)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(BUILD)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	mkdir -p "$(ME_WEB_PREFIX)" ; \
	mkdir -p "$(ME_WEB_PREFIX)/bench" ; \
	cp src/server/web/bench/1b.html $(ME_WEB_PREFIX)/bench/1b.html ; \
	cp src/server/web/bench/4k.html $(ME_WEB_PREFIX)/bench/4k.html ; \
	cp src/server/web/bench/64k.html $(ME_WEB_PREFIX)/bench/64k.html ; \
	cp src/server/web/favicon.ico $(ME_WEB_PREFIX)/favicon.ico ; \
	mkdir -p "$(ME_WEB_PREFIX)/icons" ; \
	cp src/server/web/icons/back.gif $(ME_WEB_PREFIX)/icons/back.gif ; \
	cp src/server/web/icons/blank.gif $(ME_WEB_PREFIX)/icons/blank.gif ; \
	cp src/server/web/icons/compressed.gif $(ME_WEB_PREFIX)/icons/compressed.gif ; \
	cp src/server/web/icons/folder.gif $(ME_WEB_PREFIX)/icons/folder.gif ; \
	cp src/server/web/icons/parent.gif $(ME_WEB_PREFIX)/icons/parent.gif ; \
	cp src/server/web/icons/space.gif $(ME_WEB_PREFIX)/icons/space.gif ; \
	cp src/server/web/icons/text.gif $(ME_WEB_PREFIX)/icons/text.gif ; \
	cp src/server/web/iehacks.css $(ME_WEB_PREFIX)/iehacks.css ; \
	mkdir -p "$(ME_WEB_PREFIX)/images" ; \
	cp src/server/web/images/banner.jpg $(ME_WEB_PREFIX)/images/banner.jpg ; \
	cp src/server/web/images/bottomShadow.jpg $(ME_WEB_PREFIX)/images/bottomShadow.jpg ; \
	cp src/server/web/images/shadow.jpg $(ME_WEB_PREFIX)/images/shadow.jpg ; \
	cp src/server/web/index.html $(ME_WEB_PREFIX)/index.html ; \
	cp src/server/web/min-index.html $(ME_WEB_PREFIX)/min-index.html ; \
	cp src/server/web/print.css $(ME_WEB_PREFIX)/print.css ; \
	cp src/server/web/screen.css $(ME_WEB_PREFIX)/screen.css ; \
	mkdir -p "$(ME_WEB_PREFIX)/test" ; \
	cp src/server/web/test/bench.html $(ME_WEB_PREFIX)/test/bench.html ; \
	cp src/server/web/test/index.html $(ME_WEB_PREFIX)/test/index.html ; \
	cp src/server/web/test/test.cgi $(ME_WEB_PREFIX)/test/test.cgi ; \
	cp src/server/web/test/test.esp $(ME_WEB_PREFIX)/test/test.esp ; \
	cp src/server/web/test/test.html $(ME_WEB_PREFIX)/test/test.html ; \
	cp src/server/web/test/test.pl $(ME_WEB_PREFIX)/test/test.pl ; \
	cp src/server/web/test/test.py $(ME_WEB_PREFIX)/test/test.py ; \
	mkdir -p "$(ME_WEB_PREFIX)/test" ; \
	cp src/server/web/test/test.cgi $(ME_WEB_PREFIX)/test/test.cgi ; \
	chmod 755 "$(ME_WEB_PREFIX)/test/test.cgi" ; \
	cp src/server/web/test/test.pl $(ME_WEB_PREFIX)/test/test.pl ; \
	chmod 755 "$(ME_WEB_PREFIX)/test/test.pl" ; \
	cp src/server/web/test/test.py $(ME_WEB_PREFIX)/test/test.py ; \
	chmod 755 "$(ME_WEB_PREFIX)/test/test.py" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/appman $(ME_VAPP_PREFIX)/bin/appman ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/appman" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/appman" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/appman" "$(ME_BIN_PREFIX)/appman" ; \
	mkdir -p "$(ME_ROOT_PREFIX)/etc/init.d" ; \
	cp installs/linux/appweb.init $(ME_ROOT_PREFIX)/etc/init.d/appweb ; \
	chmod 755 "$(ME_ROOT_PREFIX)/etc/init.d/appweb" ; \
	if [ "$(ME_COM_ESP)" = 1 ]; then true ; \
	fi ; \
	if [ "$(ME_COM_ESP)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/esp-compile.json $(ME_VAPP_PREFIX)/bin/esp-compile.json ; \
	cp $(BUILD)/bin/vcvars.bat $(ME_VAPP_PREFIX)/bin/vcvars.bat ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/http $(ME_VAPP_PREFIX)/bin/http ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/http" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/http" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/http" "$(ME_BIN_PREFIX)/http" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp $(BUILD)/inc/me.h $(ME_VAPP_PREFIX)/inc/me.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/me.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/me.h" "$(ME_INC_PREFIX)/appweb/me.h" ; \
	cp src/osdep/osdep.h $(ME_VAPP_PREFIX)/inc/osdep.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/osdep.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/osdep.h" "$(ME_INC_PREFIX)/appweb/osdep.h" ; \
	cp src/appweb.h $(ME_VAPP_PREFIX)/inc/appweb.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/appweb.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/appweb.h" "$(ME_INC_PREFIX)/appweb/appweb.h" ; \
	cp src/customize.h $(ME_VAPP_PREFIX)/inc/customize.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/customize.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/customize.h" "$(ME_INC_PREFIX)/appweb/customize.h" ; \
	cp src/http/http.h $(ME_VAPP_PREFIX)/inc/http.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/http.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/http.h" "$(ME_INC_PREFIX)/appweb/http.h" ; \
	cp src/mpr/mpr.h $(ME_VAPP_PREFIX)/inc/mpr.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/mpr.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mpr.h" "$(ME_INC_PREFIX)/appweb/mpr.h" ; \
	cp src/pcre/pcre.h $(ME_VAPP_PREFIX)/inc/pcre.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/pcre.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/pcre.h" "$(ME_INC_PREFIX)/appweb/pcre.h" ; \
	if [ "$(ME_COM_ESP)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp src/esp/esp.h $(ME_VAPP_PREFIX)/inc/esp.h ; \
	mkdir -p "$(ME_INC_PREFIX)/appweb" ; \
	rm -f "$(ME_INC_PREFIX)/appweb/esp.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/esp.h" "$(ME_INC_PREFIX)/appweb/esp.h" ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man1" ; \
	cp doc/man/appman.1 $(ME_VAPP_PREFIX)/doc/man1/appman.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/appman.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/appman.1" "$(ME_MAN_PREFIX)/man1/appman.1" ; \
	cp doc/man/appweb.1 $(ME_VAPP_PREFIX)/doc/man1/appweb.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/appweb.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/appweb.1" "$(ME_MAN_PREFIX)/man1/appweb.1" ; \
	cp doc/man/appwebMonitor.1 $(ME_VAPP_PREFIX)/doc/man1/appwebMonitor.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/appwebMonitor.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/appwebMonitor.1" "$(ME_MAN_PREFIX)/man1/appwebMonitor.1" ; \
	cp doc/man/authpass.1 $(ME_VAPP_PREFIX)/doc/man1/authpass.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/authpass.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/authpass.1" "$(ME_MAN_PREFIX)/man1/authpass.1" ; \
	cp doc/man/esp.1 $(ME_VAPP_PREFIX)/doc/man1/esp.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/esp.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/esp.1" "$(ME_MAN_PREFIX)/man1/esp.1" ; \
	cp doc/man/http.1 $(ME_VAPP_PREFIX)/doc/man1/http.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/http.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/http.1" "$(ME_MAN_PREFIX)/man1/http.1" ; \
	cp doc/man/makerom.1 $(ME_VAPP_PREFIX)/doc/man1/makerom.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/makerom.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/makerom.1" "$(ME_MAN_PREFIX)/man1/makerom.1"

#
#   start
#
DEPS_66 += compile
DEPS_66 += stop

start: $(DEPS_66)
	./$(BUILD)/bin/appman install enable start

#
#   install
#
DEPS_67 += installPrep
DEPS_67 += compile
DEPS_67 += stop
DEPS_67 += installBinary
DEPS_67 += start

install: $(DEPS_67)

#
#   run
#
DEPS_68 += compile

run: $(DEPS_68)
	( \
	cd src/server; \
	../../$(BUILD)/bin/appweb --log stdout:2 ; \
	)

#
#   uninstall
#
DEPS_69 += stop

uninstall: $(DEPS_69)
	( \
	cd installs; \
	rm -f "$(ME_ETC_PREFIX)/appweb.conf" ; \
	rm -f "$(ME_ETC_PREFIX)/esp.conf" ; \
	rm -f "$(ME_ETC_PREFIX)/mine.types" ; \
	rm -f "$(ME_ETC_PREFIX)/install.conf" ; \
	rm -fr "$(ME_INC_PREFIX)/appweb" ; \
	)

#
#   uninstallBinary
#

uninstallBinary: $(DEPS_70)
	rm -fr "$(ME_WEB_PREFIX)" ; \
	rm -fr "$(ME_SPOOL_PREFIX)" ; \
	rm -fr "$(ME_CACHE_PREFIX)" ; \
	rm -fr "$(ME_LOG_PREFIX)" ; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rmdir -p "$(ME_ETC_PREFIX)" 2>/dev/null ; true ; \
	rmdir -p "$(ME_WEB_PREFIX)" 2>/dev/null ; true ; \
	rmdir -p "$(ME_LOG_PREFIX)" 2>/dev/null ; true ; \
	rmdir -p "$(ME_SPOOL_PREFIX)" 2>/dev/null ; true ; \
	rmdir -p "$(ME_CACHE_PREFIX)" 2>/dev/null ; true ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true

#
#   version
#

version: $(DEPS_71)
	echo $(VERSION)


EXTRA_MAKEFILE := $(strip $(wildcard ./projects/extra.mk))
ifneq ($(EXTRA_MAKEFILE),)
include $(EXTRA_MAKEFILE)
endif
