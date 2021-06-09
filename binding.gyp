{
    "targets": [
        {
            "target_name": "NodePython",
            "sources": [
                "src/main.cpp",
                "src/npi.cpp",
                "src/interop_helpers.cpp",
                "src/node_wrapper.c",
                "src/python_wrapper.cpp",
                "src/type_helpers.cpp",
            ],
            "include_dirs": [
                "<!@(echo $NVM_INC)",
                "<!@(node -p \"require('node-addon-api').include_dir\")",
                "extern/fmt/include",
            ],
            "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "cflags+": ["-g"],
            "cflags_cc+": ["-g"],
            "conditions": [
                [
                    "OS != \"win\"",
                    {
                        "variables": {
                            "PY_INCLUDE%": "<!(if [ -z \"$PY_INCLUDE\" ]; then echo $(python3 build_include.py); else echo $PY_INCLUDE; fi)",
                            "PY_LIBS%": "<!(if [ -z \"$PY_LIBS\" ]; then echo $(python3 build_ldflags.py); else echo $PY_LIBS; fi)",
                        },
                        "include_dirs+": ["<(PY_INCLUDE)"],
                        "libraries": ["<(PY_LIBS)"],
                    },
                ],
            ],
        },
    ],
}
