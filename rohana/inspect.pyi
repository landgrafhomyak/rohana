from typing import Tuple

from . import command, CMD_WR_CLB


def inspect(cmd_obj: command) -> Tuple[str, ...]: ...


def unwrap_command(cmd_obj: command[CMD_WR_CLB]) -> CMD_WR_CLB: ...
