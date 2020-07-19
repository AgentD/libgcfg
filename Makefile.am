ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE -I$(top_srcdir)/include
AM_CFLAGS = $(WARN_CFLAGS)

libgcfg_a_SOURCES = lib/dec_num.c lib/parse_number.c lib/util.h
libgcfg_a_SOURCES += lib/number_to_double.c lib/parse_boolean.c
libgcfg_a_SOURCES += lib/parse_string.c lib/parse_file.c
libgcfg_a_SOURCES += lib/parse_enum.c lib/xdigit.c lib/file.c include/gcfg.h
libgcfg_a_SOURCES += lib/parse_size.c lib/is_valid_utf8.c
libgcfg_a_CPPFLAGS = $(AM_CPPFLAGS)
libgcfg_a_CFLAGS = $(AM_CFLAGS)

include_HEADERS = include/gcfg.h
pkgconfig_DATA = libgcfg0.pc
lib_LIBRARIES = libgcfg.a
check_PROGRAMS =
EXTRA_DIST = LICENSE README
TESTS =

dec_num_SOURCES = lib/dec_num.c lib/dummy_file.c lib/util.h include/gcfg.h
dec_num_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
dec_num_CFLAGS = $(AM_CFLAGS)
dec_num_LDADD = libgcfg.a

xdigit_SOURCES = lib/xdigit.c lib/util.h include/gcfg.h
xdigit_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
xdigit_CFLAGS = $(AM_CFLAGS)
xdigit_LDADD = libgcfg.a

num_to_double_SOURCES = lib/number_to_double.c lib/util.h include/gcfg.h
num_to_double_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
num_to_double_CFLAGS = $(AM_CFLAGS)
num_to_double_LDADD = libgcfg.a -lm

parse_number_SOURCES = lib/parse_number.c lib/dummy_file.c lib/util.h
parse_number_SOURCES += include/gcfg.h
parse_number_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_number_CFLAGS = $(AM_CFLAGS)
parse_number_LDADD = libgcfg.a -lm

parse_string_SOURCES = lib/parse_string.c lib/dummy_file.c lib/util.h
parse_string_SOURCES += include/gcfg.h
parse_string_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_string_CFLAGS = $(AM_CFLAGS)
parse_string_LDADD = libgcfg.a

parse_boolean_SOURCES = lib/parse_boolean.c lib/dummy_file.c
parse_boolean_SOURCES += lib/util.h include/gcfg.h
parse_boolean_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_boolean_CFLAGS = $(AM_CFLAGS)
parse_boolean_LDADD = libgcfg.a

parse_size_SOURCES = lib/parse_size.c lib/dummy_file.c lib/util.h include/gcfg.h
parse_size_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_size_CFLAGS = $(AM_CFLAGS)
parse_size_LDADD = libgcfg.a

is_valid_utf8_SOURCES = lib/is_valid_utf8.c lib/util.h include/gcfg.h
is_valid_utf8_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
is_valid_utf8_CFLAGS = $(AM_CFLAGS)
is_valid_utf8_LDADD = libgcfg.a

check_PROGRAMS += dec_num xdigit num_to_double parse_number parse_string
check_PROGRAMS += parse_boolean parse_size is_valid_utf8

TESTS += dec_num xdigit num_to_double parse_number parse_string
TESTS += parse_boolean parse_size is_valid_utf8

if DISABLE_ALLOC
libgcfg_a_CPPFLAGS += -DGCFG_DISABLE_ALLOC
endif

if DISABLE_NETWORK
libgcfg_a_CPPFLAGS += -DGCFG_DISABLE_NETWORK
else
libgcfg_a_SOURCES += lib/ipv4address.c lib/parse_ipv4.c lib/parse_ipv6.c
libgcfg_a_SOURCES += lib/parse_bandwidth.c lib/parse_mac_addr.c

ipv4address_SOURCES = lib/ipv4address.c lib/dummy_file.c
ipv4address_SOURCES += lib/util.h include/gcfg.h
ipv4address_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
ipv4address_CFLAGS = $(AM_CFLAGS)
ipv4address_LDADD = libgcfg.a

parse_ipv4_SOURCES = lib/parse_ipv4.c lib/dummy_file.c lib/util.h include/gcfg.h
parse_ipv4_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_ipv4_CFLAGS = $(AM_CFLAGS)
parse_ipv4_LDADD = libgcfg.a

parse_ipv6_SOURCES = lib/parse_ipv6.c lib/dummy_file.c lib/util.h include/gcfg.h
parse_ipv6_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_ipv6_CFLAGS = $(AM_CFLAGS)
parse_ipv6_LDADD = libgcfg.a

parse_bandwidth_SOURCES = lib/parse_bandwidth.c lib/dummy_file.c lib/util.h
parse_bandwidth_SOURCES += include/gcfg.h
parse_bandwidth_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_bandwidth_CFLAGS = $(AM_CFLAGS)
parse_bandwidth_LDADD = libgcfg.a

parse_mac_addr_SOURCES = lib/parse_mac_addr.c lib/dummy_file.c lib/util.h
parse_mac_addr_SOURCES += include/gcfg.h
parse_mac_addr_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_mac_addr_CFLAGS = $(AM_CFLAGS)
parse_mac_addr_LDADD = libgcfg.a

sample_network_SOURCES = samples/network.c
sample_network_CPPFLAGS = $(AM_CPPFLAGS)
sample_network_CPPFLAGS += -DCFGPATH=$(top_srcdir)/samples/network.cfg
sample_network_LDADD = libgcfg.a

check_PROGRAMS += parse_bandwidth parse_mac_addr parse_ipv4 parse_ipv6
check_PROGRAMS += ipv4address sample_network
TESTS += ipv4address parse_ipv4 parse_ipv6 parse_bandwidth parse_mac_addr
TESTS += sample_network
endif

if DISABLE_VECTOR
libgcfg_a_CPPFLAGS += -DGCFG_DISABLE_VECTOR
else
libgcfg_a_SOURCES += lib/parse_vector.c

parse_vector_SOURCES = lib/parse_vector.c lib/dummy_file.c lib/util.h
parse_vector_SOURCES += include/gcfg.h
parse_vector_CPPFLAGS = $(AM_CPPFLAGS) -DBUILD_TEST
parse_vector_CFLAGS = $(AM_CFLAGS)
parse_vector_LDADD = libgcfg.a

check_PROGRAMS += parse_vector
TESTS += parse_vector
endif

sample_service_SOURCES = samples/service.c
sample_service_CPPFLAGS = $(AM_CPPFLAGS)
sample_service_CPPFLAGS += -DCFGPATH=$(top_srcdir)/samples/service.cfg
sample_service_LDADD = libgcfg.a
check_PROGRAMS += sample_service
TESTS += sample_service

EXTRA_DIST += samples/network.cfg samples/service.cfg
