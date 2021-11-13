from typing import Tuple, List, Dict

from .errors import BuildFailed
from .parse_xml_pyhp import Tag, PlainText


class InvalidPage(BuildFailed):
    pass


class validator:
    @staticmethod
    def validate_toplevel(root_tag: Tag) -> Tuple[str, dict, List[Tag]]:
        if root_tag.name != "rohana":
            raise InvalidPage("unexpected top-level tag")
        if "type" not in root_tag.params:
            raise InvalidPage("page type not specified")

        clear_root_params = root_tag.params.copy()
        page_type = clear_root_params.pop("type")

        return page_type, clear_root_params, root_tag.children

    @staticmethod
    def parse_head_body(root_tag_children: List[Tag]) -> Tuple[List[Tag], Tag, Dict[str, Tag]]:
        try:
            body_tag = root_tag_children[-1]
            if body_tag.name != "body":
                raise InvalidPage
        except (IndexError, InvalidPage):
            raise InvalidPage("missed tag <body>")

        try:
            head_tag = root_tag_children[-2]
            if head_tag.name != "head":
                raise InvalidPage
        except (IndexError, InvalidPage):
            raise InvalidPage("missed tag <head>")

        if len(tuple(_ for _ in head_tag.children if _.name == "title")) > 1:
            raise InvalidPage("duplication of tag <title>")

        for param in head_tag.params:
            raise InvalidPage(f"unexpected param '{param}' in tag <head>")

        for tag in head_tag.children:
            if tag.name == "title":
                if len(tag.children) > 1 or len(tag.children) == 1 and type(tag.children[0]) is not PlainText:
                    raise InvalidPage("invalid page title")
            elif tag.name == "script":
                pass
            elif tag.name == "link":
                pass
            elif tag.name == "style":
                pass
            else:
                raise InvalidPage(f"unexpected tag <{tag.name}> in <head>")

        for param in body_tag.params:
            raise InvalidPage(f"unexpected param '{param}' in tag <body>")

        if len(body_tag.children) == 0:
            raise InvalidPage(f"page must have body at least for one language")

        for tag in body_tag.children:
            if tag.standalone:
                raise InvalidPage("all language tags must have body")

            for param in tag.params:
                raise InvalidPage(f"unexpected param '{param}' in language tag")

        return root_tag_children[:-2], head_tag, {tag.name: tag for tag in body_tag.children}


class base_page:
    pass


class ExternalSource(base_page):
    __slots__ = "destination", "local"

    def __init__(self, destination, local):
        self.destination = destination
        self.local = local


class Redirect(ExternalSource):
    pass


class Unchanged(ExternalSource):
    pass


class Copy(ExternalSource):
    pass


class Page(base_page):
    __slots__ = "title", "__sources", "head"

    def __init__(self, title, sources, head):
        self.title = title
        self.__sources = sources
        self.head = head

    def __getitem__(self, item: str):
        return self.__sources.get(item, None)

    def __iter__(self):
        return iter(self.__sources.items())


class Symbol(Page):
    __slots__ = "return_type", "args"

    def __init__(self, sources, head, return_type, args):
        super().__init__(None, sources, head)
        self.return_type = return_type
        self.args = args


class Arg:
    __slots__ = "type", "name", "default"

    def __init__(self, type, name, default):
        self.type = type
        self.name = name
        self.default = default


class Path:
    __slots__ = "__split", "__is_absolute"

    def __init__(self, split, is_absolute):
        self.__split = split
        self.__is_absolute = is_absolute

    @classmethod
    def raw(cls, s):
        return cls(*((s.split("/")[1:], True) if s.startswith("/") else (s.split("/"), False)))

    def __call__(self, root, current):
        if self.__is_absolute:
            p = root
        else:
            p = current
        for x in self.__split:
            p = p[x]
        return p


def determine(tree: Tag):
    page_type, root_params, toplvl_tags = validator.validate_toplevel(tree)

    if page_type == "redirect":
        if not tree.standalone:
            raise InvalidPage("redirect tag mustn't have body (use <rohana type='redirect' .../>)")
        if "to" not in root_params:
            raise InvalidPage("redirect destination not specified")

        destination = root_params.pop("to")
        if "nonlocal" in root_params:
            local = False
            if root_params.pop("nonlocal") is not None:
                raise InvalidPage("param 'nonlocal' mustn't have value")
        else:
            local = True

        for param in root_params:
            raise InvalidPage(f"unexpected param '{param}' in redirect tag")
        return Redirect(destination, local)

    toplvl_tags, head_tag, body = validator.parse_head_body(toplvl_tags)

    if page_type == "page":
        title = {tag.name: tag.children[0] for tag in head_tag.children}.get("title", None)

        if title is None:
            raise InvalidPage("missed tag <title>")

        return Page(str(title), body, head_tag)

    elif page_type == "symbol":
        if "title" in {tag.name for tag in head_tag.children}:
            raise InvalidPage("redundant tag <title> in symbol page")

        if len(toplvl_tags) < 1 or toplvl_tags[0].name != "return" or "type" not in toplvl_tags[0].params:
            raise InvalidPage("return type of symbol not specified")

        for param in set(toplvl_tags[0].params) - {"type"}:
            raise InvalidPage(f"unexpected param {param} in tag <return>")

        args = []
        for tag in toplvl_tags[1:]:
            if tag.name != "arg":
                raise InvalidPage(f"unexpected tag <{tag.name}> in symbol declaration")

            for param in set(tag.params) - {"type", "name", "default"}:
                raise InvalidPage(f"unexpected param '{param}' in tag <arg>")

            args.append(Arg(tag.params.get("type", None), tag.params.get("name", None), tag.params.get("default", None)))

        return Symbol(body, head_tag, toplvl_tags[0].params["type"], args)
