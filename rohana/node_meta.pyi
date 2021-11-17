from typing import overload, Union, Literal, Any, NoReturn, Generic, TypeVar, Type, final, Callable

from .errors import BuildFailed

__META_CLASS = False
__CUSTOM_KEY_ERROR = False


if __CUSTOM_KEY_ERROR:
    @final
    class UnexpectedNodeError(BuildFailed):
        name: str
else:
    UnexpectedNodeError = KeyError

@final
class InvalidNodeError(BuildFailed):
    name: str
    msg: str

    def __new__(self, name: str, msg: str) -> InvalidNodeError: ...


_RNT = TypeVar("_RNT")
_PTT = TypeVar("_PTT")


@final
class pool(Generic[_RNT, _PTT]):
    def __new__(
            cls,
            /,
            converter: Callable[[_RNT], str],
            *,
            raw_node_class: Type[_RNT] = object,
            plain_text_class: Type[_PTT] = str,
    ) -> pool[_RNT, _PTT]: ...

    @overload
    def __call__(self: _PO, source: _RNT, /) -> node[_PO]: ...

    @overload
    def __call__(self: _PO, source: _PTT, /) -> _PTT: ...

    def add(self, name: str, cls: node_meta if __META_CLASS else Type[node], /) -> NoReturn: ...


_PO = TypeVar("_PO", bound=pool)

if __META_CLASS:
    @final
    class node_meta(type):
        def __new__(mcs, name, bases, dct) -> node_meta: ...

        def __getitem__(cls, pool: _PO, /) -> node_factory[_PO]: ...


class node(Generic[_PO], metaclass=node_meta if __META_CLASS else type):
    def __new__(cls, source: Any, /) -> NoReturn: ...

    @property
    def __pool__(self, /) -> _PO: ...

    def __init__(self, source: Any, /) -> NoReturn: ...

    if not __META_CLASS:
        @classmethod
        def __class_getitem__(cls, pool: _PO, /) -> node_factory[_PO]:


@final
class node_factory(Generic[_PO]):
    def __call__(self, source) -> node[_PO]: ...


@overload
def is_node_bound(tag: node_factory, /) -> Literal[True]: ...


@overload
def is_node_bound(tag: Any, /) -> Literal[False]: ...
