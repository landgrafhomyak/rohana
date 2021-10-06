from typing import Tuple, Union

from . import command, CMD_WR_CLB, bound_command, command_dependency_decorator
from .meta import rohana_generator_meta


def get_dependencies(cmd_obj: Union[command[CMD_WR_CLB], bound_command[CMD_WR_CLB], command_dependency_decorator]) -> Tuple[str, ...]: ...


def unwrap_command(cmd_obj: Union[command[CMD_WR_CLB], bound_command[CMD_WR_CLB], command_dependency_decorator]) -> CMD_WR_CLB: ...


def get_commands_list(gen_cls: rohana_generator_meta) -> Tuple[str, ...]: ...