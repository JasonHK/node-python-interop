import sysconfig

flags = []

includes = [
    sysconfig.get_path("include"),
    sysconfig.get_path("platinclude"),
]

for include in includes:
    if include:
        flags.append("-I" + include)

print(" ".join(flags))
