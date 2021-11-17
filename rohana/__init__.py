import os
from pathlib import Path, PurePath
from shutil import rmtree, copytree
from packaging.version import Version, InvalidVersion

from rohana.process_xml import Page
from .environment import Environment, IndexMeta
from .errors import BuildFailed
from .parse_tree import parse_tree
from .parse_xml_pyhp import parse as parse_xml, Tag, PlainText
from .node_meta import pool

from .tags.a import a as tag_a


def build(
        location=os.getcwd(),
        *,
        output_dir="docs",
        root_url="/",
        version_class=Version
):
    try:
        location = Path(location)
        output_dir = location.joinpath(output_dir)
        if output_dir.exists() and not output_dir.is_dir():
            raise BuildFailed("Output path is not a dir")

        if output_dir.exists():
            rmtree(output_dir)

        static_dir = location.joinpath("static")
        if not static_dir.exists() or not static_dir.exists():
            raise BuildFailed("Can't get static files")

        copytree(static_dir, output_dir)

        source_dir = location.joinpath("sources")
        if not source_dir.exists() or not source_dir.is_dir():
            raise BuildFailed("сan't get sources")

        tree = parse_tree(source_dir)
        env = Environment(tree, root_url, IndexMeta())

        for dir in tree.dirs():
            index_exists = False
            for name in dir.paths():
                if env.path2url(str(name)).endswith("/"):
                    if index_exists:
                        raise BuildFailed(f"too many index files in {dir}")
                    index_exists = True


        tag_pool = pool(lambda _t: _t.name, raw_node_class=Tag, plain_text_class=PlainText)
        for n in "a",:
            print(globals()[f"tag_{n}"].mro())
            tag_pool.add(n, globals()[f"tag_{n}"])

        for i in tree.items():
            pass
            if isinstance(i.value, Page):
                for lang, root_tag in i.value:
                    root_tag.children = [tag_pool(child) for child in root_tag.children]
                    print(i.path, i.value, lang, root_tag)
        print("checked")

        # print()
        # versions = dict()
        # version_independent = set()
        # for subdir in source_dir.iterdir():
        #     try:
        #         versions[Version(subdir.name)] = subdir
        #     except InvalidVersion:
        #         version_independent.add(subdir)
        #
        # print("parsed versions:", *versions)

        return tree
    except BuildFailed:
        raise
    except Exception as exc:
        raise BuildFailed from exc


if __name__ == "__main__":
    build()
