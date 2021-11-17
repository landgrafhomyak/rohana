#include <Python.h>
#include <structmember.h>

#define Rohana_NM_META_CLASS (0)
#define Rohana_NM_CustomKeyError (0)

#if Rohana_NM_CustomKeyError
typedef struct Rohana_NM_UnexpectedNodeError_Object
{
    PyBaseExceptionObject *base;
    PyObject *input;
    PyObject *key;
} Rohana_NM_UnexpectedNodeError_Object;
#endif
typedef struct Rohana_NM_InvalidNodeError_Object
{
    PyBaseExceptionObject *base;
    PyObject *name;
    PyObject *msg;
} Rohana_NM_InvalidNodeError_Object;


typedef struct Rohana_NM_Pool_Object
{
    PyObject_HEAD
    PyObject *dict;
    PyTypeObject *raw_node_cls;
    PyTypeObject *plain_text_cls;
    PyObject *converter;
} Rohana_NM_Pool_Object;

#if Rohana_NM_META_CLASS
typedef PyHeapTypeObject Rohana_NM_NodeMeta_Object;
#endif
typedef struct Rohana_NM_Node_Object
{
    PyObject_HEAD
    Rohana_NM_Pool_Object *pool;
    int is_inited;
} Rohana_NM_Node_Object;

typedef struct Rohana_NM_NodeFactory_Object
{
    PyObject_HEAD
#if Rohana_NM_META_CLASS
    Rohana_NM_NodeMeta_Object *cls;
#else
    PyTypeObject *cls;
#endif
    Rohana_NM_Pool_Object *pool;
} Rohana_NM_NodeFactory_Object;

#if Rohana_NM_CustomKeyError
static PyMemberDef Rohana_NM_UnexpectedNodeError_Members[] = {
        {"input", T_OBJECT_EX, offsetof(Rohana_NM_UnexpectedNodeError_Object, input)},
        {"key",   T_OBJECT_EX, offsetof(Rohana_NM_UnexpectedNodeError_Object, key)},
        {NULL}
};

static void Rohana_NM_UnexpectedNodeError_Dealloc(Rohana_NM_UnexpectedNodeError_Object *self);

static PyTypeObject Rohana_NM_UnexpectedNodeError_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.node_meta.UnexpectedNodeError",
        .tp_basicsize = sizeof(Rohana_NM_UnexpectedNodeError_Object),
        .tp_dealloc = (destructor) Rohana_NM_UnexpectedNodeError_Dealloc,
        .tp_members = Rohana_NM_UnexpectedNodeError_Members,
        .tp_flags = Py_TPFLAGS_BASE_EXC_SUBCLASS
};
#endif

static Rohana_NM_Pool_Object *Rohana_NM_Pool_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static PyObject *Rohana_NM_Pool_Add(Rohana_NM_Pool_Object *self, PyObject *args);

static PyObject *Rohana_NM_Pool_Call(Rohana_NM_Pool_Object *self, PyObject *args, PyObject *kwargs);

static void Rohana_NM_Pool_Dealloc(Rohana_NM_Pool_Object *self);

PyMethodDef Rohana_NM_Pool_Methods[] = {
        {"add", (PyCFunction) Rohana_NM_Pool_Add, METH_VARARGS},
        {NULL}
};

static PyTypeObject Rohana_NM_Pool_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.node_meta.pool",
        .tp_basicsize = sizeof(Rohana_NM_Pool_Object),
        .tp_new = (newfunc) Rohana_NM_Pool_New,
        .tp_dealloc = (destructor) Rohana_NM_Pool_Dealloc,
        .tp_methods = Rohana_NM_Pool_Methods,
        .tp_call = (ternaryfunc) Rohana_NM_Pool_Call,
};

#if Rohana_NM_META_CLASS
static Rohana_NM_NodeMeta_Object *Rohana_NM_NodeMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs);

static Rohana_NM_NodeFactory_Object *Rohana_NM_NodeMeta_GetItem(Rohana_NM_NodeMeta_Object *cls, Rohana_NM_Pool_Object *pool);

static void Rohana_NM_NodeMeta_Dealloc(Rohana_NM_NodeMeta_Object *cls);

static PyObject *Rohana_NM_NodeMeta_Call(Rohana_NM_NodeMeta_Object *cls, PyObject *args, PyObject *kwargs);

#else

static PyObject *Rohana_NM_Node_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static Rohana_NM_NodeFactory_Object *Rohana_NM_NodeMeta_GetItem(PyTypeObject *cls, Rohana_NM_Pool_Object *pool);

#endif
#if Rohana_NM_META_CLASS
static PyMappingMethods Rohana_NM_NodeMeta_Mapping = {
        .mp_subscript = (binaryfunc) Rohana_NM_NodeMeta_GetItem
};


static PyTypeObject Rohana_NM_NodeMeta_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.node_meta.node_meta",
        .tp_basicsize = sizeof(Rohana_NM_NodeMeta_Object),
        .tp_new = (newfunc) Rohana_NM_NodeMeta_New,
        .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS,
        .tp_as_mapping = &Rohana_NM_NodeMeta_Mapping,
        .tp_call = (ternaryfunc) Rohana_NM_NodeMeta_Call,
        .tp_dealloc = (destructor) Rohana_NM_NodeMeta_Dealloc
};
#endif

static int Rohana_NM_Node_Init(Rohana_NM_Node_Object *self, PyObject *args, PyObject *kwargs);

static void Rohana_NM_Node_Dealloc(Rohana_NM_Node_Object *self);

static PyMemberDef Rohana_NM_Node_Members[] = {
        {"__pool__", T_OBJECT_EX, offsetof(Rohana_NM_Node_Object, pool), READONLY},
        {NULL}
};

static PyMethodDef Rohana_NM_Node_Methods[] = {
        {"__class_getitem__", (PyCFunction) Rohana_NM_NodeMeta_GetItem, METH_O | METH_CLASS},
        {NULL}
};

static PyTypeObject Rohana_NM_Node_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.node_meta.node",
        .tp_basicsize = sizeof(Rohana_NM_Node_Object),
        .tp_flags = Py_TPFLAGS_BASETYPE,
        .tp_new = Rohana_NM_Node_New,
        .tp_init = (initproc) Rohana_NM_Node_Init,
        .tp_members = Rohana_NM_Node_Members,
        .tp_dealloc = (destructor) Rohana_NM_Node_Dealloc,
        .tp_methods = Rohana_NM_Node_Methods
};

static Rohana_NM_Node_Object *Rohana_NM_NodeFactory_Call(Rohana_NM_NodeFactory_Object *self, PyObject *args, PyObject *kwargs);

static void Rohana_NM_NodeFactory_Dealloc(Rohana_NM_NodeFactory_Object *self);

static PyTypeObject Rohana_NM_NodeFactory_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.node_meta.node_factory",
        .tp_basicsize  = sizeof(Rohana_NM_NodeFactory_Object),
        .tp_call = (ternaryfunc) Rohana_NM_NodeFactory_Call,
        .tp_dealloc = (destructor) Rohana_NM_NodeFactory_Dealloc
};

static int type_converter(PyObject *src, PyTypeObject **dst)
{
    if (!PyObject_TypeCheck(Py_TYPE(src), &PyType_Type))
    {
        PyErr_Format(PyExc_TypeError, "required class, got %R", src);
        return 0;
    }
    *dst = (PyTypeObject *) src;
    return 1;
}

#if Rohana_NM_CustomKeyError
static void Rohana_NM_UnexpectedNodeError_Dealloc(Rohana_NM_UnexpectedNodeError_Object *self)
{
    printf("asdadkjlasd\n");
    Py_DECREF(self->key);
    Rohana_NM_UnexpectedNodeError_Type.tp_base->tp_dealloc((PyObject *) self);
}
#endif

static Rohana_NM_Pool_Object *Rohana_NM_Pool_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"converter", "raw_node_class", "plain_text_class", NULL};
    PyTypeObject *rn_cls = &PyBaseObject_Type;
    PyTypeObject *pt_cls = &PyUnicode_Type;
    PyObject *converter;
    Rohana_NM_Pool_Object *self;
    PyObject *dict;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|$O&O&", kw_list, &converter, type_converter, &rn_cls, type_converter, &pt_cls))
    {
        return NULL;
    }

    if (!PyCallable_Check(converter))
    {
        PyErr_Format(PyExc_TypeError, "converter must be callable");
        return NULL;
    }
    switch (PyObject_IsSubclass((PyObject *) pt_cls, (PyObject *) rn_cls))
    {
        case -1:
            return NULL;
        case 1:
        dinherit:
            PyErr_Format(PyExc_TypeError, "plain text and raw node classes is the same or inherit each other");
            return NULL;
        case 0:
            break;
    }
    switch (PyObject_IsSubclass((PyObject *) rn_cls, (PyObject *) pt_cls))
    {
        case -1:
            return NULL;
        case 1:
            goto dinherit;
        case 0:
            break;
    }


    dict = PyDict_New();
    if (dict == NULL)
    {
        return NULL;
    }

    self = (Rohana_NM_Pool_Object *) cls->tp_alloc(cls, 0);
    if (self == NULL)
    {
        return NULL;
    }

    self->dict = dict;
    Py_INCREF(pt_cls);
    self->plain_text_cls = pt_cls;
    Py_INCREF(rn_cls);
    self->raw_node_cls = rn_cls;
    Py_INCREF(converter);
    self->converter = converter;
    return self;

}

static PyObject *Rohana_NM_Pool_Add(Rohana_NM_Pool_Object *self, PyObject *args)
{
    PyObject *key;
#if Rohana_NM_META_CLASS
    Rohana_NM_NodeMeta_Object *cls;
    Rohana_NM_NodeMeta_Object *scls;
#else
    PyTypeObject *cls;
    PyTypeObject *scls;
#endif

#if Rohana_NM_META_CLASS
    if (!PyArg_ParseTuple(args, "OO!", &key, &Rohana_NM_NodeMeta_Type, &cls))
#else
    if (!PyArg_ParseTuple(args, "OO", &key, &cls))
#endif
    {
        return NULL;
    }

#if !Rohana_NM_META_CLASS
    if (!PyType_IsSubtype(cls, &Rohana_NM_Node_Type))
    {
        PySys_FormatStdout("%R", cls);
        PyErr_Format(PyExc_TypeError, "node class must be subclass of '%s'", Rohana_NM_Node_Type.tp_name);
        return NULL;
    }
#endif

#if Rohana_NM_META_CLASS
    scls = (Rohana_NM_NodeMeta_Object *) PyDict_GetItemWithError(self->dict, key);
#else
    scls = (PyTypeObject *) PyDict_GetItemWithError(self->dict, key);
#endif
    if (scls == NULL && PyErr_Occurred())
    {
        return NULL;
    }
    if (scls != NULL)
    {
        if (!PyObject_TypeCheck(cls, scls))
        {
            PyErr_Format(PyExc_TypeError, "overriding node possible only by it's subclasses");
            return NULL;
        }
    }

    if (PyDict_SetItem(self->dict, key, (PyObject *) cls) != 0)
    {
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *Rohana_NM_Pool_Call(Rohana_NM_Pool_Object *self, PyObject *args, PyObject *kwargs)
{

    static char *kw_list[] = {"", NULL};
    PyObject *source;
    PyObject *key;
#if Rohana_NM_META_CLASS
    Rohana_NM_NodeMeta_Object *cls;
#else
    PyTypeObject *cls;
#endif
#if Rohana_NM_CustomKeyError
    Rohana_NM_UnexpectedNodeError_Object *exc;
    PyObject *exc_msg;
    PyObject *exc_args;
#endif
    Rohana_NM_NodeFactory_Object *bound;
    Rohana_NM_Node_Object *node;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &source))
    {
        return NULL;
    }

    switch (PyObject_IsInstance(source, (PyObject *) self->raw_node_cls))
    {
        case -1:
            return NULL;
        case 1:
            break;
        case 0:
            switch (PyObject_IsInstance(source, (PyObject *) self->plain_text_cls))
            {
                case 1:
                    Py_INCREF(source);
                    return source;
                case 0:
                    PyErr_Format(PyExc_TypeError, "don't know what to do with %R", Py_TYPE(source));
                case -1:
                    return NULL;
            }
    }

    key = PyObject_CallFunction(self->converter, "O", source);
    if (key == NULL)
    {
        return NULL;
    }

#if Rohana_NM_META_CLASS
    cls = (Rohana_NM_NodeMeta_Object *) PyDict_GetItemWithError(self->dict, key);
#else
    cls = (PyTypeObject *) PyDict_GetItemWithError(self->dict, key);
#endif
    if (cls == NULL)
    {
        if (!PyErr_Occurred())
        {

#if Rohana_NM_CustomKeyError
            exc_msg = PyUnicode_FromFormat("node class for key %R not found", key);
            if (exc_msg == NULL)
            {
                Py_DECREF(key);
                return NULL;
            }
            exc_args = Py_BuildValue("(O)", exc_msg);
            Py_DECREF(exc_msg);
            if (exc_args == NULL)
            {
                Py_DECREF(key);
                return NULL;
            }

            exc = (Rohana_NM_UnexpectedNodeError_Object *) Rohana_NM_UnexpectedNodeError_Type.tp_base->tp_new(&Rohana_NM_UnexpectedNodeError_Type, exc_args, NULL);
            if (exc == NULL)
            {
                Py_DECREF(exc_args);
                Py_DECREF(key);
                return NULL;
            }
            exc->key = key;
            Py_INCREF(source);
            exc->input = source;
            if (Rohana_NM_UnexpectedNodeError_Type.tp_base->tp_init((PyObject *) exc, exc_args, NULL) != 0)
            {
                Py_DECREF(exc);
                Py_DECREF(exc_args);
                return NULL;
            }
            Py_DECREF(exc_args);


            PyErr_SetObject((PyObject *) &Rohana_NM_UnexpectedNodeError_Type, (PyObject *) exc);
#else
            PyErr_Format(PyExc_KeyError, "%R", key);
            Py_DECREF(key);
#endif
            return NULL;

        }
        else
        {
            Py_DECREF(key);
        }
        return NULL;
    }
    Py_DECREF(key);

    bound = Rohana_NM_NodeMeta_GetItem(cls, self);
    Py_DECREF(cls);
    if (bound == NULL)
    {
        return NULL;
    }

    node = Rohana_NM_NodeFactory_Call(bound, args, kwargs);
    Py_DECREF(bound);
    if (node == NULL)
    {
        return NULL;
    }

    return (PyObject *) node;

}

static void Rohana_NM_Pool_Dealloc(Rohana_NM_Pool_Object *self)
{
    Py_DECREF(self->dict);
    Py_TYPE(self)->tp_free(self);
}

#if Rohana_NM_META_CLASS
static Rohana_NM_NodeMeta_Object *Rohana_NM_NodeMeta_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"key", "bases", "dct", NULL};
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
    return (Rohana_NM_NodeMeta_Object *) PyType_Type.tp_new(mcs, args, kwargs);;
}
#endif

#if Rohana_NM_META_CLASS
static Rohana_NM_NodeFactory_Object *Rohana_NM_NodeMeta_GetItem(Rohana_NM_NodeMeta_Object *cls, Rohana_NM_Pool_Object *pool)
#else

static Rohana_NM_NodeFactory_Object *Rohana_NM_NodeMeta_GetItem(PyTypeObject *cls, Rohana_NM_Pool_Object *pool)
#endif
{
    Rohana_NM_NodeFactory_Object *bound;


    if (cls == &Rohana_NM_Node_Type)
    {
        PyErr_Format(PyExc_TypeError, "%R is base type for subclassing, don't use if for binding with pool", cls);
        return NULL;
    }

    if (!PyObject_TypeCheck(pool, &Rohana_NM_Pool_Type))
    {
        PyErr_Format(PyExc_TypeError, "can bind node only to pool objects");
        return NULL;
    }

    bound = (Rohana_NM_NodeFactory_Object *) Rohana_NM_NodeFactory_Type.tp_alloc(&Rohana_NM_NodeFactory_Type, 0);
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

#if Rohana_NM_META_CLASS
static PyObject *Rohana_NM_NodeMeta_Call(Rohana_NM_NodeMeta_Object *cls, PyObject *args, PyObject *kwargs)
#else

static PyObject *Rohana_NM_Node_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
#endif
{
    PyErr_Format(
            PyExc_TypeError,
            "creating instances allowed only with bounding to nodes pool: %s[<%s object>](...)",
            cls->tp_name,
            Rohana_NM_Pool_Type.tp_name
    );
    return NULL;
}

#if Rohana_NM_META_CLASS
static void Rohana_NM_NodeMeta_Dealloc(Rohana_NM_NodeMeta_Object *cls)
{
    if (cls != &Rohana_NM_Node_Type)
    {
        Rohana_NM_NodeMeta_Type.tp_base->tp_dealloc((PyObject *) cls);
    }
}
#endif

static int Rohana_NM_Node_Init(Rohana_NM_Node_Object *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    PyObject *raw_node;
    PyObject *init_args;

    if (self->is_inited)
    {
        PyErr_Format(PyExc_RuntimeError, "'__init__' function called on already inited object");
        return -1;
    }
    self->is_inited = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &raw_node))
    {
        return -1;
    }

    switch (PyObject_IsInstance(raw_node, (PyObject *) self->pool->raw_node_cls))
    {
        case 1:
            break;
        case 0:
            PyErr_Format(PyExc_TypeError, "raw node type not corresponds to pool's settings");
        case -1:
            return -1;
    }

    init_args = PyTuple_New(0);
    if (init_args == NULL)
    {
        return -1;
    }

    if (Rohana_NM_Node_Type.tp_base->tp_init(self, init_args, NULL) != 0)
    {
        Py_DECREF(args);
        return -1;
    }

    Py_DECREF(args);
    return 0;
}

static void Rohana_NM_Node_Dealloc(Rohana_NM_Node_Object *self)
{
    Py_DECREF(self->pool);
    Py_TYPE(self)->tp_free(self);
}

static Rohana_NM_Node_Object *Rohana_NM_NodeFactory_Call(Rohana_NM_NodeFactory_Object *self, PyObject *args, PyObject *kwargs)
{
    Rohana_NM_Node_Object *node;
    PyObject *new_args;
    PyObject *init_func;
    PyObject *init_return;

    new_args = PyTuple_New(0);
    if (new_args == NULL)
    {
        return NULL;
    }

    node = (Rohana_NM_Node_Object *) Rohana_NM_Node_Type.tp_base->tp_new((PyTypeObject *) self->cls, new_args, NULL);
    Py_DECREF(new_args);
    if (node == NULL)
    {
        return NULL;
    }
    Py_INCREF(self->pool);
    node->pool = self->pool;
    node->is_inited = 0;

    init_func = PyObject_GetAttrString((PyObject *) node, "__init__");
    if (init_func == NULL)
    {
        Py_DECREF(self);
        return NULL;
    }
    init_return = PyObject_Call(init_func, args, kwargs);
    Py_DECREF(init_func);
    if (init_return == NULL)
    {
        Py_DECREF(node);
        return NULL;
    }
    Py_DECREF(init_return);
    return node;
}

static void Rohana_NM_NodeFactory_Dealloc(Rohana_NM_NodeFactory_Object *self)
{
    Py_DECREF(self->pool);
    Py_DECREF(self->cls);
    Py_TYPE(self)->tp_free(self);
}

static PyModuleDef module_def = {
        .m_name = "rohana.node_meta",
        .m_methods = NULL
};

PyMODINIT_FUNC PyInit_node_meta(void)
{
    PyObject *module;

#if Rohana_NM_CustomKeyError
    Rohana_NM_UnexpectedNodeError_Type.tp_base = (PyTypeObject *) PyExc_Exception;
    if (PyType_Ready(&Rohana_NM_UnexpectedNodeError_Type))
    {
        return NULL;
    }
#endif

    if (PyType_Ready(&Rohana_NM_Pool_Type))
    {
        return NULL;
    }
#if Rohana_NM_META_CLASS
    Rohana_NM_NodeMeta_Type.tp_base = &PyType_Type;
    Rohana_NM_NodeMeta_Type.tp_init = PyType_Type.tp_init;
    if (PyType_Ready(&Rohana_NM_NodeMeta_Type))
    {
        return NULL;
    }
#endif

    if (PyType_Ready((PyTypeObject *) &Rohana_NM_Node_Type))
    {
        return NULL;
    }

    if (PyType_Ready((PyTypeObject *) &Rohana_NM_NodeFactory_Type))
    {
        return NULL;
    }

    module = PyModule_Create(&module_def);
    if (module == NULL)
    {
        return NULL;
    }

    if (PyModule_AddObject(module, "pool", (PyObject *) &Rohana_NM_Pool_Type))
    {
        Py_DECREF(module);
        return NULL;
    }
#if Rohana_NM_META_CLASS
    if (PyModule_AddObject(module, "node_meta", (PyObject *) &Rohana_NM_NodeMeta_Type))
    {
        Py_DECREF(module);
        return NULL;
    }
#endif
    if (PyModule_AddObject(module, "node", (PyObject *) &Rohana_NM_Node_Type))
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "node_factory", (PyObject *) &Rohana_NM_NodeFactory_Type))
    {
        Py_DECREF(module);
        return NULL;
    }


    if (PyModule_AddObject(module, "InvalidNodeError", (PyObject *) Py_None))
    {
        Py_DECREF(module);
        return NULL;
    }
#if Rohana_NM_CustomKeyError
    if (PyModule_AddObject(module, "UnexpectedNodeError", (PyObject *) &Rohana_NM_UnexpectedNodeError_Type))
#else
    if (PyModule_AddObject(module, "UnexpectedNodeError", PyExc_KeyError))
#endif
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}