from ..node_meta import node, InvalidNodeError


class a(node):
    __slots__ = "href", "nolocal", "__children"

    def __init__(self, source):
        super().__init__(source)

        if "href" not in source.params:
            raise Exception("a", "'href' not specified")
        self.href = source.params["href"]
        self.nolocal = "nolocal" in source.params
        if self.nolocal and source.params["nolocal"] is not None:
            raise InvalidNodeError("a", "'nolocal' mustn't has value")

        self.__children = [self.__pool__(child) for child in source.children]


