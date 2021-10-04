#include <Python.h>

static PyObject *_install_dependencies(PyObject *dependencies, PyObject *logger)
{
    PyThreadState *prev_interpreter;
    PyThreadState *sub_interpreter;
    PyObject *pip_module;
    PyObject *pip_main_func;
    Py_ssize_t i;
    PyObject *args_list;
    PyObject *exit_code_o;
    long long exit_code;

    dependencies = PyObject_CallFunction((PyObject *)&PyTuple_Type, "O", dependencies);
    if (dependencies == NULL)
    {
        return NULL;
    }
    if (!PyTuple_CheckExact(dependencies))
    {
        PyErr_Format(
            PyExc_TypeError,
            "Tuple constructor returned not a tuple (%R)",
            Py_TYPE(dependencies)
        );
        Py_DECREF(dependencies);
        return NULL;
    }
    args_list = PyList_New(PyTuple_GET_SIZE(dependencies) + 1);
    if (args_list == NULL)
    {
        Py_DECREF(dependencies);
        return NULL;
    }

    PyList_SET_ITEM(args_list, 0, PyUnicode_FromString("install"));
    if (PyList_GET_ITEM(dependencies, 0) == NULL)
    {
        Py_DECREF(dependencies);
        Py_DECREF(args_list);
        return NULL;
    }

    for (i = PyTuple_GET_SIZE(dependencies) - 1; i >= 0; i--)
    {
        if (!PyUnicode_CheckExact(PyTuple_GET_ITEM(dependencies, i)))
        {
            PyErr_Format(
                PyExc_TypeError,
                "Dependencies list must contain only string, but element %zd is not (%R)",
                i,
                Py_TYPE(PyTuple_GET_ITEM(dependencies, i))
            );
            Py_DECREF(dependencies);
            Py_DECREF(args_list);
            return NULL;
        }
        Py_INCREF(PyTuple_GET_ITEM(dependencies, i));
        PyList_SET_ITEM(args_list, i + 1, PyTuple_GET_ITEM(dependencies, i));
    }
    Py_DECREF(dependencies);

    prev_interpreter = PyThreadState_Get();
    if (prev_interpreter == NULL)
    {
        Py_DECREF(args_list);
        return NULL;
    }

    sub_interpreter = Py_NewInterpreter();
    if (sub_interpreter == NULL)
    {
        Py_EndInterpreter(sub_interpreter);
        PyThreadState_Swap(prev_interpreter);
        Py_DECREF(args_list);
        PyErr_Format(PyExc_RuntimeError, "Can't create safe environment (sub-interpreter) for using pip");
        return NULL;
    }

    pip_module = PyImport_ImportModule("pip");
    if (pip_module == NULL)
    {
        Py_EndInterpreter(sub_interpreter);
        PyThreadState_Swap(prev_interpreter);
        Py_DECREF(args_list);
        PyErr_Format(PyExc_ImportError, "Can't import pip");
        return NULL;
    }

    pip_main_func = PyObject_GetAttrString(pip_module, "main");
    Py_DECREF(pip_module);
    if (pip_main_func == NULL)
    {
        Py_EndInterpreter(sub_interpreter);
        PyThreadState_Swap(prev_interpreter);
        Py_DECREF(args_list);
        PyErr_Format(PyExc_NameError, "Can't get pip's main func");
        return NULL;
    }

    exit_code_o = PyObject_CallFunction(pip_main_func, "O", args_list);
    PyThreadState_Swap(prev_interpreter);
    Py_DECREF(args_list);
    PyThreadState_Swap(sub_interpreter);
    if (exit_code_o == NULL)
    {
        Py_EndInterpreter(sub_interpreter);
        PyThreadState_Swap(prev_interpreter);
        PyErr_Format(PyExc_RuntimeError, "Unexpected error in pip script");
        return NULL;
    }

    exit_code = PyLong_AsLongLong(exit_code_o);
    Py_DECREF(exit_code_o);
    if (exit_code == -1 && PyErr_Occurred())
    {
        Py_EndInterpreter(sub_interpreter);
        PyThreadState_Swap(prev_interpreter);
        PyErr_Format(PyExc_RuntimeError, "Can't get pip's exit code (pip script failed)");
        return NULL;
    }
    Py_EndInterpreter(sub_interpreter);
    PyThreadState_Swap(prev_interpreter);

    if (exit_code != 0)
    {
        PyErr_Format(
            PyExc_RuntimeError,
            "Pip script failed with exit code %lld",
            exit_code
         );
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *_install_dependencies_func(PyObject *module, PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"dependencies", "logger", NULL};
    PyObject *dependencies;
    PyObject *logger;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kw_list, &dependencies, &logger))
    {
        return NULL;
    }


    return _install_dependencies(dependencies, logger);
}

static PyMethodDef module_methods[] = {
    {"_install_dependencies", (PyCFunction)_install_dependencies_func, METH_VARARGS | METH_KEYWORDS},
//    {"install_dependencies", (PyCFunction)NULL, METH_O},
    {NULL}
};

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "rohana.commands.install_dependencies",
    .m_methods = module_methods
};

PyMODINIT_FUNC PyInit_install_dependencies(void)
{
    return PyModule_Create(&module_def);
}