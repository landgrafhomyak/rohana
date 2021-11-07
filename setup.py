from setuptools import setup, Extension

setup(
    name="rohana",
    version="0.0.0",
    packages=["rohana"],
    install_requires=[
        "packaging",
        # "pyhp @ https://github.com/LandgrafHomyak/pyhp/archive/refs/tags/v0.0.0b2.tar.gz"
    ],
    package_data={
        "rohana": ["py.typed", "*.pyi"],
    },
    ext_package="rohana",
    ext_modules=[
        Extension(
            name="file_tree",
            sources=["rohana/file_tree.c"]
        )
    ]
)
