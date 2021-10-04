from typing import Callable, Any, Tuple, TypeVar, Generic

CMD_WR_CLB = TypeVar("CMD_WR_CLB", bound=Callable[[Any], Any])

class command(Generic[CMD_WR_CLB]):
    def __new__(cls, callable: CMD_WR_CLB) -> command: ...

    @classmethod
    def depends(cls, *args: Tuple[str]) -> Callable[[CMD_WR_CLB], command]: ...

