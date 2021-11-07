import re
from os.path import split as splitpath

_default_names = frozenset(("index",))
_default_exts = frozenset((".html",))


class IndexMeta:
    __slots__ = "__names", "__exts", "__pattern"

    default_names = _default_names
    default_exts = _default_exts

    def __new__(cls, *, names=default_names, exts=default_exts):
        self = super().__new__(cls)
        if type(names) is str:
            self.__names = frozenset((names,))
        elif isinstance(names, str):
            self.__names = frozenset((str(names),))
        elif isinstance(names, (bytes, bytearray)):
            raise TypeError("index names must be str, not bytes")
        else:
            self.__names = frozenset(names)
            if not all(map(lambda _v: isinstance(_v, str), self.__names)):
                raise TypeError("index names must be str")
            self.__names = frozenset(map(str, self.__names))

        if type(exts) is str:
            self.__exts = frozenset((exts,))
        elif isinstance(exts, str):
            self.__exts = frozenset((str(exts),))
        elif isinstance(exts, (bytes, bytearray)):
            raise TypeError("index exts must be str, not bytes")
        else:
            self.__exts = frozenset(exts)
            if not all(map(lambda _v: isinstance(_v, str), self.__exts)):
                raise TypeError("index exts must be str")
            self.__exts = frozenset(map(str, self.__exts))

        # todo not secure
        self.__pattern = re.compile(r"(?:^|(?<=/))(" + "|".join(self.__names) + ")(" + "|".join(self.__exts) + ")$")

        return self

    def __call__(self, path):
        return self.__pattern.search(path) is not None


class Environment:
    __slots__ = "__ft", "__url", "__index_meta"

    @property
    def url(self):
        return self.__url

    def __new__(cls, ft, url, index_meta):
        self = super().__new__(cls)
        self.__ft = ft
        self.__url = re.fullmatch(r"([^\n]*?)/*", url)
        if self.__url is None:
            raise ValueError("url has new-line symbols")
        else:
            self.__url = self.__url.group(1)
        self.__index_meta = index_meta
        return self

    def path2url(self, path, /):
        if path.endswith(".rohana") or path.endswith(".pyhp"):
            path = path[:path.rfind(".")] + ".html"

        if self.__index_meta(path):
            return self.url + splitpath(path)[0] + "/"
        else:
            return self.url + path

    def __getitem__(self, path):
        return self.__ft[path]
