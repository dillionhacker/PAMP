dnl
dnl $Id: config.m4,v 1.6 2005/05/29 23:16:45 sniper Exp $
dnl

PHP_ARG_ENABLE(sysvshm,whether to enable System V shared memory support,
[  --enable-sysvshm        Enable the System V shared memory support])

if test "$PHP_SYSVSHM" != "no"; then
  AC_DEFINE(HAVE_SYSVSHM, 1, [ ])
  PHP_NEW_EXTENSION(sysvshm, sysvshm.c, $ext_shared)
fi
