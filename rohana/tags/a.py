from ..tag_meta import tag, InvalidTagError


class a(tag):
    __slots__ = "href", "nolocal", "__children"

    def __new__(cls, source):
        self = super().__new__(cls, source)

        if "href" not in source.params:
            raise InvalidTagError("a", "'href' not specified")
        self.href = source.params["href"]
        self.nolocal = "nolocal" in source.params
        if self.nolocal and source.params["nolocal"] is not None:
            raise InvalidTagError("a", "'nolocal' mustn't has value")

        self.__children = [self.__pool__(child) for child in source.children]

        return self

