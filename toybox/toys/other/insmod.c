/* insmod.c - Load a module into the Linux kernel.
 *
 * Copyright 2012 Elie De Brauwer <eliedebrauwer@gmail.com>

USE_INSMOD(NEWTOY(insmod, "<1", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))

config INSMOD
  bool "insmod"
  default y
  help
    usage: insmod MODULE [MODULE_OPTIONS]

    Load the module named MODULE passing options if given.
*/

#include "toys.h"

#include <sys/syscall.h>
#ifdef SYS_finit_module
#define finit_module(fd, opts, flags) syscall(SYS_finit_module, fd, opts, flags)
#else
#define finit_module(a, b, c) (errno = ENOSYS)
#endif
#define init_module(mod, len, opts) syscall(SYS_init_module, mod, len, opts)

#ifdef VENDOR_EDIT
//fangpan@Swdp.shanghai, 2016/01/20, add the insmod -f option
#define MODULE_INIT_IGNORE_MODVERSIONS  1
#define MODULE_INIT_IGNORE_VERMAGIC     2
#endif

void insmod_main(void)
{
  int fd = xopenro(*toys.optargs);
  int i, rc;

#ifdef VENDOR_EDIT
//fangpan@Swdp.shanghai, 2016/01/20, add the insmod -f option
  int flags = 0;

  if (toys.optargs[1] != NULL && strcmp(toys.optargs[1], "-f") == 0) {
    i = 2;
    flags = MODULE_INIT_IGNORE_MODVERSIONS | MODULE_INIT_IGNORE_VERMAGIC;
  }
  else {
    i = 1;
  }
#else
  i = 1;
#endif
  while (toys.optargs[i] &&
    strlen(toybuf) + strlen(toys.optargs[i]) + 2 < sizeof(toybuf))
  {
    strcat(toybuf, toys.optargs[i++]);
    strcat(toybuf, " ");
  }

  // finit_module was new in Linux 3.8, and doesn't work on stdin,
  // so we fall back to init_module if necessary.
#ifdef VENDOR_EDIT
//fangpan@Swdp.shanghai, 2016/01/20, add the insmod -f option
  rc = finit_module(fd, toybuf, flags);
#else
  rc = finit_module(fd, toybuf, 0);
#endif
  if (rc && (fd == 0 || errno == ENOSYS)) {
    off_t len = 0;
    char *path = !strcmp(*toys.optargs, "-") ? "/dev/stdin" : *toys.optargs;
    char *buf = readfileat(AT_FDCWD, path, NULL, &len);

    rc = init_module(buf, len, toybuf);
    if (CFG_TOYBOX_FREE) free(buf);
  }

  if (rc) perror_exit("failed to load %s", toys.optargs[0]);

  if (CFG_TOYBOX_FREE) close(fd);
}
