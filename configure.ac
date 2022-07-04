AC_PREREQ([2.60])
AC_INIT([libgcfg], [0.1], [goliath@infraroot.at], libgcfg)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_RANLIB
AC_PROG_INSTALL

PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

UL_WARN_ADD([-Wall])
UL_WARN_ADD([-Wextra])
UL_WARN_ADD([-Wunused])
UL_WARN_ADD([-pedantic])
UL_WARN_ADD([-Wpedantic])
UL_WARN_ADD([-Wmissing-prototypes])
UL_WARN_ADD([-Wmissing-declarations])
UL_WARN_ADD([-Wwrite-strings])
UL_WARN_ADD([-Wjump-misses-init])
UL_WARN_ADD([-Wuninitialized])
UL_WARN_ADD([-Winit-self])
UL_WARN_ADD([-Wlogical-op])
UL_WARN_ADD([-Wunused-but-set-parameter])
UL_WARN_ADD([-Wunused-but-set-variable])
UL_WARN_ADD([-Wunused-parameter])
UL_WARN_ADD([-Wunused-result])
UL_WARN_ADD([-Wunused-variable])
UL_WARN_ADD([-Wduplicated-cond])
UL_WARN_ADD([-Wduplicated-branches])
UL_WARN_ADD([-Wrestrict])
UL_WARN_ADD([-Wnull-dereference])
UL_WARN_ADD([-Walloc-zero])
UL_WARN_ADD([-Walloca])
UL_WARN_ADD([-Warith-conversion])
UL_WARN_ADD([-Wcast-align])
UL_WARN_ADD([-Wcast-align=strict])
UL_WARN_ADD([-Wcast-qual])
UL_WARN_ADD([-Wconversion])
UL_WARN_ADD([-Wdangling-else])
UL_WARN_ADD([-Wdate-time])
UL_WARN_ADD([-Wdouble-promotion])
UL_WARN_ADD([-Wfloat-conversion])
UL_WARN_ADD([-Wfloat-equal])
UL_WARN_ADD([-Wformat-nonliteral])
UL_WARN_ADD([-Wformat-security])
UL_WARN_ADD([-Wformat-signedness])
UL_WARN_ADD([-Wlong-long])
UL_WARN_ADD([-Wmissing-format-attribute])
UL_WARN_ADD([-Wmissing-noreturn])
UL_WARN_ADD([-Wpacked])
UL_WARN_ADD([-Wpacked-not-aligned])
UL_WARN_ADD([-Wredundant-decls])
UL_WARN_ADD([-Wsign-conversion])
UL_WARN_ADD([-Wswitch-default])
UL_WARN_ADD([-Wswitch-enum])
UL_WARN_ADD([-Wtrampolines])
UL_WARN_ADD([-Wundef])
UL_WARN_ADD([-Wunused-const-variable])
UL_WARN_ADD([-Wunused-const-variable])
UL_WARN_ADD([-Wvla])
UL_WARN_ADD([-fanalyzer])

AC_ARG_ENABLE([alloc],
	[AS_HELP_STRING([--disable-alloc],
			[Disable all runtime memory allocation.])],
	[], [enable_alloc="yes"])

AC_ARG_ENABLE([vector],
	[AS_HELP_STRING([--disable-vector],
			[Disable support fo vector types.])],
	[], [enable_vector="yes"])

AC_ARG_ENABLE([network],
	[AS_HELP_STRING([--disable-network],
			[Disable support fo network related data type.])],
	[], [enable_network="yes"])

AC_ARG_ENABLE([stdio],
	[AS_HELP_STRING([--disable-stdio],
			[Disable stdio file implementation.])],
	[], [enable_stdio="yes"])

AM_CONDITIONAL([DISABLE_ALLOC], [test "x$enable_alloc" != "xyes"])
AM_CONDITIONAL([DISABLE_VECTOR], [test "x$enable_vector" != "xyes"])
AM_CONDITIONAL([DISABLE_NETWORK], [test "x$enable_network" != "xyes"])
AM_CONDITIONAL([DISABLE_STDIO], [test "x$enable_stdio" != "xyes"])

AC_SUBST([WARN_CFLAGS])

AC_CONFIG_FILES([libgcfg0.pc])

AC_OUTPUT([Makefile])

AC_MSG_RESULT([
	${PACKAGE}  ${VERSION}

	prefix:            ${prefix}
	exec prefix:       ${exec_prefix}

	runstatedir:       ${runstatedir}
	bindir:            ${bindir}
	sbindir:           ${sbindir}
	libdir:            ${libdir}
	includedir:        ${includedir}

	use malloc:        ${enable_alloc}
	vector types:      ${enable_vector}
	network types:     ${enable_network}
	stdio files:       ${enable_stdio}

	compiler:          ${CC}
	cflags:            ${CFLAGS}
	ldflags:           ${LDFLAGS}

	warnings:

${WARN_CFLAGS}

	Type 'make' to compile.
])
