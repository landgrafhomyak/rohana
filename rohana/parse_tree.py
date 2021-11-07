from .errors import BuildFailed
from .parse_xml_pyhp import parse as parse_xml
from .process_xml import determine as determine_xml
from .file_tree import builder as ft_builder, compile as ft_compile


class StaticFile:
    __instance = None

    def __new__(cls):
        if cls.__instance is None:
            cls.__instance = super().__new__(cls)

        return cls.__instance


class PyhpScript:
    __slots__ = "source"

    def __init__(self, source):
        self.source = source


def parse_tree(location):
    ftb = ft_builder(location.iterdir())
    while ftb:
        for child in ftb:
            if child.is_dir():
                ftb.enter(child.name, child.iterdir())
            elif child.is_file():
                if child.suffix == ".rohana":
                    ftb.push(child.name, determine_xml(parse_xml(child.read_text("utf-8"))))
                elif child.suffix == ".pyhp":
                    ftb.push(child.name, PyhpScript(child.read_text("utf-8")))
                else:
                    ftb.push(child.name, StaticFile())
            else:
                raise BuildFailed(f"unknown type of file {str(child) !a}")
            break
        else:
            ftb.leave()

    return ft_compile(ftb)
