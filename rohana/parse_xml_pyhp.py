import re
from io import StringIO
from typing import List

__all__ = "Tag", "PlainText", "parse", "ParseXmlError"


class Tag:
    __slots__ = "name", "children", "standalone", "params"

    def __init__(self, name, standalone):
        self.name = name
        self.standalone = standalone
        self.children = []
        self.params = dict()

    def dump(self, ident=0):
        s = StringIO()
        s.write(" " * ident + f"<{self.name}")
        for param in self.params:
            if self.params[param]:
                s.write(f" {param}={self.params[param]!a}")
            else:
                s.write(f" {param}")
        if self.standalone or not self.children:
            s.write(" />\n")
            s.seek(0)
            return s.read()
        s.write(">\n")

        for child in self.children:
            if isinstance(child, PlainText):
                s.write(" " * (ident + 1) + child + "\n")
            else:
                s.write(child.dump(ident + 1))
        s.write(" " * ident + f"</{self.name}>\n")
        s.seek(0)
        return s.read()

    def __str__(self):
        return f"<xml tag '{self.name}{'/' if self.standalone else ''}'>"

    def __repr__(self):
        return f"<xml tag object '{self.name}{'/' if self.standalone else ''}' at {hex(id(self)).zfill(8)}>"


class PlainText(str):
    pass


tag_pattern = re.compile(r"<([^<>]+)>")
tag_data_pattern = re.compile(r"(/)?\s*([\w\-]+)(?(1)\s*|(?:\s+([\s\S]*(?<!/))(/)?|))")
tag_param_pattern = re.compile(r"(?:(?<=\s)|^)([\w\-]+)(?:=['\"]?([\w\-]+|(?<=')[^']*(?=')|(?<=\")[^\"]*(?=\"))['\"]?|)(?=\s|$)")


class ParseXmlError(Exception):
    pass


class UnexpectedCloseTag(ParseXmlError):
    __slots__ = "open", "close"

    def __init__(self, open, close):
        self.open = open
        self.close = close

    def __str__(self):
        if self.open is None:
            return f"</{self.close}>"
        else:
            return f"<{self.open}></{self.close}>"


def parse(s):
    root = None
    tags: List[Tag] = []
    pos = 0
    while pos < len(s):
        m = tag_pattern.search(s, pos=pos)
        if m is None:
            if len(tags) == 0:
                raise ParseXmlError("plain text is not allowed at top level")
            tags[-1].children.append(PlainText(s[pos:]))
            break
        if m.start() > pos and not s[pos:m.start()].isspace():
            if len(tags) == 0:
                raise ParseXmlError("plain text is not allowed at top level")
            tags[-1].children.append(PlainText(s[pos:m.start()]))
        data = tag_data_pattern.fullmatch(m.group(1))
        if data is None:
            if m.group(1).startswith("/"):
                raise ParseXmlError(f"close tag have params: <{m.group(1)!a}>")
            else:
                raise ParseXmlError("invalid tag name")

        if data.group(1):
            if len(tags) == 0:
                raise UnexpectedCloseTag(None, data.group(2))
            else:
                if data.group(2) != tags[-1].name:
                    raise UnexpectedCloseTag(tags[-1].name, data.group(2))
                tags.pop()
                pos = m.end()
                continue
        new_tag = Tag(data.group(2), bool(data.group(4)))
        if len(tags) == 0:
            root = new_tag
        else:
            tags[-1].children.append(new_tag)
        if not new_tag.standalone:
            tags.append(new_tag)
        for param in tag_param_pattern.finditer(data.group(3) or ""):
            if param.group(1) in new_tag.params:
                raise ParseXmlError("param duplication")
            new_tag.params[param.group(1)] = param.group(2) or ""

        pos = m.end()

    return root
