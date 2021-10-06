from abc import abstractmethod
from typing import overload, Tuple, Literal, NoReturn, Optional, Type, final, Any
from types import TracebackType


@final
class rohana_generator_meta(type):
    @overload
    def __call__(cls) -> NoReturn: ...

    @overload
    def __call__(cls, command: str, /, *args: str) -> NoReturn: ...

    def __setattr__(self, a: str, v: Any, /) -> NoReturn: ...

    def __delattr__(self, a: str, /) -> NoReturn: ...


class rohana_generator(metaclass=rohana_generator_meta):
    @final
    def __new__(cls) -> rohana_generator: ...

    @property
    @abstractmethod
    def logger(self): ...

    def __init__(self, *args: str) -> NoReturn: ...
