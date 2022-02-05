import platform

from setuptools import Extension, setup


def get_ext_modules() -> list:
    """
    获取三方模块

    Linux需要编译封装接口
    Windows直接使用预编译的pyd即可
    Mac由于缺乏二进制库支持无法使用
    """
    if platform.system() == "Linux":
        extra_compile_flags = [
            "-std=c++17",
            "-O3",
            "-Wno-delete-incomplete",
            "-Wno-sign-compare",
        ]
        extra_link_args = ["-lstdc++"]
        runtime_library_dirs = ["$ORIGIN"]
    else:
        extra_compile_flags = ["-O2"]
        extra_link_args = []
        runtime_library_dirs = []

    vnminimd = Extension(
        "vnpy_mini.api.vnminimd",
        [
            "vnpy_mini/api/vnmini/vnminimd/vnminimd.cpp",
        ],
        include_dirs=["vnpy_mini/api/include",
                      "vnpy_mini/api/vnmini"],
        define_macros=[],
        undef_macros=[],
        library_dirs=["vnpy_mini/api/libs", "vnpy_mini/api"],
        libraries=["thostmduserapi", "thosttraderapi"],
        extra_compile_args=extra_compile_flags,
        extra_link_args=extra_link_args,
        runtime_library_dirs=runtime_library_dirs,
        depends=[],
        language="cpp",
    )

    vnminitd = Extension(
        "vnpy_mini.api.vnminitd",
        [
            "vnpy_mini/api/vnmini/vnminitd/vnminitd.cpp",
        ],
        include_dirs=["vnpy_mini/api/include",
                      "vnpy_mini/api/vnmini"],
        define_macros=[],
        undef_macros=[],
        library_dirs=["vnpy_mini/api/libs", "vnpy_mini/api"],
        libraries=["thostmduserapi", "thosttraderapi"],
        extra_compile_args=extra_compile_flags,
        extra_link_args=extra_link_args,
        runtime_library_dirs=runtime_library_dirs,
        depends=[],
        language="cpp",
    )

    return [vnminitd, vnminimd]


setup(
    ext_modules=get_ext_modules(),
)
