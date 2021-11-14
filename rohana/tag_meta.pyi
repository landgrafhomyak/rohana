from typing import overload, Union, Literal, Any, NoReturn, Generic, TypeVar, Type, final

from .errors import BuildFailed
from .parse_xml_pyhp import Tag as xml_tag, PlainText


@final
class UnexpectedTagError(BuildFailed):
    name: str


@final
class InvalidTagError(BuildFailed):
    name: str
    msg: str

    def __new__(self, name: str, msg: str) -> InvalidTagError: ...


@final
class pool:
    def __new__(cls, /) -> pool: ...

    @overload
    def __call__(self: _PO, source: xml_tag, /) -> tag[_PO]: ...

    @overload
    def __call__(self: _PO, source: PlainText, /) -> PlainText: ...

    def add(self, name: str, cls: tag_meta, /) -> NoReturn: ...


_PO = TypeVar("_PO", bound=pool)


@final
class tag_meta(type):
    def __new__(mcs, name, bases, dct) -> tag_meta: ...

    def __getitem__(cls, pool: _PO, /) -> tag_factory[_PO]: ...


class tag(Generic[_PO], metaclass=tag_meta):
    def __new__(cls, source: xml_tag, /) -> NoReturn: ...

    @property
    def __pool__(self, /) -> _PO: ...


@final
class tag_factory(Generic[_PO]):
    def __call__(self, *args, **kwargs) -> tag[_PO]: ...


@overload
def istagbound(tag: tag_factory, /) -> Literal[True]: ...


@overload
def istagbound(tag: Any, /) -> Literal[False]: ...
