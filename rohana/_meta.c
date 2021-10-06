#include <Python.h>


typedef struct RohanaMeta_Command_Object
{
    PyObject_VAR_HEAD
    PyObject *wrapped_callable;
    PyObject *command_names[1];
} RohanaMeta_Command_Object;

typedef struct RohanaMeta_CommandDependencyDecorator_Object
{
    PyObject_VAR_HEAD
    PyObject *command_names[1];
} RohanaMeta_CommandDependencyDecorator_Object;


typedef struct RohanaMeta_GeneratorMeta_Object
{
    PyHeapTypeObject ht;
    PyObject *commands_dict;
} RohanaMeta_GeneratorMeta_Object;


typedef struct RohanaMeta_BoundCommand_Object
{
    PyObject_HEAD
    RohanaMeta_GeneratorMeta_Object *generator;
    RohanaMeta_Command_Object *command;
} RohanaMeta_BoundCommand_Object;

extern PyTypeObject RohanaMeta_Command_Type;
extern PyTypeObject RohanaMeta_BoundCommand_Type;
extern PyTypeObject RohanaMeta_GeneratorMeta_Type;

static void RohanaMeta_CommandDependencyDecorator_Dealloc(RohanaMeta_CommandDependencyDecorator_Object *self)
{
    Py_ssize_t i;

    for (i = Py_SIZE(self) - 1; i >= 0; i--)
    {
        Py_DECREF(self->command_names[i]);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject *RohanaMeta_CommandDependencyDecorator_Call(RohanaMeta_CommandDependencyDecorator_Object *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    RohanaMeta_Command_Object *dwo;
    Py_ssize_t i;
    PyObject *wrapped_callable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &wrapped_callable))
    {
        return NULL;
    }

    dwo = (RohanaMeta_Command_Object *) RohanaMeta_Command_Type.tp_alloc(&RohanaMeta_Command_Type, Py_SIZE(self));
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
    return (PyObject *) dwo;
}

static PyTypeObject RohanaMeta_CommandDependencyDecorator_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.command_dependency_decorator",
    .tp_basicsize = sizeof(RohanaMeta_CommandDependencyDecorator_Object) - sizeof(PyObject *[1]),
    .tp_itemsize = sizeof(PyObject *),
    .tp_dealloc = (destructor) RohanaMeta_CommandDependencyDecorator_Dealloc,
    .tp_call = (ternaryfunc) RohanaMeta_CommandDependencyDecorator_Call
};

static PyObject *RohanaMeta_Command_New(PyTypeObject *Py_UNUSED(cls), PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    PyObject *wrapped_callable;
    RohanaMeta_Command_Object *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &wrapped_callable))
    {
        return NULL;
    }

    self = (RohanaMeta_Command_Object *) RohanaMeta_Command_Type.tp_alloc(&RohanaMeta_Command_Type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    Py_INCREF(wrapped_callable);
    self->wrapped_callable = wrapped_callable;
    return (PyObject *) self;

}

static void RohanaMeta_Command_Dealloc(RohanaMeta_Command_Object *self)
{
    Py_ssize_t i;

    Py_DECREF(self->wrapped_callable);
    for (i = Py_SIZE(self) - 1; i >= 0; i--)
    {
        Py_DECREF(self->command_names[i]);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject *RohanaMeta_Command_Getattr(RohanaMeta_Command_Object *self, char *attrname)
{
    return PyObject_GetAttrString(self->wrapped_callable, attrname);
}

static int RohanaMeta_Command_Setattr(RohanaMeta_Command_Object *self, char *attrname, PyObject *value)
{
    return PyObject_SetAttrString(self->wrapped_callable, attrname, value);
}

static PyObject *RohanaMeta_Command_GetattrO(RohanaMeta_Command_Object *self, PyObject *attrname)
{
    return PyObject_GetAttr(self->wrapped_callable, attrname);
}

static int RohanaMeta_Command_SetattrO(RohanaMeta_Command_Object *self, PyObject *attrname, PyObject *value)
{
    return PyObject_SetAttr(self->wrapped_callable, attrname, value);
}

static PyObject *RohanaMeta_Command_Call(RohanaMeta_Command_Object *self, PyObject *args, PyObject *kwargs)
{
    return PyObject_Call(self->wrapped_callable, args, kwargs);
}

static PyObject *RohanaMeta_Command_DescrGet(RohanaMeta_Command_Object *self, PyObject *instance, PyObject *owner)
{
    RohanaMeta_BoundCommand_Object *bound;
    if (instance == NULL || instance == Py_None)
    {
        if (Py_TYPE(owner) != &RohanaMeta_GeneratorMeta_Type)
        {
            Py_INCREF(self);
            return (PyObject *) self;
        }
        bound = (RohanaMeta_BoundCommand_Object *) RohanaMeta_BoundCommand_Type.tp_alloc(&RohanaMeta_BoundCommand_Type, 0);
        if (bound == NULL)
        {
            return PyErr_NoMemory();
        }
        Py_INCREF(self);
        bound->command = self;
        Py_INCREF(owner);
        bound->generator = (RohanaMeta_GeneratorMeta_Object *) owner;
        return (PyObject *) bound;
    }
    if (Py_TYPE(Py_TYPE(instance)) == &RohanaMeta_GeneratorMeta_Type)
    {
        PyErr_Format(PyExc_TypeError, "you mustn't access other command while executing on of them");
        return NULL;
    }
    Py_INCREF(self);
    return (PyObject *) self;
}

static int RohanaMeta_Command_Traverse(RohanaMeta_Command_Object *self, visitproc visit, void *arg)
{
    Py_VISIT(self->wrapped_callable);
    return 0;
}

static PyObject *RohanaMeta_Command_DependsFunc(PyObject *Py_UNUSED(cls), PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {NULL};
    PyObject *fake_args;
    RohanaMeta_CommandDependencyDecorator_Object *self;
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

    self = (RohanaMeta_CommandDependencyDecorator_Object *) RohanaMeta_CommandDependencyDecorator_Type.tp_alloc(&RohanaMeta_CommandDependencyDecorator_Type, PyTuple_GET_SIZE(args));
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
                "dependency list must contain only strings, but elem %zd is not (%R)",
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
    return (PyObject *) self;
}

static PyMethodDef RohanaMeta_Command_Methods[] = {
    {"depends", (PyCFunction) RohanaMeta_Command_DependsFunc, METH_CLASS | METH_VARARGS | METH_KEYWORDS},
    {NULL}
};

static PyTypeObject RohanaMeta_Command_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.command",
    .tp_basicsize = sizeof(RohanaMeta_Command_Object) - sizeof(PyObject *[1]),
    .tp_itemsize = sizeof(PyObject *),
    .tp_new = (newfunc) RohanaMeta_Command_New,
    .tp_dealloc = (destructor) RohanaMeta_Command_Dealloc,
    .tp_getattr = (getattrfunc) RohanaMeta_Command_Getattr,
    .tp_setattr = (setattrfunc) RohanaMeta_Command_Setattr,
    .tp_getattro = (getattrofunc) RohanaMeta_Command_GetattrO,
    .tp_setattro = (setattrofunc) RohanaMeta_Command_SetattrO,
    .tp_traverse = (traverseproc) RohanaMeta_Command_Traverse,
    .tp_descr_get = (descrgetfunc) RohanaMeta_Command_DescrGet,
    .tp_call = (ternaryfunc) RohanaMeta_Command_Call,
    .tp_methods = RohanaMeta_Command_Methods

};

static void RohanaMeta_BoundCommand_Dealloc(RohanaMeta_BoundCommand_Object *self)
{
    Py_DECREF(self->generator);
    Py_DECREF(self->command);
    Py_TYPE(self)->tp_free(self);
}


static PyObject *RohanaMeta_BoundCommand_Getattr(RohanaMeta_BoundCommand_Object *self, char *attrname)
{
    return RohanaMeta_Command_Getattr(self->command, attrname);
}

static int RohanaMeta_BoundCommand_Setattr(RohanaMeta_BoundCommand_Object *self, char *attrname, PyObject *value)
{
    return RohanaMeta_Command_Setattr(self->command, attrname, value);
}

static PyObject *RohanaMeta_BoundCommand_GetattrO(RohanaMeta_BoundCommand_Object *self, PyObject *attrname)
{
    return RohanaMeta_Command_GetattrO(self->command, attrname);
}

static int RohanaMeta_BoundCommand_SetattrO(RohanaMeta_BoundCommand_Object *self, PyObject *attrname, PyObject *value)
{
    return RohanaMeta_Command_SetattrO(self->command, attrname, value);
}


static int RohanaMeta_BoundCommand_SynchronizedCall(RohanaMeta_GeneratorMeta_Object *generator_type, PyObject *command_name, RohanaMeta_Command_Object *command_object, PyObject *complete_set, PyObject *args)
{
    Py_ssize_t i;
    PyObject *ret;

    if (command_name == NULL && command_object == NULL)
    {
        PyErr_BadInternalCall();
        return -1;
    }

    if (command_name != NULL)
    {
        switch (PySet_Contains(complete_set, command_name))
        {
            case 1:
                return 0;
            case 0:
                break;
            case -1:
                return -1;
        }
    }


    if (command_object == NULL)
    {
        command_object = (RohanaMeta_Command_Object *) PyDict_GetItemWithError(generator_type->commands_dict, command_name);
        if (command_object == NULL)
        {
            if (!

                PyErr_Occurred()

                )
            {
                PyErr_Format(PyExc_RuntimeError,
                             "reference to command '%U' was lost", command_name);
            }
            return -1;
        }
    }

    for (i = Py_SIZE(command_object) - 1; i >= 0; i--)
    {
        if (
            RohanaMeta_BoundCommand_SynchronizedCall(generator_type, command_object->command_names[i], NULL, complete_set, args) != 0)
        {
            return -1;
        }
    }
    if (command_name != NULL)
    {
        switch (PySet_Add(complete_set, command_name))
        {
            case 0:
                break;
            case -1:
                return -1;
        }
    }

    ret = PyObject_CallObject(command_object->wrapped_callable, args);
    if (ret == NULL)
    {
        return -1;
    }
    Py_DECREF(ret);
    return 0;
}

static PyObject *RohanaMeta_BoundCommand_Call(RohanaMeta_BoundCommand_Object *self, PyObject *argv, PyObject *kwargs)
{
    static char *kw_list[] = {NULL};
    PyObject *fake_args;
    PyObject *complete_commands;
    PyObject *generator_instance;
    PyObject *args;
    PyObject *init_func;
    PyObject *ret;

    fake_args = PyTuple_New(0);
    if (fake_args == NULL)
    {
        return NULL;
    }
    if (!PyArg_ParseTupleAndKeywords(fake_args, kwargs, "", kw_list))
    {
        Py_DECREF(fake_args);
        return NULL;
    }
    Py_DECREF(fake_args);

    complete_commands = PySet_New(NULL);
    if (complete_commands == NULL)
    {
        return NULL;
    }

    generator_instance = self->generator->ht.ht_type.tp_alloc((struct _typeobject *) self->generator, 0);
    if (generator_instance == NULL)
    {
        Py_DECREF(complete_commands);
        return PyErr_NoMemory();
    }

    if (self->generator->ht.ht_type.tp_init != NULL)
    {

        if (self->generator->ht.ht_type.tp_init(generator_instance, argv, NULL) != 0)
        {
            Py_DECREF(generator_instance);
            Py_DECREF(complete_commands);
            return NULL;
        }
    }
    else
    {
        init_func = PyObject_GetAttrString(generator_instance, "__init__");
        if (init_func == NULL)
        {
            Py_DECREF(generator_instance);
            Py_DECREF(complete_commands);
            return NULL;
        }
        ret = PyObject_CallObject(init_func, argv);
        Py_DECREF(init_func);
        if (ret == NULL)
        {
            Py_DECREF(generator_instance);
            Py_DECREF(complete_commands);
            return NULL;
        }
        Py_DECREF(ret);
    }
    args = PyTuple_Pack(1, generator_instance);
    Py_DECREF(generator_instance);
    if (args == NULL)
    {
        Py_DECREF(complete_commands);
        return NULL;
    }

    argv = PySequence_InPlaceConcat(args, argv);
    if (argv == NULL)
    {
        Py_DECREF(args);
        Py_DECREF(complete_commands);
        return NULL;
    }
    Py_DECREF(args);
    args = argv;

    if (RohanaMeta_BoundCommand_SynchronizedCall(self->generator, NULL, self->command, complete_commands, args) != 0)
    {
        Py_DECREF(args);
        Py_DECREF(complete_commands);
        return NULL;
    }
    Py_DECREF(args);
    Py_DECREF(complete_commands);
    Py_RETURN_NONE;
}

static int RohanaMeta_BoundCommand_Traverse(RohanaMeta_BoundCommand_Object *self, visitproc visit, void *arg)
{
    Py_VISIT(self->generator);
    Py_VISIT(self->command);
    return 0;
}

static PyTypeObject RohanaMeta_BoundCommand_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.bound_command",
    .tp_basicsize = sizeof(RohanaMeta_BoundCommand_Object),
    .tp_dealloc = (destructor) RohanaMeta_BoundCommand_Dealloc,
    .tp_getattr = (getattrfunc) RohanaMeta_BoundCommand_Getattr,
    .tp_setattr = (setattrfunc) RohanaMeta_BoundCommand_Setattr,
    .tp_getattro = (getattrofunc) RohanaMeta_BoundCommand_GetattrO,
    .tp_setattro = (setattrofunc) RohanaMeta_BoundCommand_SetattrO,
    .tp_call = (ternaryfunc) RohanaMeta_BoundCommand_Call,
    .tp_traverse = (traverseproc) RohanaMeta_BoundCommand_Traverse
};

extern RohanaMeta_GeneratorMeta_Object RohanaMeta_Generator_Type;

static int RohanaMeta_GeneratorMeta_SetAttr(RohanaMeta_GeneratorMeta_Object *cls, char *Py_UNUSED(attr), PyObject *Py_UNUSED(value))
{
    PyErr_Format(PyExc_TypeError, "can't set attributes of rohana generator class '%s'", cls->ht.ht_type.tp_name);
    return -1;
}

static int RohanaMeta_GeneratorMeta_SetAttrO(RohanaMeta_GeneratorMeta_Object *cls, PyObject *Py_UNUSED(attr), PyObject *Py_UNUSED(value))
{
    PyErr_Format(PyExc_TypeError, "can't set attributes of rohana generator class '%s'", cls->ht.ht_type.tp_name);
    return -1;
}

static PyObject *RohanaMeta_GeneratorMeta_New(PyTypeObject *Py_UNUSED(mcs), PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"name", "bases", "dct", NULL};
    PyObject *name;
    PyObject *bases;
    PyObject *dct;
    PyObject *new_cmd_dct = NULL;
    Py_ssize_t i;
    Py_ssize_t dct_iter_pos;
    PyObject *dct_key;
    PyObject *dct_value;
    PyObject *commands_left = NULL;
    struct
    {
        struct commands_stack_node
        {
            PyObject *name;
            RohanaMeta_Command_Object *obj;
            Py_ssize_t pos;
        } *array;
        Py_ssize_t len;
    } commands_stack = {NULL, 0};
    PyObject *dependency_loop;
    Py_ssize_t j;
    RohanaMeta_GeneratorMeta_Object *cls = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!", kw_list, &PyUnicode_Type, &name, &PyTuple_Type, &bases, &PyDict_Type, &dct))
    {
        goto err;
    }
    if (!PyArg_ValidateKeywordArguments(dct))
    {
        PyErr_Format(PyExc_TypeError, "'dct' keys are not only strings");
        goto err;
    }

    if (PyDict_GetItemString(dct, "__new__") != NULL)
    {
        PyErr_WarnFormat(PyExc_Warning, -1, "magic method '__new__' has no effect, use '__init__'");
    }

    if (PyTuple_GET_SIZE(bases) == 0)
    {
        PyErr_Format(PyExc_TypeError, "rohana generator must inherit '%s' or it's subclasses", RohanaMeta_Generator_Type.ht.ht_type.tp_name);
        goto err;
    }

    new_cmd_dct = PyDict_New();
    if (new_cmd_dct == NULL)
    {
        goto err;
    }

    for (i = PyTuple_GET_SIZE(bases) - 1; i >= 0; i--)
    {
        if (Py_TYPE(PyTuple_GET_ITEM(bases, i)) == &RohanaMeta_GeneratorMeta_Type)
        {
            if (PyTuple_GET_ITEM(bases, i) != (PyObject *) &RohanaMeta_Generator_Type)
            {
                switch (PyDict_Update(new_cmd_dct, ((RohanaMeta_GeneratorMeta_Object *) PyTuple_GET_ITEM(bases, i))->commands_dict))
                {
                    case 0:
                        break;
                    case -1:
                        goto err;
                    default:
                        Py_UNREACHABLE();
                        goto err;
                }
            }
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "rohana generator can inherits only other generator classes");
            goto err;
        }
    }

    dct_iter_pos = 0;
    while (PyDict_Next(dct, &dct_iter_pos, &dct_key, &dct_value))
    {
        if (Py_TYPE(dct_value) == &RohanaMeta_Command_Type)
        {
            switch (PyDict_SetItem(new_cmd_dct, dct_key, dct_value))
            {
                case 0:
                    break;
                case -1:
                    goto err;
                default:
                    Py_UNREACHABLE();
                    goto err;
            }
        }
    }
    dct_key = PyDict_Keys(new_cmd_dct);
    if (dct_key == NULL)
    {
        goto err;
    }

    commands_left = PySet_New(dct_key);
    Py_DECREF(dct_key);
    if (commands_left == NULL)
    {
        goto err;
    }

    commands_stack.array = PyMem_Calloc(PySet_GET_SIZE(commands_left) + 1, sizeof(struct commands_stack_node));
    if (commands_stack.array == NULL)
    {
        PyErr_NoMemory();
        goto err;
    }
    while (PySet_GET_SIZE(commands_left) > 0 || commands_stack.len > 0)
    {
        if (commands_stack.len == 0)
        {
            commands_stack.array[commands_stack.len++] = (struct commands_stack_node) {PySet_Pop(commands_left), NULL, 0};
            if (commands_stack.array[commands_stack.len - 1].name == NULL)
            {
                goto err;
            }
        }
        else
        {
            if (commands_stack.array[commands_stack.len - 1].pos >= Py_SIZE(commands_stack.array[commands_stack.len - 1].obj))
            {
                Py_DECREF(commands_stack.array[--commands_stack.len].obj);
                Py_DECREF(commands_stack.array[commands_stack.len].name);
                continue;
            }
            commands_stack.array[commands_stack.len] = (struct commands_stack_node) {commands_stack.array[commands_stack.len - 1].obj->command_names[commands_stack.array[commands_stack.len - 1].pos++], NULL, 0};
            Py_INCREF(commands_stack.array[commands_stack.len++].name);
            for (i = commands_stack.len - 2; i >= 0; i--)
            {
                switch (PyUnicode_Compare(commands_stack.array[commands_stack.len - 1].name, commands_stack.array[i].name))
                {
                    case 0:
                        dependency_loop = PyTuple_New(commands_stack.len - i);
                        if (dependency_loop == NULL)
                        {
                            goto err;
                        }
                        for (j = 0; i < commands_stack.len; j++, i++)
                        {
                            Py_INCREF(commands_stack.array[i].name);
                            PyTuple_SET_ITEM(dependency_loop, j, commands_stack.array[i].name);
                        }
                        PyErr_Format(PyExc_ValueError, "dependency loop detected %S", dependency_loop);
                        Py_DECREF(dependency_loop), dependency_loop = NULL;
                        goto err;
                    case -1:
                        if (PyErr_Occurred())
                        {
                            goto err;
                        }
                    case 1:
                        break;
                }
            }
        }
        switch (PyDict_Contains(new_cmd_dct, commands_stack.array[commands_stack.len - 1].name))
        {
            case 1:
                break;
            case 0:
                if (commands_stack.len > 1)
                {
                    PyErr_Format(PyExc_ValueError, "command '%U' not found (dependency of command '%U')", commands_stack.array[commands_stack.len - 1].name, commands_stack.array[commands_stack.len - 2].name);
                }
                else
                {
                    PyErr_Format(PyExc_RuntimeError, "can't get command '%s' from dict", commands_stack.array[commands_stack.len - 1].name);
                }
            case -1:
                goto err;
        }

        commands_stack.array[commands_stack.len - 1].obj = (RohanaMeta_Command_Object *) PyDict_GetItem(new_cmd_dct, commands_stack.array[commands_stack.len - 1].name);
        Py_INCREF(commands_stack.array[commands_stack.len - 1].obj);
        if (commands_stack.array[commands_stack.len - 1].obj == NULL)
        {
            goto err;
        }

    }
    PyMem_Free(commands_stack.array), commands_stack.array = NULL;
    Py_DECREF(commands_left), commands_left = NULL;

    cls = (RohanaMeta_GeneratorMeta_Object *) RohanaMeta_GeneratorMeta_Type.tp_alloc(&RohanaMeta_GeneratorMeta_Type, 0);
    if (cls == NULL)
    {
        PyErr_NoMemory();
        goto err;
    }

    cls->ht.ht_type.tp_name = PyUnicode_AsUTF8(name);
    if (cls->ht.ht_type.tp_name == NULL)
    {
        goto err;
    }
    Py_INCREF(name);
    cls->ht.ht_name = name;
    Py_INCREF(bases);
    cls->ht.ht_type.tp_bases = bases;
    Py_INCREF(dct);
    cls->ht.ht_type.tp_dict = dct;
    cls->ht.ht_qualname = PyDict_GetItemString(dct, "__qualname__");
    if (cls->ht.ht_qualname == NULL)
    {
        cls->ht.ht_qualname = PyDict_GetItemString(dct, "__module__");
        if (cls->ht.ht_qualname == NULL)
        {
            Py_INCREF(name);
            cls->ht.ht_qualname = name;
        }
        else
        {
            cls->ht.ht_qualname = PyUnicode_FromFormat("%U.%U", cls->ht.ht_qualname, name);
            if (cls->ht.ht_qualname == NULL)
            {
                goto err;
            }
        }
    }
    else
    {
        Py_INCREF(cls->ht.ht_qualname);
    }
    cls->commands_dict = new_cmd_dct;
    cls->ht.ht_type.tp_flags = Py_TPFLAGS_HEAPTYPE;
    cls->ht.ht_type.tp_setattr = (setattrfunc) RohanaMeta_GeneratorMeta_SetAttr;
    cls->ht.ht_type.tp_setattro = (setattrofunc) RohanaMeta_GeneratorMeta_SetAttrO;
    if (PyType_Ready((PyTypeObject *) cls))
    {
        Py_DECREF(cls);
        return NULL;
    }

    return (PyObject *) cls;

    err:
    Py_XDECREF(cls);
    if (commands_stack.array != NULL)
    {
        while (commands_stack.len > 0)
        {
            Py_XDECREF(commands_stack.array[--(commands_stack.len)].name);
            Py_XDECREF(commands_stack.array[commands_stack.len].obj);
        }
        PyMem_Free(commands_stack.array);
    }
    Py_XDECREF(commands_left);
    Py_XDECREF(new_cmd_dct);
    return NULL;
}


static void RohanaMeta_GeneratorMeta_Dealloc(RohanaMeta_GeneratorMeta_Object *cls)
{
    Py_DECREF(cls->commands_dict);
    PyType_Type.tp_dealloc((PyObject *) cls);
}

static int RohanaMeta_GeneratorMeta_Traverse(RohanaMeta_GeneratorMeta_Object *cls, visitproc visit, void *arg)
{
    Py_VISIT(cls->commands_dict);
    if (PyType_Type.tp_traverse != NULL)
    {
        return PyType_Type.tp_traverse((PyObject *) cls, visit, arg);
    }
    else
    {
        return 0;
    }
}

static int RohanaMeta_GeneratorMeta_Clear(RohanaMeta_GeneratorMeta_Object *cls)
{
    if (PyType_Type.tp_clear != NULL)
    {
        return PyType_Type.tp_clear((PyObject *) cls);
    }
    else
    {
        return 0;
    }
}

static PyObject *RohanaMeta_GeneratorMeta_Call(RohanaMeta_GeneratorMeta_Object *cls, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    PyObject *fake_args;
    PyObject *command_name = NULL;
    RohanaMeta_BoundCommand_Object bound_entry_object = {PyObject_HEAD_INIT(&RohanaMeta_BoundCommand_Type) cls, NULL};
    PyObject *ret;

    if (args != NULL && PyTuple_GET_SIZE(args) >= 1)
    {
        fake_args = PyTuple_GetSlice(args, 0, 1);
        if (fake_args == NULL)
        {
            return NULL;
        }
        args = PyTuple_GetSlice(args, 1, PyTuple_GET_SIZE(args));
        if (args == NULL)
        {
            Py_DECREF(fake_args);
            return NULL;
        }
    }
    else
    {
        fake_args = PyTuple_New(0);
        if (fake_args == NULL)
        {
            return NULL;
        }
        Py_INCREF(args);
    }

    if (!PyArg_ParseTupleAndKeywords(fake_args, kwargs, "|U", kw_list, &command_name))
    {
        Py_DECREF(fake_args);
        Py_DECREF(args);
        return NULL;
    }
    if (command_name == NULL)
    {
        Py_DECREF(fake_args);
        Py_DECREF(args);
        args = PySys_GetObject("argv");
        if (args == NULL)
        {
            return NULL;
        }
        if (!PyList_CheckExact(args))
        {
            PyErr_Format(PyExc_TypeError, "'sys.argv' is not a list");
            return NULL;
        }
        if (PyList_GET_SIZE(args) < 2)
        {
            PyErr_Format(PyExc_ValueError, "command to run not passed");
            return NULL;
        }
        command_name = PyList_GET_ITEM(args, 1);
        if (command_name == NULL)
        {
            return NULL;
        }
        fake_args = PyList_GetSlice(args, 2, PyList_GET_SIZE(args));
        if (fake_args == NULL)
        {
            return NULL;
        }
        args = PyList_AsTuple(fake_args);
        Py_DECREF(fake_args);
        if (args == NULL)
        {
            return NULL;
        }
    }
    else
    {
        Py_DECREF(fake_args);
    }
    bound_entry_object.command = (RohanaMeta_Command_Object *) PyDict_GetItemWithError(cls->commands_dict, command_name);
    if (bound_entry_object.command == NULL)
    {
        if (!PyErr_Occurred())
        {
            PyErr_Format(PyExc_ValueError, "command '%U' not found", command_name);
        }
        Py_DECREF(args);
        return NULL;
    }

    ret = RohanaMeta_BoundCommand_Call(&bound_entry_object, args, NULL);
    Py_DECREF(args);

    if (ret == NULL)
    {
        return NULL;
    }
    Py_DECREF(ret);
    Py_RETURN_NONE;
}


static PyTypeObject RohanaMeta_GeneratorMeta_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "rohana.meta.rohana_generator_meta",
    .tp_basicsize = sizeof(RohanaMeta_GeneratorMeta_Object),
    .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS | Py_TPFLAGS_HAVE_GC,
    .tp_new = (newfunc) RohanaMeta_GeneratorMeta_New,
    .tp_dealloc = (destructor) RohanaMeta_GeneratorMeta_Dealloc,
    .tp_traverse = (traverseproc) RohanaMeta_GeneratorMeta_Traverse,
    .tp_clear = (inquiry) RohanaMeta_GeneratorMeta_Clear,
    .tp_call = (ternaryfunc) RohanaMeta_GeneratorMeta_Call,
    .tp_setattr = (setattrfunc) RohanaMeta_GeneratorMeta_SetAttr,
    .tp_setattro = (setattrofunc) RohanaMeta_GeneratorMeta_SetAttrO
};


static int RohanaMeta_Generator_Init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {NULL};
    PyObject *fake_args;
    Py_ssize_t i;

    fake_args = PyTuple_New(0);
    if (fake_args == NULL)
    {
        return -1;
    }

    if (!PyArg_ParseTupleAndKeywords(fake_args, kwargs, "", kw_list))
    {
        Py_DECREF(fake_args);
        return -1;
    }
    Py_DECREF(fake_args);

    for (i = PyTuple_GET_SIZE(args) - 1; i >= 0; i--)
    {
        if (!PyUnicode_CheckExact(PyTuple_GET_ITEM(args, i)))
        {
            PyErr_Format(PyExc_TypeError, "argument %zd is not a string (%s)", i, Py_TYPE(PyTuple_GET_ITEM(args, i))->tp_name);
            return -1;
        }
    }
    return 0;
}

static void RohanaMeta_Generator_Dealloc(PyObject *self)
{
    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free(self);
    if (tp != (PyTypeObject *) &RohanaMeta_Generator_Type)
    {
        Py_DECREF(tp);
    }
}

static RohanaMeta_GeneratorMeta_Object RohanaMeta_Generator_Type = {
    {
        {
            PyVarObject_HEAD_INIT(&RohanaMeta_GeneratorMeta_Type, 0)
            .tp_name = "rohana.meta.rohana_generator",
            .tp_init = (initproc) RohanaMeta_Generator_Init,
            .tp_dealloc = (destructor) RohanaMeta_Generator_Dealloc,
            .tp_flags = Py_TPFLAGS_BASETYPE
        }
    },
    NULL
};


static PyObject *RohanaInspect_GetDependencies(PyObject *Py_UNUSED(module), PyObject *dp)
{
    PyObject *out;
    Py_ssize_t i;
    PyObject **it;

    if (Py_TYPE(dp) == &RohanaMeta_CommandDependencyDecorator_Type)
    {
        i = Py_SIZE(dp);
        it = ((RohanaMeta_CommandDependencyDecorator_Object *) dp)->command_names;
    }
    else if (Py_TYPE(dp) == &RohanaMeta_Command_Type)
    {
        i = Py_SIZE(dp);
        it = ((RohanaMeta_Command_Object *) dp)->command_names;
    }
    else if (Py_TYPE(dp) == &RohanaMeta_BoundCommand_Type)
    {
        i = Py_SIZE(((RohanaMeta_BoundCommand_Object *) dp)->command);
        it = ((RohanaMeta_BoundCommand_Object *) dp)->command->command_names;
    }
    else
    {
        PyErr_Format(PyExc_TypeError, "can't extract dependencies from '%R'", Py_TYPE(dp));
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

static PyObject *RohanaInspect_UnwrapCommand(PyObject *Py_UNUSED(module), PyObject *cmd)
{
    if (Py_TYPE(cmd) == &RohanaMeta_Command_Type)
    {
        Py_INCREF(((RohanaMeta_Command_Object *) cmd)->wrapped_callable);
        return ((RohanaMeta_Command_Object *) cmd)->wrapped_callable;
    }
    else if (Py_TYPE(cmd) == &RohanaMeta_BoundCommand_Type)
    {
        Py_INCREF(((RohanaMeta_BoundCommand_Object *) cmd)->command->wrapped_callable);
        return ((RohanaMeta_BoundCommand_Object *) cmd)->command->wrapped_callable;
    }
    else
    {
        PyErr_Format(PyExc_TypeError, "can't extract only from commands");
        return NULL;
    }
}

static PyObject *RohanaInspect_GetCommandsList(PyObject *Py_UNUSED(module), RohanaMeta_GeneratorMeta_Object *gcls)
{
    PyObject *list;
    PyObject *tuple;
    if (Py_TYPE(gcls) == &RohanaMeta_GeneratorMeta_Type)
    {
        list = PyDict_Keys(gcls->commands_dict);
        if (list == NULL)
        {
            return NULL;
        }
        tuple = PyList_AsTuple(list);
        Py_DECREF(list);
        if (tuple == NULL)
        {
            return NULL;
        }
        return tuple;
    }
    else
    {
        PyErr_Format(PyExc_TypeError, "can't get commands list from '%s'", (PyType_Check(gcls) ? gcls->ht.ht_type.tp_name : Py_TYPE(gcls)->tp_name));
        return NULL;
    }
}

static PyMethodDef module_methods[] = {
    {"get_dependencies",  (PyCFunction) RohanaInspect_GetDependencies, METH_O},
    {"unwrap_command",    (PyCFunction) RohanaInspect_UnwrapCommand,   METH_O},
    {"get_commands_list", (PyCFunction) RohanaInspect_GetCommandsList, METH_O},
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

    if (PyType_Ready(&RohanaMeta_CommandDependencyDecorator_Type))
    {
        return NULL;
    }

    if (PyType_Ready(&RohanaMeta_BoundCommand_Type))
    {
        return NULL;
    }
    if (PyType_Ready(&RohanaMeta_Command_Type))
    {
        return NULL;
    }

    RohanaMeta_GeneratorMeta_Type.tp_base = &PyType_Type;
    if (PyType_Ready(&RohanaMeta_GeneratorMeta_Type))
    {
        return NULL;
    }
    if (PyType_Ready((struct _typeobject *) &RohanaMeta_Generator_Type))
    {
        return NULL;
    }

    if (module == NULL)
    {
        return NULL;
    }

    if (PyModule_AddObject(module, "command", (PyObject *) &RohanaMeta_Command_Type) != 0)
    {
        Py_DECREF(module);
        return NULL;
    }
    if (PyModule_AddObject(module, "rohana_generator_meta", (PyObject *) &RohanaMeta_GeneratorMeta_Type) != 0)
    {
        Py_DECREF(module);
        return NULL;
    }

    if (PyModule_AddObject(module, "rohana_generator", (PyObject *) &RohanaMeta_Generator_Type) != 0)
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}