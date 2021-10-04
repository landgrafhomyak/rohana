from setuptools import setup, Extension

setup(
    name="rohana",
    version="0.0.0",
    packages=["rohana", "rohana.commands"],
    ext_package="rohana",
    ext_modules=[
        Extension("_meta", ["rohana/_meta.c"]),
        Extension("commands.install_dependencies", ["rohana/commands/install_dependencies.c"])
    ]
)