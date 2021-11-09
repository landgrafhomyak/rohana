from .parse_xml_pyhp import PlainText, Tag


class UnexpectedTagError(Exception):
    def __init__(self, name):
        super().__init__(f"<name>")
        self.name = name


class pool:
    __slots__ = "__dct"

    def __new__(cls):
        self = super().__new__(cls)
        self.__dct = dict()
        return self

    def __call__(self, source):
        if isinstance(source, PlainText):
            return source
        elif isinstance(source, Tag):
            if source.name not in self.__dct:
                raise UnexpectedTagError(source.name)
            return self.__dct[source.name][self](source)
        else:
            raise

    def add(self, name, klass):
        if type(name) is not str:
            raise TypeError("tag name must be str")
        if type(klass) is not tag_meta:
            raise TypeError(f"tag class must be subclass of {tag}")

        if name in self.__dct:
            if not issubclass(klass, self.__dct[name]):
                raise TypeError("overriding tag possible only by it's subclasses")
        self.__dct[name] = klass


class tag_meta(type):
    def __new__(mcs, name, bases, dct):
        cls = super().__new__(mcs, name, tuple(k for k in bases if type(k) is tag_meta) or (tag,), dict())
        cls.__pool_cache = dict()
        cls.__dct = dct
        cls.__orig_bases = bases
        return cls

    def __getitem__(cls, pl):
        if type(pl) is not pool:
            raise TypeError("can bind only to pool")
        if cls is tag:
            raise TypeError(f"{cls !r} is base type for subclassing, don't use if for binding with pool")

        bound = cls.__pool_cache.get(pool, None)
        if bound is None:
            bound = super(bound_tag_meta, None).__new__(
                bound_tag_meta,
                f"{cls.__name__}[{pl !r}]", tuple(k[pl] if type(k) is tag_meta else k for k in cls.__orig_bases),
                {**cls.__dct, "__name__": f"{cls.__dct.get('__name__', cls.__name__)}[{pl !r}]", "__qualname__": f"{cls.__dct.get('__qualname__', cls.__name__)}[{pl !r}]"}
            )
            bound.__unbound__ = cls
            bound.__pool__ = pl

            cls.__pool_cache[pl] = bound
        return bound

    def __subclasscheck__(self, subclass):
        return super().__subclasscheck__(subclass) or super(bound_tag_meta, bound_tag).__subclasscheck__(subclass)

    def __instancecheck__(self, instance):
        return super().__instancecheck__(instance) or super(bound_tag_meta, bound_tag).__instancecheck__(instance)


def tag_new(cls, sources):
    raise TypeError("can't create instance of unbound tag")


tag = super(tag_meta, tag_meta).__new__(
    tag_meta,
    "tag",
    (),
    {"__new__": tag_new, "__qualname__": f"{__name__}.tag"}
)


class bound_tag_meta(type):
    def __new__(mcs, name, bases, dct):
        raise TypeError(f"{mcs !r} is not accessible as metaclass and it's instances for subclassing")

    def __subclasscheck__(self, subclass):
        return super().__subclasscheck__(subclass) or super(tag_meta, tag).__subclasscheck__(subclass)


def bound_tag_new(cls, sources):
    if cls is bound_tag:
        raise TypeError(f"{cls !r} can't have instances")
    else:
        if not isinstance(sources, Tag):
            raise TypeError("can parse only tags")
        return super(bound_tag, bound_tag).__new__(cls)


bound_tag = super(bound_tag_meta, bound_tag_meta).__new__(
    bound_tag_meta,
    "bound_tag",
    (),
    {"__new__": bound_tag_new, "__qualname__": f"{__name__}.bound_tag"}
)
