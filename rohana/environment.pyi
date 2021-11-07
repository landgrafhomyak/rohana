from os.path import splitext, split as splitpath
from typing import FrozenSet, ClassVar, Union, Iterable, final

from .file_tree import file_tree

from .process_xml import base_page


@final
class IndexMeta:
    default_names: ClassVar[FrozenSet[str]] = {"index"}
    default_exts: ClassVar[FrozenSet[str]] = {".html"}

    def __new__(
            cls,
            *,
            names: Union[str, Iterable[str]] = default_names,
            exts: Union[str, Iterable[str]] = default_exts
    ) -> IndexMeta: ...

    def __call__(self, path: str, /) -> bool: ...


class Environment:
    @property
    def url(self, /) -> str: ...

    def path2url(self, path: str, /) -> str: ...

    def __getitem__(self, item: file_tree.path) -> Union[base_page]: ...
