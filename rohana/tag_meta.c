#include <Python.h>
#include <structmember.h>

typedef struct Rohana_TM_Pool_Object
{
    PyObject_HEAD
    PyObject *dict;
} Rohana_TM_Pool_Object;

typedef struct Rohana_TM_BoundTagMeta_Object Rohana_TM_BoundTagMeta_Object;

typedef PyTypeObject Rohana_TM_TagMeta_Object;

typedef struct Rohana_TM_Tag_Object
{
    PyObject_HEAD
    Rohana_TM_Pool_Object *pool;
} Rohana_TM_Tag_Object;

typedef struct Rohana_TM_TagFactory_Object
{
    PyObject_HEAD
    Rohana_TM_TagMeta_Object *cls;
    Rohana_TM_Pool_Object *pool;
} Rohana_TM_TagFactory_Object;


static Rohana_TM_Pool_Object *Rohana_TM_Pool_New(PyTypeObject *cls);

static PyObject *Rohana_TM_Pool_Add(Rohana_TM_Pool_Object *self, PyObject *args);

static PyObject *Rohana_TM_Pool_Call(Rohana_TM_Pool_Object *self, PyObject *args, PyObject *kwargs);

static void Rohana_TM_Pool_Dealloc(Rohana_TM_Pool_Object *self);

PyMethodDef Rohana_TM_Pool_Methods[] = {
        {"add", (PyCFunction) Rohana_TM_Pool_Add, METH_VARARGS},
        {NULL}
};

static PyTypeObject Rohana_TM_Pool_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.pool",
        .tp_basicsize = sizeof(Rohana_TM_Pool_Object),
        .tp_new = (newfunc) Rohana_TM_Pool_New,
        .tp_dealloc = (destructor) Rohana_TM_Pool_Dealloc,
        .tp_methods = Rohana_TM_Pool_Methods,
        .tp_call = (ternaryfunc) Rohana_TM_Pool_Call
};

static Rohana_TM_TagMeta_Object *Rohana_TM_TagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs);

static Rohana_TM_TagFactory_Object *Rohana_TM_TagMeta_GetItem(Rohana_TM_TagMeta_Object *cls, Rohana_TM_Pool_Object *pool);

static PyObject *Rohana_TM_TagMeta_Call(Rohana_TM_TagMeta_Object *cls, PyObject *args, PyObject *kwargs);


static PyMappingMethods Rohana_TM_TagMeta_Mapping = {
        .mp_subscript = (binaryfunc) Rohana_TM_TagMeta_GetItem
};


static PyTypeObject Rohana_TM_TagMeta_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.tag_meta",
        .tp_basicsize = sizeof(Rohana_TM_TagMeta_Object),
        .tp_new = (newfunc) Rohana_TM_TagMeta_New,
        .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS,
        .tp_as_mapping = &Rohana_TM_TagMeta_Mapping,
        .tp_call = (ternaryfunc) Rohana_TM_TagMeta_Call
};

static void Rohana_TM_Tag_Dealloc(Rohana_TM_Tag_Object *self);

static PyMemberDef Rohana_TM_Tag_Members[] = {
        {"__pool__", T_OBJECT_EX, offsetof(Rohana_TM_Tag_Object, pool), READONLY},
        {NULL}
};

static Rohana_TM_TagMeta_Object Rohana_TM_Tag_Type = {
        PyVarObject_HEAD_INIT(&Rohana_TM_TagMeta_Type, 0)
        .tp_name = "rohana.tag_meta.tag",
        .tp_flags = Py_TPFLAGS_BASETYPE,
        .tp_members = Rohana_TM_Tag_Members,
        .tp_dealloc = (destructor) Rohana_TM_Tag_Dealloc
};

static Rohana_TM_Tag_Object *Rohana_TM_TagFactory_Call(Rohana_TM_TagFactory_Object *self, PyObject *args, PyObject *kwargs);

static void Rohana_TM_TagFactory_Dealloc(Rohana_TM_TagFactory_Object *self);

static PyTypeObject Rohana_TM_TagFactory_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.tag_meta.tag_factory",
        .tp_basicsize  = sizeof(Rohana_TM_TagFactory_Object),
        .tp_call = (ternaryfunc) Rohana_TM_TagFactory_Call,
        .tp_dealloc = (destructor) Rohana_TM_TagFactory_Dealloc
};

static Rohana_TM_Pool_Object *Rohana_TM_Pool_New(PyTypeObject *cls)
{
    Rohana_TM_Pool_Object *self;
    PyObject *dict;

    dict = PyDict_New();
    if (dict == NULL)
    {
        return NULL;
    }

    self = (Rohana_TM_Pool_Object *) cls->tp_alloc(cls, 0);
    if (self == NULL)
    {
        return NULL;
    }

    self->dict = dict;
    return self;

}

static PyObject *Rohana_TM_Pool_Add(Rohana_TM_Pool_Object *self, PyObject *args)
{
    PyObject *name;
    Rohana_TM_TagMeta_Object *cls;
    Rohana_TM_TagMeta_Object *scls;

    if (!PyArg_ParseTuple(args, "UO!", &name, &Rohana_TM_TagMeta_Type, &cls))
    {
        return NULL;
    }

    scls = (Rohana_TM_TagMeta_Object *) PyDict_GetItemWithError(self->dict, name);
    if (scls == NULL && PyErr_Occurred())
    {
        return NULL;
    }
    if (scls != NULL)
    {
        if (!PyObject_TypeCheck(cls, scls))
        {
            PyErr_Format(PyExc_TypeError, "overriding tag possible only by it's subclasses");
            return NULL;
        }
    }

    if (PyDict_SetItem(self->dict, name, (PyObject *) cls) != 0)
    {
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *Rohana_TM_Pool_Call(Rohana_TM_Pool_Object *self, PyObject *args, PyObject *kwargs)
{
    Py_RETURN_NONE;
    PyErr_SetObject;
}

static void Rohana_TM_Pool_Dealloc(Rohana_TM_Pool_Object *self)
{
    Py_DECREF(self->dict);
    Py_TYPE(self)->tp_free(self);
}

static Rohana_TM_TagMeta_Object *Rohana_TM_TagMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"name", "bases", "dct", NULL};
    PyObject *name;
    PyObject *bases;
    PyObject *dct;
    PyObject *key;
    PyObject *value;
    Py_ssize_t dct_iter_pos;


    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "UO!O!", kw_list, &name, &PyTuple_Type, &bases, &PyDict_Type, &dct))
    {
        return NULL;
    }

    dct_iter_pos = 0;
    while (PyDict_Next(dct, &dct_iter_pos, &key, &value))
    {
        if (!PyUnicode_Check(key))
        {
            PyErr_Format(PyExc_TypeError, "attributes' names must be str");
            return NULL;
        }
        switch (PyUnicode_CompareWithASCIIString(key, "__new__"))
        {
            case -1:
                if (PyErr_Occurred())
                {
                    return NULL;
                }
            case 1:
                break;
            case 0:
                PyErr_WarnFormat(PyExc_Warning, -1, "overloading __new__ method has no effect");
                continue;
        }
        switch (PyUnicode_CompareWithASCIIString(key, "__pool__"))
        {
            case -1:
                if (PyErr_Occurred())
                {
                    return NULL;
                }
            case 1:
                break;
            case 0:
                PyErr_Format(PyExc_TypeError, "overloading __pool__ not allowed");
                return NULL;
        }
    }

    return (Rohana_TM_TagMeta_Object *) PyType_Type.tp_new(mcs, args, kwargs);
}

static Rohana_TM_TagFactory_Object *Rohana_TM_TagMeta_GetItem(Rohana_TM_TagMeta_Object *cls, Rohana_TM_Pool_Object *pool)
{
    Rohana_TM_TagFactory_Object *bound;


    if (cls == &Rohana_TM_Tag_Type)
    {
        PyErr_Format(PyExc_TypeError, "%R is base type for subclassing, don't use if for binding with pool", cls);
        return NULL;
    }

    if (!PyObject_TypeCheck(pool, &Rohana_TM_Pool_Type))
    {
        PyErr_Format(PyExc_TypeError, "can bind tag only to pool objects");
        return NULL;
    }

    bound = (Rohana_TM_TagFactory_Object *) Rohana_TM_TagFactory_Type.tp_alloc(&Rohana_TM_TagFactory_Type, 0);
    if (bound == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    Py_INCREF(cls);
    bound->cls = cls;
    Py_INCREF(pool);
    bound->pool = pool;

    return bound;
}

static PyObject *Rohana_TM_TagMeta_Call(Rohana_TM_TagMeta_Object *cls, PyObject *args, PyObject *kwargs)
{
    PyErr_Format(
            PyExc_TypeError,
            "creating instances allowed only with bounding to tags pool: %s[<%s object>](...)",
            cls->tp_name,
            Rohana_TM_Pool_Type.tp_name
    );
    return NULL;
}

static void Rohana_TM_Tag_Dealloc(Rohana_TM_Tag_Object *self)
{
    Py_DECREF(self->pool);
    Py_TYPE(self)->tp_free(self);
}

static Rohana_TM_Tag_Object *Rohana_TM_TagFactory_Call(Rohana_TM_TagFactory_Object *self, PyObject *args, PyObject *kwargs)
{
    Rohana_TM_Tag_Object *tag;

    tag = (Rohana_TM_Tag_Object *) PyBaseObject_Type.tp_new(self->cls, args, kwargs);
    if (tag == NULL)
    {
        return NULL;
    }
    Py_INCREF(self->pool);
    tag->pool = self->pool;
    if (Py_TYPE(tag)->tp_init((PyObject *) tag, args, kwargs))
    {
        Py_DECREF(tag);
        return NULL;
    }
    return NULL;
}

static void Rohana_TM_TagFactory_Dealloc(Rohana_TM_TagFactory_Object *self)
{
    Py_DECREF(self->pool);
    Py_DECREF(self->cls);
    Py_TYPE(self)->tp_free(self);
}

static PyModuleDef module_def = {
        .m_name = "rohana.tag_meta",
        .m_methods = NULL
};

PyMODINIT_FUNC PyInit_tag_meta(void)
{
    PyObject *module;

    if (PyType_Ready(&Rohana_TM_Pool_Type))
    {
        return NULL;
    }

    Rohana_TM_TagMeta_Type.tp_base = &PyType_Type;
    Rohana_TM_TagMeta_Type.tp_init = PyType_Type.tp_init;
    Rohana_TM_TagMeta_Type.tp_dealloc = PyType_Type.tp_dealloc;
    if (PyType_Ready(&Rohana_TM_TagMeta_Type))
    {
        return NULL;
    }

    if (PyType_Ready((PyTypeObject *) &Rohana_TM_Tag_Type))
    {
        return NULL;
    }

    if (PyType_Ready((PyTypeObject *) &Rohana_TM_TagFactory_Type))
    {
        return NULL;
    }

    module = PyModule_Create(&module_def);
    if (module == NULL)
    {
        return NULL;
    }

    if (PyModule_AddObject(module, "pool", (PyObject *) &Rohana_TM_Pool_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "tag_meta", (PyObject *) &Rohana_TM_TagMeta_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "tag", (PyObject *) &Rohana_TM_Tag_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "tag_factory", (PyObject *) &Rohana_TM_TagFactory_Type))
    {
        Py_DECREF(module);
        return NULL;
    }


    if (PyModule_AddObject(module, "InvalidTagError", (PyObject *) Py_None))
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
