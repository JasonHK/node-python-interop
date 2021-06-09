import sys
from sysconfig import get_config_var

flags = []

VERSION = get_config_var("VERSION") or ""
abiflags = getattr(sys, "abiflags", "")

flags.append("-lpython" + VERSION + abiflags)

flags.extend(get_config_var("LIBS").split())
flags.extend(get_config_var("SYSLIBS").split())

if not get_config_var("Py_ENABLE_SHARED"):
    LIBPL = get_config_var("LIBPL")
    if not LIBPL:
        raise RuntimeError("LIBPL was not found.")
    flags.insert(0, "-L" + LIBPL)

if not get_config_var("PYTHONFRAMEWORK"):
    flags.extend(get_config_var("LINKFORSHARED").split())

print(" ".join(flags))
