from random import randint

import pytest

import rohana.file_tree


def dir_emulator(left):
    if left < 0:
        raise ValueError("invalid capacity")

    while left > 0:
        if left % 2 == 0:
            yield from dir_emulator(left // 2)
        else:
            yield object()
        left -= 1


def random_name():
    return chr(randint(ord("a"), ord("z")))


def test_builder_alloc_dealloc():
    rohana.file_tree.builder(dir_emulator(0))


def test_builder_push():
    b = rohana.file_tree.builder(dir_emulator(0))
    for _ in range(randint(20, 50)):
        b.push(random_name(), object())


def test_builder_enter():
    b = rohana.file_tree.builder(dir_emulator(0))
    for _ in range(randint(10, 20)):
        b.enter(random_name(), dir_emulator(0))


def test_builder_enter_leave_consistently():
    b = rohana.file_tree.builder(dir_emulator(0))
    x = randint(10, 20)
    for _ in range(x):
        b.enter(random_name(), dir_emulator(0))
    for _ in range(x):
        b.leave()


def test_builder_enter_leave_parallel():
    b = rohana.file_tree.builder(dir_emulator(0))
    for _ in range(randint(10, 20)):
        b.enter(random_name(), dir_emulator(0))
        b.leave()


@pytest.mark.timeout(100)
def test_builder_enter_leave_bool():
    b = rohana.file_tree.builder(dir_emulator(0))
    for _ in range(randint(10, 20)):
        b.enter(random_name(), dir_emulator(0))
    while b:
        b.leave()


@pytest.mark.timeout(100)
def test_builder_enter_leave_bool_count():
    b = rohana.file_tree.builder(dir_emulator(0))
    t = randint(10, 20)
    for _ in range(t):
        b.enter(random_name(), dir_emulator(0))
    r = 0
    while b:
        b.leave()
        r += 1
    assert t + 1 == r

