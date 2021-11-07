from pathlib import Path
from typing import Union

from .file_tree import file_tree


class StaticFile:
    pass


class PyhpScript:
    source: str


def parse_tree(location: Path) -> file_tree[Union[StaticFile, PyhpScript, base_page]]: ...
