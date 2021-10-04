#include <Python.h>


typedef struct command_object {
    PyObject_VAR_HEAD
    PyObject *wrapped_callable;
    PyObject *command_names[1];
} command_object;

typedef struct dependency_decorator_object {
    PyObject_VAR_HEAD
    PyObject *command_names[1];
} dependency_decorator_object;

extern PyTypeObject command;

static void dependency_decorator_dealloc(dependency_decorator_object *self)
{
    Py_ssize_t i;

    for (i = Py_SIZE(self) - 1; i >= 0; i--)
    {
        Py_DECREF(self->command_names[i]);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject* dependency_decorator_call(dependency_decorator_object *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    command_object *dwo;
    Py_ssize_t i;
    PyObject *wrapped_callable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &wrapped_callable))
    {
        return NULL;
    }

    dwo = (command_object *)command.tp_alloc(&command, Py_SIZE(self));
    if (dwo == NULL)
    {
        return PyErr_NoMemory();
    }
    Py_INCREF(wrapped_callable);
    dwo->wrapped_callable = wrapped_callable;
    for (i = Py_SIZE(self) - 1; i >= 0; i--)
    {
        Py_INCREF(self->command_names[i]);
        dwo->command_names[i] = self->command_names[i];
    }
    return (PyObject *)dwo;
}

static PyTypeObject dependency_decorator = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.dependency_decorator",
    .tp_basicsize = sizeof(dependency_decorator_object) - sizeof(PyObject *[1]),
    .tp_itemsize = sizeof(PyObject *),
    .tp_dealloc = (destructor)dependency_decorator_dealloc,
    .tp_call = (ternaryfunc)dependency_decorator_call
};

static PyObject *command_new(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    PyObject *wrapped_callable;
    command_object *self;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &wrapped_callable))
    {
        return NULL;
    }

    self = (command_object *)command.tp_alloc(&command, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    Py_INCREF(wrapped_callable);
    self->wrapped_callable = wrapped_callable;
    return (PyObject *)self;

}

static void command_dealloc(command_object *self)
{
    Py_ssize_t i;

    Py_XDECREF(self->wrapped_callable);
    for (i = Py_SIZE(self) - 1; i >= 0; i--)
    {
        Py_DECREF(self->command_names[i]);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject *command_getattr(command_object *self, char *attrname)
{
    return PyObject_GetAttrString(self->wrapped_callable, attrname);
}
static int command_setattr(command_object *self, char *attrname, PyObject *value)
{
    return PyObject_SetAttrString(self->wrapped_callable, attrname, value);
}

static PyObject *command_getattro(command_object *self, PyObject *attrname)
{
    return PyObject_GetAttr(self->wrapped_callable, attrname);
}
static int command_setattro(command_object *self, PyObject *attrname, PyObject *value)
{
    return PyObject_SetAttr(self->wrapped_callable, attrname, value);
}

static PyObject* command_call(command_object *self, PyObject *args, PyObject *kwargs)
{
    return PyObject_Call(self->wrapped_callable, args, kwargs);
}


static PyObject* command_depends(PyObject *_self, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {NULL};
    PyObject *fake_args;
    dependency_decorator_object *self;
    Py_ssize_t i;

    fake_args = PyTuple_New(0);
    if (fake_args == NULL)
    {
        return NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(fake_args, kwargs, "$", kw_list))
    {
        return NULL;
    }
    if (args == NULL)
    {
        args = fake_args;
        Py_INCREF(fake_args);
    }
    else
    {
        if (!PyTuple_CheckExact(args))
        {
            Py_DECREF(fake_args);
            PyErr_Format(PyExc_TypeError, "*args is not a tuple (%R)", Py_TYPE(args));
            return NULL;
        }
        Py_INCREF(args);
    }
    Py_DECREF(fake_args);

    self = (dependency_decorator_object *)dependency_decorator.tp_alloc(&dependency_decorator, PyTuple_GET_SIZE(args));
    if (self == NULL)
    {
        Py_DECREF(args);
        return PyErr_NoMemory();
    }

    for (i = PyTuple_GET_SIZE(args) - 1; i >= 0; i--)
    {
        if (!PyUnicode_CheckExact(PyTuple_GET_ITEM(args, i)))
        {
            PyErr_Format(
                PyExc_TypeError,
                "Dependency list must contain only strings, but elem %zd is not (%R)",
                i,
                Py_TYPE(PyTuple_GET_ITEM(args, i))
            );
            Py_DECREF(args);
            return NULL;
        }
        Py_INCREF(PyTuple_GET_ITEM(args, i));
        self->command_names[i] = PyTuple_GET_ITEM(args, i);
    }
    Py_DECREF(args);
    return (PyObject *)self;
}

static PyMethodDef command_methods[] = {
    {"depends", (PyCFunction)command_depends, METH_CLASS | METH_VARARGS | METH_KEYWORDS},
    {NULL}
};

static PyTypeObject command = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.command",
    .tp_basicsize = sizeof(command_object) - sizeof(PyObject *[1]),
    .tp_itemsize = sizeof(PyObject *),
    .tp_new = (newfunc)command_new,
    .tp_dealloc = (destructor)command_dealloc,
    .tp_getattr = (getattrfunc)command_getattr,
    .tp_setattr = (setattrfunc)command_setattr,
    .tp_getattro = (getattrofunc)command_getattro,
    .tp_setattro = (setattrofunc)command_setattro,
    .tp_call = (ternaryfunc)command_call,
    .tp_methods = command_methods,

};


static PyObject *get_dependencies(PyObject *module, PyObject *dp)
{
    PyObject *out;
    Py_ssize_t i;
    PyObject **it;

    if (Py_TYPE(dp) == &dependency_decorator)
    {
        i = Py_SIZE(dp);
        it = ((dependency_decorator_object *)dp)->command_names;
    }
    else if (Py_TYPE(dp) == &command)
    {
        i = Py_SIZE(dp);
        it = ((command_object *)dp)->command_names;
    }
    else
    {
        PyErr_Format(PyExc_TypeError, "Can't extract dependencies from '%R'", Py_TYPE(dp));
        return 0;
    }

    out = PyTuple_New(i);
    if (out == NULL)
    {
        return NULL;
    }

    for (i--; i >= 0; i--)
    {
        Py_INCREF(it[i]);
        PyTuple_SET_ITEM(out, i, it[i]);
    }
    return out;
}

static PyObject *unwrap_command(PyObject *module, PyObject *cmd)
{
    if (Py_TYPE(cmd) == &command)
    {
        Py_INCREF(((command_object *)cmd)->wrapped_callable);
        return ((command_object *)cmd)->wrapped_callable;
    }
    else
    {
        PyErr_Format(PyExc_TypeError, "Can't extract only from commands");
        return NULL;
    }
}
static PyMethodDef module_methods[] = {
    {"get_dependencies", (PyCFunction)get_dependencies, METH_O},
    {"unwrap_command", (PyCFunction)unwrap_command, METH_O},
    {NULL}
};

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "rohana.inspect",
    .m_methods = module_methods
};

PyMODINIT_FUNC PyInit__meta(void)
{
    PyObject *module;
    module = PyModule_Create(&module_def);

    if (PyType_Ready(&dependency_decorator))
    {
        return NULL;
    }

    if (PyType_Ready(&command))
    {
        return NULL;
    }

    if (module == NULL)
    {
        return NULL;
    }

    if (PyModule_AddObject(module, "command", (PyObject *)&command) != 0)
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}