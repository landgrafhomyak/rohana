#include <Python.h>
#include <structmember.h>

typedef struct Rohana_MT_Pool_Object
{
    PyObject_HEAD
    PyObject *dict;
} Rohana_MT_Pool_Object;

typedef struct Rohana_MT_BoundTagMeta_Object Rohana_MT_BoundTagMeta_Object;

#define Rohana_MT_CUSTOM_POOL_CACHE (0)

#if Rohana_MT_CUSTOM_POOL_CACHE
typedef struct Rohana_MT_TagMeta_HeapCell
{
    Rohana_MT_Pool_Object *pool;
    Rohana_MT_BoundTagMeta_Object *type;
    /* https://habr.com/ru/post/150732/ */
    /*
    int balance_factor;
    Py_ssize_t lo, hi;
    */
} Rohana_MT_TagMeta_HeapCell;
#endif

typedef struct Rohana_MT_TagMeta_Object
{
    PyHeapTypeObject ht;
    PyObject *bases;
    PyObject *dct;
#if Rohana_MT_CUSTOM_POOL_CACHE
    Py_ssize_t used;
    Py_ssize_t allocated;
    Rohana_MT_TagMeta_HeapCell *pool_cache;
#else
    PyObject *pools_cache;
#endif
} Rohana_MT_TagMeta_Object;

struct Rohana_MT_BoundTagMeta_Object
{
    PyHeapTypeObject ht;
    Rohana_MT_TagMeta_Object *unbound;
    Rohana_MT_Pool_Object *pool;
};

typedef struct Rohana_MT_BoundTag_Object
{
    PyObject_HEAD
} Rohana_MT_BoundTag_Object;

static Rohana_MT_Pool_Object *Rohana_MT_Pool_New(PyTypeObject *cls);

static void Rohana_MT_Pool_Dealloc(Rohana_MT_Pool_Object *self);

static PyTypeObject Rohana_MT_Pool_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.pool",
        .tp_basicsize = sizeof(Rohana_MT_Pool_Object),
        .tp_new = (newfunc) Rohana_MT_Pool_New,
        .tp_dealloc = (destructor) Rohana_MT_Pool_Dealloc
};

static Rohana_MT_TagMeta_Object *Rohana_MT_TagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs);

#if Rohana_MT_CUSTOM_POOL_CACHE
static PyObject *Rohana_MT_TagMeta_Sizeof(PyTypeObject *mcs, Rohana_MT_TagMeta_Object *cls);
#endif

static Rohana_MT_BoundTagMeta_Object *Rohana_MT_TagMeta_GetItem(Rohana_MT_TagMeta_Object *cls, Rohana_MT_Pool_Object *pool);

static void Rohana_MT_TagMeta_Dealloc(Rohana_MT_TagMeta_Object *cls);

static PyMethodDef Rohana_MT_TagMeta_Methods[] = {
#if Rohana_MT_CUSTOM_POOL_CACHE
        {"__sizeof__", (PyCFunction) Rohana_MT_TagMeta_Sizeof, METH_O},
#endif
        {NULL}
};

static PyMappingMethods Rohana_MT_TagMeta_Mapping = {
        .mp_subscript =(binaryfunc) Rohana_MT_TagMeta_GetItem
};

static PyTypeObject Rohana_MT_TagMeta_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.tag_meta",
        .tp_basicsize = sizeof(Rohana_MT_TagMeta_Object),
        .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS,
        .tp_new = (newfunc) Rohana_MT_TagMeta_New,
        .tp_dealloc = (destructor) Rohana_MT_TagMeta_Dealloc,
        .tp_methods = Rohana_MT_TagMeta_Methods,
        .tp_as_mapping = &Rohana_MT_TagMeta_Mapping
};

static Rohana_MT_TagMeta_Object Rohana_MT_Tag_Type = {
        .ht = {
                {

                        PyVarObject_HEAD_INIT(&Rohana_MT_TagMeta_Type, 0)
                        .tp_name = "rohana.tag_meta.tag",
                        /* hasn't instances */
                        .tp_flags = Py_TPFLAGS_BASETYPE,
                }
        },
#if Rohana_MT_CUSTOM_POOL_CACHE
        .pools_cache = NULL,
        .used = 0,
        .allocated = 0,
#else
        .pools_cache = NULL,
#endif
        .dct = NULL,
        .bases = NULL,
};

static void *Rohana_MT_BoundTagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs);

static void Rohana_MT_BoundTagMeta_Dealloc(Rohana_MT_BoundTagMeta_Object *cls);

static PyMemberDef Rohana_MT_BoundTagMeta_Members[] = {
        {"__unbound__", T_OBJECT_EX, offsetof(Rohana_MT_BoundTagMeta_Object, unbound), READONLY},
        {"__pool__",    T_OBJECT_EX, offsetof(Rohana_MT_BoundTagMeta_Object, pool),    READONLY},
        {NULL}
};

static PyTypeObject Rohana_MT_BoundTagMeta_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.bound_tag_meta",
        .tp_basicsize = sizeof(Rohana_MT_BoundTagMeta_Object),
        .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS,
        .tp_members = Rohana_MT_BoundTagMeta_Members,
        .tp_new = (newfunc) Rohana_MT_BoundTagMeta_New,
        .tp_dealloc = (destructor) Rohana_MT_BoundTagMeta_Dealloc
};

static Rohana_MT_Pool_Object *Rohana_MT_BoundTag_GetPool(Rohana_MT_BoundTag_Object *self);

static PyGetSetDef Rohana_MT_BoundTag_GetSet[] = {
        {"__pool__", (getter) Rohana_MT_BoundTag_GetPool, NULL},
        {NULL}
};

static Rohana_MT_BoundTagMeta_Object Rohana_MT_BoundTag_Type = {
        .ht = {
                {
                        PyVarObject_HEAD_INIT(&Rohana_MT_BoundTagMeta_Type, 0)
                        .tp_name = "rohana.tag_meta.bound_tag",
                        .tp_basicsize = sizeof(Rohana_MT_BoundTag_Object),
                        .tp_flags = Py_TPFLAGS_BASETYPE,
                        .tp_getset = Rohana_MT_BoundTag_GetSet
                }
        },
        .pool = NULL,
        .unbound = &Rohana_MT_Tag_Type
};

static Rohana_MT_Pool_Object *Rohana_MT_Pool_New(PyTypeObject *cls)
{
    Rohana_MT_Pool_Object *self;
    PyObject *dict;

    dict = PyDict_New();
    if (dict == NULL)
    {
        return NULL;
    }

    self = (Rohana_MT_Pool_Object *) cls->tp_alloc(cls, 0);
    if (self == NULL)
    {
        return NULL;
    }

    self->dict = dict;
    return self;

}

static void Rohana_MT_Pool_Dealloc(Rohana_MT_Pool_Object *self)
{
    Py_DECREF(self->dict);
    Py_TYPE(self)->tp_free(self);
}

static Rohana_MT_TagMeta_Object *Rohana_MT_TagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"name", "bases", "dct", NULL};
    PyObject *name;
    PyObject *bases;
    PyObject *dct;
    PyObject *tag_bases;
    Py_ssize_t i;
    Py_ssize_t tag_bases_count;
    PyObject *qualname;
    PyObject *pools_cache;
    Rohana_MT_TagMeta_Object *cls;


    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "UO!O!", kw_list, &name, &PyTuple_Type, &bases, &PyDict_Type, &dct))
    {
        return NULL;
    }
    qualname = PyDict_GetItemString(dct, "__qualname__");
    if (qualname == NULL)
    {
        if (PyErr_Occurred())
        {
            return NULL;
        }
        qualname = name;
    }

    tag_bases_count = 0;
    for (i = PyTuple_GET_SIZE(bases) - 1; i >= 0; i--)
    {
        if (Py_TYPE(PyTuple_GET_ITEM(bases, i)) == &Rohana_MT_TagMeta_Type)
        {
            tag_bases_count++;
        }
    }


    tag_bases = PyTuple_New(tag_bases_count);
    if (tag_bases == NULL)
    {
        return NULL;
    }

    for (i = PyTuple_GET_SIZE(bases) - 1; i >= 0; i--)
    {
        if (Py_TYPE(PyTuple_GET_ITEM(bases, i)) == &Rohana_MT_TagMeta_Type)
        {
            Py_INCREF(PyTuple_GET_ITEM(bases, i));
            PyTuple_SET_ITEM(tag_bases, --tag_bases_count, PyTuple_GET_ITEM(bases, i));
        }
    }

    pools_cache = PyDict_New();
    if (pools_cache == NULL)
    {
        Py_DECREF(tag_bases);
        return NULL;
    }

    cls = (Rohana_MT_TagMeta_Object *) mcs->tp_alloc(mcs, 0);
    if (cls == NULL)
    {
        Py_DECREF(tag_bases);
        Py_DECREF(pools_cache);
        return NULL;
    }
    cls->bases = bases;
    Py_INCREF(bases);
#if Rohana_MT_CUSTOM_POOL_CACHE
    cls->allocated = 0;
    cls->used = 0;
    cls->pools_cache = NULL;
#else
    cls->pools_cache = pools_cache;
#endif
    cls->dct = dct;
    cls->ht.ht_name = name;
    Py_INCREF(name);
    cls->ht.ht_qualname = qualname;
    Py_INCREF(qualname);
    cls->ht.ht_type.tp_name = PyUnicode_AsUTF8(qualname);
    cls->ht.ht_type.tp_bases = tag_bases;
    cls->ht.ht_type.tp_flags = Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HEAPTYPE;

    if (cls->ht.ht_type.tp_name == NULL)
    {
        Py_DECREF(cls);
        return NULL;
    }
    if (PyType_Ready((PyTypeObject *) cls))
    {
        Py_DECREF(cls);
        return NULL;
    }

    return cls;

}

#if Rohana_MT_CUSTOM_POOL_CACHE
static PyObject *Rohana_MT_TagMeta_Sizeof(PyTypeObject *Py_UNUSED(mcs), Rohana_MT_TagMeta_Object *cls)
{
    return PyLong_FromSsize_t((Py_ssize_t) (sizeof(Rohana_MT_TagMeta_Object) + sizeof(Rohana_MT_TagMeta_HeapCell) * cls->allocated));
}
#endif

static Rohana_MT_BoundTagMeta_Object *Rohana_MT_TagMeta_GetItem(Rohana_MT_TagMeta_Object *cls, Rohana_MT_Pool_Object *pool)
{
    Rohana_MT_BoundTagMeta_Object *bound;
    PyObject *new_bases;
    Py_ssize_t i;
    PyObject *suffix;
    PyObject *qualname;
    PyObject *name;
    PyObject *new_dct;
    PyObject *args;

    if (cls == &Rohana_MT_Tag_Type)
    {
        PyErr_Format(PyExc_TypeError, "%R is base type for subclassing, don't use if for binding with pool", cls);
        return NULL;
    }

    if (!PyObject_TypeCheck(pool, &Rohana_MT_Pool_Type))
    {
        PyErr_Format(PyExc_TypeError, "can bind tag only to pool objects");
        return NULL;
    }

    bound = (Rohana_MT_BoundTagMeta_Object *) PyDict_GetItemWithError(cls->pools_cache, (PyObject *) pool);
    if (bound == NULL && PyErr_Occurred())
    {
        return NULL;
    }

    if (bound == NULL)
    {
        new_bases = PyTuple_New(PyTuple_GET_SIZE(cls->bases));
        if (new_bases == NULL)
        {
            return NULL;
        }

        for (i = PyTuple_GET_SIZE(cls->bases) - 1; i >= 0; i--)
        {
            if (PyTuple_GET_ITEM(cls->bases, i) == (PyObject *) &Rohana_MT_Tag_Type)
            {
                Py_INCREF(&Rohana_MT_BoundTag_Type);
                PyTuple_SET_ITEM(new_bases, i, (PyObject *) &Rohana_MT_BoundTag_Type);
            }
            else if (Py_TYPE(PyTuple_GET_ITEM(cls->bases, i)) == &Rohana_MT_TagMeta_Type)
            {
                bound = Rohana_MT_TagMeta_GetItem((Rohana_MT_TagMeta_Object *) PyTuple_GET_ITEM(cls->bases, i), pool);
                if (bound == NULL)
                {
                    Py_DECREF(new_bases);
                    return NULL;
                }
                PyTuple_SET_ITEM(new_bases, i, (PyObject *) bound);
            }
            else
            {
                Py_INCREF(PyTuple_GET_ITEM(cls->bases, i));
                PyTuple_SET_ITEM(new_bases, i, PyTuple_GET_ITEM(cls->bases, i));
            }
        }
        PyObject_Print(new_bases, stdout, Py_PRINT_RAW);


        suffix = PyUnicode_FromFormat("[%R]", pool);
        if (suffix == NULL)
        {
            Py_DECREF(new_bases);
            return NULL;
        }

        qualname = PyUnicode_Concat(cls->ht.ht_qualname, suffix);
        if (qualname == NULL)
        {
            Py_DECREF(new_bases);
            Py_DECREF(suffix);
            return NULL;
        }

        if (cls->ht.ht_name == cls->ht.ht_qualname)
        {
            Py_INCREF(qualname);
            name = qualname;
        }
        else
        {
            name = PyUnicode_Concat(cls->ht.ht_name, suffix);
            if (name == NULL)
            {
                Py_DECREF(new_bases);
                Py_DECREF(suffix);
                Py_DECREF(qualname);
                return NULL;
            }
        }
        Py_DECREF(suffix);

        new_dct = PyDict_Copy(cls->dct);
        if (new_dct == NULL)
        {
            Py_DECREF(new_bases);
            Py_DECREF(qualname);
            Py_DECREF(name);
        }

        if (PyDict_SetItemString(new_dct, "__qualname__", qualname) != 0)
        {
            Py_DECREF(new_bases);
            Py_DECREF(qualname);
            Py_DECREF(name);
            Py_DECREF(new_dct);
            return NULL;
        }
        Py_DECREF(qualname);

        if (PyDict_SetItemString(new_dct, "__name__", name) != 0)
        {
            Py_DECREF(new_bases);
            Py_DECREF(name);
            Py_DECREF(new_dct);
            return NULL;
        }

        args = PyTuple_Pack(3, name, new_bases, new_dct);
        Py_DECREF(new_bases);
        Py_DECREF(name);
        Py_DECREF(new_dct);
        if (args == NULL)
        {
            return NULL;
        }

        printf("{");
        bound = (Rohana_MT_BoundTagMeta_Object *) PyType_Type.tp_new(&Rohana_MT_BoundTagMeta_Type, args, NULL);
        printf("}");

        if (bound == NULL)
        {
            return NULL;
        }

//        Py_INCREF(pool);
//        bound->pool = pool;
//        Py_INCREF(cls);
//        bound->unbound = cls;

        if (PyType_Type.tp_init((PyObject *) bound, args, NULL) != 0)
        {
            Py_DECREF(args);
            return NULL;
        }

        Py_DECREF(args);
        /* bound->ht.ht_type.tp_flags &= ~Py_TPFLAGS_BASETYPE; */
        if (PyDict_SetItem(cls->pools_cache, (PyObject *) pool, (PyObject *) bound) != 0)
        {
            return NULL;
        }
    }
    else
    {
        Py_INCREF(bound);
    }

    printf("s!");

    return bound;
}

static void Rohana_MT_TagMeta_Dealloc(Rohana_MT_TagMeta_Object *cls)
{
    if (cls == &Rohana_MT_Tag_Type)
    {
        return;
    }
#if Rohana_MT_CUSTOM_POOL_CACHE
        Py_ssize_t i;

        if (cls->pools_tree != NULL)
        {
            for (i = cls->used - 1; i >= 0; i--)
            {
                Py_DECREF(cls->pools_tree[i].pool);
                Py_DECREF(cls->pools_tree[i].type);
            }
            PyMem_Free(cls->pools_tree);
        }
#else
    Py_DECREF(cls->pools_cache);
#endif
    Py_DECREF(cls->bases);
    Py_DECREF(cls->dct);
    PyType_Type.tp_dealloc((PyObject *) cls);
}


static void *Rohana_MT_BoundTagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"name", "bases", "dct", NULL};
    PyObject *name;
    PyObject *bases;
    PyObject *dct;


    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "UO!O!", kw_list, &name, &PyTuple_Type, &bases, &PyDict_Type, &dct))
    {
        return NULL;
    }

    PyErr_Format(PyExc_TypeError, "%R is not accessible as metaclass and it's instances for subclassing", mcs);

    return NULL;
}

static void Rohana_MT_BoundTagMeta_Dealloc(Rohana_MT_BoundTagMeta_Object *cls)
{
    if (cls == &Rohana_MT_BoundTag_Type)
    {
        return;
    }
    Py_DECREF(cls->pool);
    Py_DECREF(cls->unbound);
    PyType_Type.tp_dealloc((PyObject *) cls);

}

static Rohana_MT_Pool_Object *Rohana_MT_BoundTag_GetPool(Rohana_MT_BoundTag_Object *self)
{
    Py_INCREF(((Rohana_MT_BoundTagMeta_Object *) (Py_TYPE(self)))->pool);
    return ((Rohana_MT_BoundTagMeta_Object *) (Py_TYPE(self)))->pool;
}

static PyModuleDef module_def = {
        .m_name = "rohana.tag_meta",
        .m_methods = NULL
};

PyMODINIT_FUNC PyInit_tag_meta(void)
{
    PyObject *module;

    if (PyType_Ready(&Rohana_MT_Pool_Type))
    {
        return NULL;
    }

    Rohana_MT_TagMeta_Type.tp_base = &PyType_Type;
    Rohana_MT_TagMeta_Type.tp_dealloc = PyType_Type.tp_dealloc;
    if (PyType_Ready(&Rohana_MT_TagMeta_Type))
    {
        return NULL;
    }

    if (PyType_Ready((PyTypeObject *) &Rohana_MT_Tag_Type))
    {
        return NULL;
    }

    Rohana_MT_BoundTagMeta_Type.tp_base = &PyType_Type;
    if (PyType_Ready(&Rohana_MT_BoundTagMeta_Type))
    {
        return NULL;
    }

    if (PyType_Ready((PyTypeObject *) &Rohana_MT_BoundTag_Type))
    {
        return NULL;
    }

    module = PyModule_Create(&module_def);
    if (module == NULL)
    {
        return NULL;
    }

    if (PyModule_AddObject(module, "pool", (PyObject *) &Rohana_MT_Pool_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "tag_meta", (PyObject *) &Rohana_MT_TagMeta_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "tag", (PyObject *) &Rohana_MT_Tag_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "bound_tag_meta", (PyObject *) &Rohana_MT_BoundTagMeta_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "bound_tag", (PyObject *) &Rohana_MT_BoundTag_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
