from abc import ABC, abstractmethod
from typing import Protocol, Any, NoReturn


class file_or_proxy(Protocol):
    def write(self, s: str) -> Any: ...

    def flush(self) -> Any: ...


class RohanaLogger(ABC):
    @abstractmethod
    def command_start(self, name: str) -> NoReturn: ...

    @abstractmethod
    def command_end(self, name: str) -> NoReturn: ...

    @abstractmethod
    def debug(self, *messages, sep: str = " ", end: str = "\n") -> NoReturn: ...

    @abstractmethod
    def info(self, *messages, sep: str = " ", end: str = "\n") -> NoReturn: ...

    @abstractmethod
    def warning(self, *messages, sep: str = " ", end: str = "\n") -> NoReturn: ...

    @abstractmethod
    def error(self, *messages, sep: str = " ", end: str = "\n") -> NoReturn: ...

    @abstractmethod
    def critical(self, *messages, sep: str = " ", end: str = "\n") -> NoReturn: ...

    @abstractmethod
    def flush(self) -> NoReturn: ...

    @abstractmethod
    def add_output(self, file: file_or_proxy) -> NoReturn: ...
