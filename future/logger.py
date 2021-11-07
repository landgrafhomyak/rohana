import sys
from abc import ABC, abstractmethod

import colorama


class RohanaLogger(ABC):
    @abstractmethod
    def command_start(self, name): ...

    @abstractmethod
    def command_end(self, name): ...

    @abstractmethod
    def debug(self, *messages, sep, end): ...

    @abstractmethod
    def info(self, *messages, sep, end): ...

    @abstractmethod
    def warning(self, *messages, sep, end): ...

    @abstractmethod
    def error(self, *messages, sep, end): ...

    @abstractmethod
    def critical(self, *messages, sep, end): ...

    @abstractmethod
    def flush(self): ...

    @abstractmethod
    def add_output(self, file): ...


class DefaultRohanaLogger(RohanaLogger):
    class StdoutProxy:
        pass

    class StderrProxy:
        pass

    __slots__ = "__command_layer", "__outputs", "__stdout", "__stderr", "__orig_stdout", "__orig_stderr"

    def __init__(self):
        self.__command_layer = False
        self.__outputs = []
        self.__orig_stdout = None
        self.__orig_stderr = None
        colorama.init()
        try:
            self.__orig_stdout = sys.stdout
            self.__outputs.append(self.__orig_stdout)
            self.__orig_stderr = sys.stderr
        except AttributeError:
            pass
        sys.stdout = self.__stdout
        sys.stderr = self.__stderr

    def finalize(self):
        self.flush()
        sys.stdout = self.__orig_stdout
        sys.stderr = self.__orig_stderr
        colorama.deinit()

