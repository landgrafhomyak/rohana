from typing import Optional, Literal, List

from .errors import BuildFailed
from .parse_xml_pyhp import Tag


class base_page:
    pass


class ExternalSource(base_page):
    destination:str
    local:bool

class Redirect(ExternalSource):
    pass


class Unchanged(ExternalSource):
    pass

class Copy(ExternalSource):
    pass

class Page(base_page):
    title: str
    head: Tag

    def __getitem__(self, item: str) -> Tag: ...


class Arg:
    type: Optional[str]
    name: Optional[str]
    default: Optional[str]


class Symbol(Page):
    title: Literal[None]
    return_type: str
    args: List[Arg]


class InvalidPage(BuildFailed):
    pass


def determine(tree: Tag) -> base_page: ...
