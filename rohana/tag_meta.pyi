from typing import overload, Union, Literal, Any, NoReturn, Generic, TypeVar, Type, final

from .errors import BuildFailed
from .parse_xml_pyhp import Tag, PlainText


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
    def __call__(self: _PO, source: Tag, /) -> bound_tag[_PO]: ...

    @overload
    def __call__(self: _PO, source: PlainText, /) -> PlainText: ...

    def add(self, name: str, cls: tag_meta, /) -> NoReturn: ...


_PO = TypeVar("_PO", bound=pool)


@final
class tag_meta(type):
    def __new__(mcs, name, bases, dct) -> Type[tag]: ...

    def __getitem__(cls: _UTM, pool: _PO, /) -> Type[_UTM, bound_tag[_PO]]: ...

    @overload
    def __subclasscheck__(cls, subclass: Union[tag_meta, bound_tag_meta], /) -> bool: ...

    @overload
    def __subclasscheck__(cls, subclass: type, /) -> Literal[False]: ...

    @overload
    def __instancecheck__(cls, instance: bound_tag, /) -> bool: ...

    @overload
    def __instancecheck__(cls, instance: Any, /) -> Literal[False]: ...


_UTM = TypeVar("_UTM", bound=tag_meta)


class tag(metaclass=tag_meta):
    def __new__(cls, source: Tag, /) -> NoReturn: ...


@final
class bound_tag_meta(type, Generic[_UTM, _PO]):
    @property
    def __unbound__(cls, /) -> _UTM: ...

    @property
    def __pool__(cls, /) -> _PO: ...

    @overload
    def __subclasscheck__(cls, subclass: Union[tag_meta, bound_tag_meta], /) -> bool: ...

    @overload
    def __subclasscheck__(cls, subclass: type, /) -> False: ...


class bound_tag(Generic[_UTM, _PO], metaclass=bound_tag_meta[_UTM, _PO]):
    @property
    def __pool__(self, /) -> _PO: ...

    def __new__(cls, source: Tag, /) -> bound_tag[_PO]: ...
