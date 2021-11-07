import re
from io import StringIO
from typing import List, Union, Dict

__all__ = "Tag", "PlainText", "parse", "ParseXmlError"


class PlainText(str):
    pass


class Tag:
    name: str
    standalone: bool
    children: List[Union[Tag, PlainText]]
    params: Dict[str, str]

    def dump(self, ident: int = 0) -> str: ...

    def __str__(self) -> str: ...

    def __repr__(self) -> str: ...


class ParseXmlError(Exception):
    pass


def parse(s: str, /) -> Tag: ...
