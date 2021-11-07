import os
from typing import NoReturn


class BuildFailed(Exception):
    pass


def build(
        location: str = os.getcwd(),
        *,
        output_dir: str = "docs"
) -> NoReturn: ...
