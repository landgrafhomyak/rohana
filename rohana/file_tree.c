#include <Python.h>

static PyObject *iter_self(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

typedef struct Rohana_FT_Builder_Cell
{
    PyObject *name;
    PyObject *value;
} Rohana_FT_Builder_Cell;

typedef struct Rohana_FT_Builder_Object
{
    PyObject_HEAD
    Py_ssize_t allocated;
    Py_ssize_t used;
    Rohana_FT_Builder_Cell *list;
    PyObject *stack;
} Rohana_FT_Builder_Object;


typedef struct Rohana_FT_FileTree_Path_Object
{
    PyObject_HEAD
    struct Rohana_FT_FileTree_Path_Object *parent;
    PyObject *name;
    PyObject *_cached_path;
} Rohana_FT_FileTree_Path_Object;

typedef struct Rohana_FT_FileTree_Cell
{
    Rohana_FT_FileTree_Path_Object key;
    PyObject *value;
} Rohana_FT_FileTree_Cell;

typedef struct Rohana_FT_FileTree_Object Rohana_FT_FileTree_Object;

typedef struct Rohana_FT_FileTree_View_Object
{
    PyObject_HEAD
} Rohana_FT_FileTree_PathsView_Object, Rohana_FT_FileTree_ValuesView_Object, Rohana_FT_FileTree_ItemsView_Object, Rohana_FT_FileTree_DirsView_Object;

typedef struct Rohana_FT_FileTree_Dir_View_Object
{
    PyObject_HEAD
} Rohana_FT_FileTree_Dir_PathsView_Object, Rohana_FT_FileTree_Dir_ValuesView_Object, Rohana_FT_FileTree_Dir_ItemsView_Object;


typedef struct Rohana_FT_FileTree_Dir_Object
{
    PyObject_VAR_HEAD
    Rohana_FT_FileTree_Dir_PathsView_Object paths_view;
    Rohana_FT_FileTree_Dir_ValuesView_Object values_view;
    Rohana_FT_FileTree_Dir_ItemsView_Object items_view;
    Rohana_FT_FileTree_Object *master;
    Rohana_FT_FileTree_Cell *root;
} Rohana_FT_FileTree_Dir_Object;

typedef struct Rohana_FT_FileTree_Dir_Iterator_Object
{
    PyObject_HEAD
    Rohana_FT_FileTree_Dir_Object *master;
    Rohana_FT_FileTree_Cell *pos;
} Rohana_FT_FileTree_Dir_PathsIterator_Object, Rohana_FT_FileTree_Dir_ValuesIterator_Object, Rohana_FT_FileTree_Dir_ItemsIterator_Object;

typedef struct Rohana_FT_FileTree_Object
{
    PyObject_VAR_HEAD
    Rohana_FT_FileTree_PathsView_Object paths_view;
    Rohana_FT_FileTree_ValuesView_Object values_view;
    Rohana_FT_FileTree_ItemsView_Object items_view;
    Rohana_FT_FileTree_DirsView_Object dirs_view;
    Py_ssize_t dirs_count;
    Rohana_FT_FileTree_Dir_Object *dirs;
    Rohana_FT_FileTree_Cell root;
    Rohana_FT_FileTree_Cell cells[1];
} Rohana_FT_FileTree_Object;

typedef struct Rohana_FT_FileTree_Iterator_Object
{
    PyObject_HEAD
    Rohana_FT_FileTree_Object *master;
    Py_ssize_t pos;
} Rohana_FT_FileTree_PathsIterator_Object, Rohana_FT_FileTree_ValuesIterator_Object, Rohana_FT_FileTree_ItemsIterator_Object, Rohana_FT_FileTree_DirsIterator_Object;

typedef struct Rohana_FT_FileTree_Item_Object
{
    PyObject_HEAD
    Rohana_FT_FileTree_Object *master;
    Rohana_FT_FileTree_Cell *data;
} Rohana_FT_FileTree_Item_Object;

typedef struct Rohana_FT_FileTree_Item_Iterator_Object
{
    PyObject_HEAD
    enum
    {
        Rohana_FT_FileTree_Item_Iterator_PATH,
        Rohana_FT_FileTree_Item_Iterator_VALUE,
        Rohana_FT_FileTree_Item_Iterator_ENDED
    } state;
    Rohana_FT_FileTree_Item_Object *data;
} Rohana_FT_FileTree_Item_Iterator_Object;

static Rohana_FT_Builder_Object *Rohana_FT_Builder_New(PyTypeObject *Py_UNUSED(cls), PyObject *args, PyObject *kwargs);

static PyObject *Rohana_FT_Builder_Sizeof(Rohana_FT_Builder_Object *self);

static PyObject *Rohana_FT_Builder_Enter(Rohana_FT_Builder_Object *self, PyObject *args);

static PyObject *Rohana_FT_Builder_Push(Rohana_FT_Builder_Object *self, PyObject *args);

static PyObject *Rohana_FT_Builder_Leave(Rohana_FT_Builder_Object *self);

static PyObject *Rohana_FT_Builder_Next(Rohana_FT_Builder_Object *self);

static int Rohana_FT_Builder_Bool(Rohana_FT_Builder_Object *self);

static void Rohana_FT_Builder_Dealloc(Rohana_FT_Builder_Object *self);

static PyMethodDef Rohana_FT_Builder_Methods[] = {
        {"__sizeof__", (PyCFunction) Rohana_FT_Builder_Sizeof, METH_NOARGS},
        {"enter",      (PyCFunction) Rohana_FT_Builder_Enter,  METH_VARARGS},
        {"leave",      (PyCFunction) Rohana_FT_Builder_Leave,  METH_NOARGS},
        {"push",       (PyCFunction) Rohana_FT_Builder_Push,   METH_VARARGS},
        {NULL}
};

static PyNumberMethods Rohana_FT_Builder_Number = {
        .nb_bool = (inquiry) Rohana_FT_Builder_Bool
};

static PyTypeObject Rohana_FT_Builder_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.builder",
        .tp_basicsize = sizeof(Rohana_FT_Builder_Object),
        .tp_new = (newfunc) Rohana_FT_Builder_New,
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_Builder_Next,
        .tp_as_number = &Rohana_FT_Builder_Number,
        .tp_dealloc = (destructor) Rohana_FT_Builder_Dealloc,
        .tp_methods = Rohana_FT_Builder_Methods
};

static Rohana_FT_FileTree_PathsView_Object *Rohana_FT_FileTree_GetPathView(Rohana_FT_FileTree_Object *self);

static Rohana_FT_FileTree_ValuesView_Object *Rohana_FT_FileTree_GetValueView(Rohana_FT_FileTree_Object *self);

static Rohana_FT_FileTree_ItemsView_Object *Rohana_FT_FileTree_GetItemsView(Rohana_FT_FileTree_Object *self);

static Rohana_FT_FileTree_DirsView_Object *Rohana_FT_FileTree_GetDirsView(Rohana_FT_FileTree_Object *self);

static Rohana_FT_FileTree_ItemsIterator_Object *Rohana_FT_FileTree_Iter(Rohana_FT_FileTree_Object *self);

static void Rohana_FT_FileTree_Dealloc(Rohana_FT_FileTree_Object *self);

static PyMethodDef Rohana_FT_FileTree_Methods[] = {
        {"paths",  (PyCFunction) Rohana_FT_FileTree_GetPathView,  METH_NOARGS},
        {"values", (PyCFunction) Rohana_FT_FileTree_GetValueView, METH_NOARGS},
        {"items",  (PyCFunction) Rohana_FT_FileTree_GetItemsView, METH_NOARGS},
        {"dirs",   (PyCFunction) Rohana_FT_FileTree_GetDirsView,  METH_NOARGS},
        {NULL}
};

static PyTypeObject Rohana_FT_FileTree_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Object) - sizeof(Rohana_FT_FileTree_Cell),
        .tp_itemsize = sizeof(Rohana_FT_FileTree_Cell),
        .tp_methods = Rohana_FT_FileTree_Methods,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dealloc,
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Iter
};

static PyObject *Rohana_FT_FileTree_Path_Str(Rohana_FT_FileTree_Path_Object *self);

static PyObject *Rohana_FT_FileTree_Path_Repr(Rohana_FT_FileTree_Path_Object *self);

static void Rohana_FT_FileTree_Path_Dealloc(Rohana_FT_FileTree_Path_Object *self);

static PyTypeObject Rohana_FT_FileTree_Path_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.path",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Object),
        .tp_str = (reprfunc) Rohana_FT_FileTree_Path_Str,
        .tp_repr = (reprfunc) Rohana_FT_FileTree_Path_Repr,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Path_Dealloc
};

static Rohana_FT_FileTree_PathsIterator_Object *Rohana_FT_FileTree_PathsView_Iter(Rohana_FT_FileTree_PathsView_Object *self);

static void Rohana_FT_FileTree_PathsView_Dealloc(Rohana_FT_FileTree_PathsView_Object *self);

static PyTypeObject Rohana_FT_FileTree_PathsView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.paths_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_PathsView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_PathsView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_PathsView_Dealloc
};

static Rohana_FT_FileTree_ValuesIterator_Object *Rohana_FT_FileTree_ValuesView_Iter(Rohana_FT_FileTree_ValuesView_Object *self);

static void Rohana_FT_FileTree_ValuesView_Dealloc(Rohana_FT_FileTree_ValuesView_Object *self);

static PyTypeObject Rohana_FT_FileTree_ValuesView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.values_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_ValuesView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_ValuesView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_ValuesView_Dealloc
};

static Rohana_FT_FileTree_ItemsIterator_Object *Rohana_FT_FileTree_ItemsView_Iter(Rohana_FT_FileTree_ItemsView_Object *self);

static void Rohana_FT_FileTree_ItemsView_Dealloc(Rohana_FT_FileTree_ItemsView_Object *self);

static PyTypeObject Rohana_FT_FileTree_ItemsView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.items_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_ItemsView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_ItemsView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_ItemsView_Dealloc
};


static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_PathsIterator_Next(Rohana_FT_FileTree_PathsIterator_Object *self);

static void Rohana_FT_FileTree_PathsIterator_Dealloc(Rohana_FT_FileTree_PathsIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_PathsIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.path_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_PathsIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_PathsIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_PathsIterator_Dealloc
};

static PyObject *Rohana_FT_FileTree_ValuesIterator_Next(Rohana_FT_FileTree_ValuesIterator_Object *self);

static void Rohana_FT_FileTree_ValuesIterator_Dealloc(Rohana_FT_FileTree_ValuesIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_ValuesIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.value_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_ValuesIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_ValuesIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_ValuesIterator_Dealloc
};

static Rohana_FT_FileTree_Item_Object *Rohana_FT_FileTree_ItemsIterator_Next(Rohana_FT_FileTree_ValuesIterator_Object *self);

static void Rohana_FT_FileTree_ItemsIterator_Dealloc(Rohana_FT_FileTree_ItemsIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_ItemsIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.items_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_ItemsIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_ItemsIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_ItemsIterator_Dealloc
};

static Rohana_FT_FileTree_Item_Iterator_Object *Rohana_FT_FileTree_Item_Iter(Rohana_FT_FileTree_Item_Object *self);

static void Rohana_FT_FileTree_Item_Dealloc(Rohana_FT_FileTree_Item_Object *self);

static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_Item_GetPath(Rohana_FT_FileTree_Item_Object *self);

static PyObject *Rohana_FT_FileTree_Item_GetValue(Rohana_FT_FileTree_Item_Object *self);

static int Rohana_FT_FileTree_Item_SetValue(Rohana_FT_FileTree_Item_Object *self, PyObject *value);

static PyGetSetDef Rohana_FT_FileTree_Item_GetSet[] = {
        {"path",  (getter) Rohana_FT_FileTree_Item_GetPath, NULL,},
        {"value", (getter) Rohana_FT_FileTree_Item_GetValue, (setter) Rohana_FT_FileTree_Item_SetValue},
        {NULL}
};

static PyTypeObject Rohana_FT_FileTree_Item_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.item",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Item_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Item_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Item_Dealloc,
        .tp_getset = Rohana_FT_FileTree_Item_GetSet
};

static PyObject *Rohana_FT_FileTree_Item_Iterator_Next(Rohana_FT_FileTree_Item_Iterator_Object *self);

static void Rohana_FT_FileTree_Item_Iterator_Dealloc(Rohana_FT_FileTree_Item_Iterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_Item_Iterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.item.iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Item_Iterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_Item_Iterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Item_Iterator_Dealloc
};

static Rohana_FT_FileTree_ItemsIterator_Object *Rohana_FT_FileTree_DirsView_Iter(Rohana_FT_FileTree_ItemsView_Object *self);

static void Rohana_FT_FileTree_DirsView_Dealloc(Rohana_FT_FileTree_DirsView_Object *self);

static PyTypeObject Rohana_FT_FileTree_DirsView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dirs_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_DirsView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_DirsView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_DirsView_Dealloc
};

static Rohana_FT_FileTree_Dir_Object *Rohana_FT_FileTree_DirsIterator_Next(Rohana_FT_FileTree_DirsIterator_Object *self);

static void Rohana_FT_FileTree_DirsIterator_Dealloc(Rohana_FT_FileTree_ItemsIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_DirsIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dirs_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_DirsView_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_DirsIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_DirsIterator_Dealloc
};

static Rohana_FT_FileTree_Dir_PathsView_Object *Rohana_FT_FileTree_Dir_GetPathView(Rohana_FT_FileTree_Dir_Object *self);

static Rohana_FT_FileTree_Dir_ValuesView_Object *Rohana_FT_FileTree_Dir_GetValueView(Rohana_FT_FileTree_Dir_Object *self);

static Rohana_FT_FileTree_Dir_ItemsView_Object *Rohana_FT_FileTree_Dir_GetItemsView(Rohana_FT_FileTree_Dir_Object *self);

static Rohana_FT_FileTree_Dir_ItemsIterator_Object *Rohana_FT_FileTree_Dir_Iter(Rohana_FT_FileTree_Dir_Object *self);

static void Rohana_FT_FileTree_Dir_Dealloc(Rohana_FT_FileTree_Dir_Object *self);

static PyMethodDef Rohana_FT_FileTree_Dir_Methods[] = {
        {"paths",  (PyCFunction) Rohana_FT_FileTree_Dir_GetPathView,  METH_NOARGS},
        {"values", (PyCFunction) Rohana_FT_FileTree_Dir_GetValueView, METH_NOARGS},
        {"items",  (PyCFunction) Rohana_FT_FileTree_Dir_GetItemsView, METH_NOARGS},
        {NULL}
};

static PyTypeObject Rohana_FT_FileTree_Dir_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Dir_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_Dealloc,
        .tp_methods = Rohana_FT_FileTree_Dir_Methods
};

static Rohana_FT_FileTree_Dir_PathsIterator_Object *Rohana_FT_FileTree_Dir_PathsView_Iter(Rohana_FT_FileTree_Dir_PathsView_Object *self);

static void Rohana_FT_FileTree_Dir_PathsView_Dealloc(Rohana_FT_FileTree_Dir_PathsView_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_PathsView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.paths_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_PathsView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Dir_PathsView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_PathsView_Dealloc
};

static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_Dir_PathsIterator_Next(Rohana_FT_FileTree_Dir_PathsIterator_Object *self);

static void Rohana_FT_FileTree_Dir_PathsIterator_Dealloc(Rohana_FT_FileTree_Dir_PathsIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_PathsIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.paths_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_PathsIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_Dir_PathsIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_PathsIterator_Dealloc
};

static Rohana_FT_FileTree_Dir_ValuesIterator_Object *Rohana_FT_FileTree_Dir_ValuesView_Iter(Rohana_FT_FileTree_Dir_ValuesView_Object *self);

static void Rohana_FT_FileTree_Dir_ValuesView_Dealloc(Rohana_FT_FileTree_Dir_ValuesView_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_ValuesView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.values_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_ValuesView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Dir_ValuesView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_ValuesView_Dealloc
};

static PyObject *Rohana_FT_FileTree_Dir_ValuesIterator_Next(Rohana_FT_FileTree_Dir_ValuesIterator_Object *self);

static void Rohana_FT_FileTree_Dir_ValuesIterator_Dealloc(Rohana_FT_FileTree_Dir_ValuesIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_ValuesIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.values_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_ValuesIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_Dir_ValuesIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_ValuesIterator_Dealloc
};

static Rohana_FT_FileTree_Dir_ItemsIterator_Object *Rohana_FT_FileTree_Dir_ItemsView_Iter(Rohana_FT_FileTree_Dir_ItemsView_Object *self);

static void Rohana_FT_FileTree_Dir_ItemsView_Dealloc(Rohana_FT_FileTree_Dir_ItemsView_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_ItemsView_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.items_view",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_ItemsView_Object),
        .tp_iter = (getiterfunc) Rohana_FT_FileTree_Dir_ItemsView_Iter,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_ItemsView_Dealloc
};

static Rohana_FT_FileTree_Item_Object *Rohana_FT_FileTree_Dir_ItemsIterator_Next(Rohana_FT_FileTree_Dir_ItemsIterator_Object *self);

static void Rohana_FT_FileTree_Dir_ItemsIterator_Dealloc(Rohana_FT_FileTree_Dir_ItemsIterator_Object *self);

static PyTypeObject Rohana_FT_FileTree_Dir_ItemsIterator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rohana.file_tree.file_tree.dir.items_iterator",
        .tp_basicsize = sizeof(Rohana_FT_FileTree_Dir_ItemsIterator_Object),
        .tp_iter = (getiterfunc) iter_self,
        .tp_iternext = (iternextfunc) Rohana_FT_FileTree_Dir_ItemsIterator_Next,
        .tp_dealloc = (destructor) Rohana_FT_FileTree_Dir_ItemsIterator_Dealloc
};

static Rohana_FT_Builder_Object *Rohana_FT_Builder_New(PyTypeObject *Py_UNUSED(cls), PyObject *args, PyObject *kwargs)
{
    static char *kw_list[] = {"", NULL};
    Rohana_FT_Builder_Object *self;
    Rohana_FT_Builder_Cell *list;
    PyObject *first_iterator;
    PyObject *stack;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kw_list, &first_iterator))
    {
        return NULL;
    }

    if (!PyIter_Check(first_iterator))
    {
        PyErr_Format(PyExc_TypeError, "directory view must be iterator");
        return NULL;
    }

    list = PyMem_Calloc(8, sizeof(Rohana_FT_Builder_Cell));
    if (list == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    stack = PyList_New(1);
    if (stack == NULL)
    {
        PyMem_Free(list);
        return NULL;
    }
    Py_INCREF(first_iterator);
    PyList_SET_ITEM(stack, 0, first_iterator);


    self = (Rohana_FT_Builder_Object *) Rohana_FT_Builder_Type.tp_alloc(&Rohana_FT_Builder_Type, 0);
    if (self == NULL)
    {
        PyMem_Free(list);
        Py_DECREF(stack);
        return NULL;
    }

    self->allocated = 8;
    self->used = 0;
    self->list = list;
    self->stack = stack;

    return self;
}

static PyObject *Rohana_FT_Builder_Sizeof(Rohana_FT_Builder_Object *self)
{
    return PyLong_FromSsize_t((Py_ssize_t) (sizeof(Rohana_FT_Builder_Object) + sizeof(Rohana_FT_Builder_Cell) * (self->allocated)));
}

static int Rohana_FT_Builder_Append(Rohana_FT_Builder_Object *self, PyObject *name, PyObject *value)
{
    Rohana_FT_Builder_Cell *new_list;

    if (self->used >= self->allocated)
    {
        new_list = PyMem_Calloc(self->allocated << 1, sizeof(Rohana_FT_Builder_Cell));
        if (new_list == NULL)
        {
            PyErr_NoMemory();
            return -1;
        }
        memcpy(new_list, self->list, sizeof(Rohana_FT_Builder_Cell) * self->used);
        PyMem_Free(self->list);
        self->list = new_list;
        self->allocated <<= 1;
    }

    Py_XINCREF(name);
    Py_XINCREF(value);
    self->list[self->used].name = name;
    self->list[self->used++].value = value;
    return 0;
}

static PyObject *Rohana_FT_Builder_Enter(Rohana_FT_Builder_Object *self, PyObject *args)
{
    PyObject *name;
    PyObject *directory_iterator;

    if (!PyArg_ParseTuple(args, "UO", &name, &directory_iterator))
    {
        return NULL;
    }

    if (PyList_GET_SIZE(self->stack) == 0)
    {
        PyErr_Format(PyExc_PermissionError, "builder's stack is empty, can't add another root");
        return NULL;
    }

    if (!PyIter_Check(directory_iterator))
    {
        PyErr_Format(PyExc_TypeError, "directory view must be iterator");
        return NULL;
    }
    if (Rohana_FT_Builder_Append(self, name, NULL) < 0)
    {
        return NULL;
    }
    if (PyList_Append(self->stack, directory_iterator) != 0)
    {
        return NULL;
    }
    Py_RETURN_NONE;
}


static PyObject *Rohana_FT_Builder_Push(Rohana_FT_Builder_Object *self, PyObject *args)
{
    PyObject *name;
    PyObject *file_object;

    if (!PyArg_ParseTuple(args, "UO", &name, &file_object))
    {
        return NULL;
    }

    if (PyList_GET_SIZE(self->stack) == 0)
    {
        PyErr_Format(PyExc_PermissionError, "builder's stack is empty, can't add files to root's level");
        return NULL;
    }

    if (Rohana_FT_Builder_Append(self, name, file_object) < 0)
    {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *Rohana_FT_Builder_Leave(Rohana_FT_Builder_Object *self)
{
    if (PyList_SetSlice(self->stack, PyList_GET_SIZE(self->stack) - 1, PyList_GET_SIZE(self->stack), NULL) != 0)
    {
        return NULL;
    }
    if (Rohana_FT_Builder_Append(self, NULL, NULL) < 0)
    {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *Rohana_FT_Builder_Next(Rohana_FT_Builder_Object *self)
{
    if (PyList_GET_SIZE(self->stack) < 1)
    {
        return NULL;
    }

    return PyIter_Next(PyList_GET_ITEM(self->stack, PyList_GET_SIZE(self->stack) - 1));
}

static int Rohana_FT_Builder_Bool(Rohana_FT_Builder_Object *self)
{
    return PyList_GET_SIZE(self->stack) > 0;
}

static void Rohana_FT_Builder_Dealloc(Rohana_FT_Builder_Object *self)
{


    Py_DECREF(self->stack);
    while (self->used-- > 0)
    {
        Py_XDECREF(self->list[self->used].name);
        Py_XDECREF(self->list[self->used].value);
    }
    PyMem_Free(self->list);
    Py_TYPE(self)->tp_free(self);

}

static Rohana_FT_FileTree_Object *Rohana_FT_Compile_Func(PyObject *Py_UNUSED(module), Rohana_FT_Builder_Object *builder)
{
    Py_ssize_t files_count = 0;
    Py_ssize_t dirs_count = 1;
    Py_ssize_t i;
    Py_ssize_t j;
    Py_ssize_t k;
    Rohana_FT_FileTree_Path_Object path_initializer = {PyObject_HEAD_INIT(&Rohana_FT_FileTree_Path_Type)};
    Rohana_FT_FileTree_Dir_Object dir_initializer = {
            PyObject_HEAD_INIT(&Rohana_FT_FileTree_Dir_Type)
            .paths_view = {PyObject_HEAD_INIT(&Rohana_FT_FileTree_Dir_PathsView_Type)},
            .values_view = {PyObject_HEAD_INIT(&Rohana_FT_FileTree_Dir_ValuesView_Type)},
            .items_view = {PyObject_HEAD_INIT(&Rohana_FT_FileTree_Dir_ItemsView_Type)},
    };
    Py_ssize_t depth;
    Rohana_FT_FileTree_Object *ft;
    Rohana_FT_FileTree_Dir_Object *dirs;
    PyObject *root_name;

    /* todo set refcnt */
    Py_REFCNT(&path_initializer) = 0;
    Py_REFCNT(&dir_initializer) = 0;
    Py_REFCNT(&(dir_initializer.paths_view)) = 0;
    Py_REFCNT(&(dir_initializer.values_view)) = 0;
    Py_REFCNT(&(dir_initializer.items_view)) = 0;

    if (Py_TYPE(builder) != &Rohana_FT_Builder_Type)
    {
        PyErr_Format(PyExc_TypeError, "can compile only from builder (got %R)", Py_TYPE(builder));
        return NULL;
    }

    for (i = 0; i < builder->used; i++)
    {
        if (builder->list[i].name != NULL)
        {
            files_count++;
            if (builder->list[i].value == NULL)
            {
                dirs_count++;
            }
        }
    }

    root_name = PyUnicode_FromString("/");
    if (root_name == NULL)
    {
        return NULL;
    }

    dirs = PyMem_Calloc(dirs_count, sizeof(Rohana_FT_FileTree_Dir_Object));
    if (dirs == NULL)
    {
        Py_DECREF(root_name);
        PyErr_NoMemory();
        return NULL;
    }
    ft = (Rohana_FT_FileTree_Object *) Rohana_FT_FileTree_Type.tp_alloc(&Rohana_FT_FileTree_Type, files_count);
    if (ft == NULL)
    {
        Py_DECREF(root_name);
        PyMem_Free(dirs);
        PyErr_NoMemory();
        return NULL;
    }

    /* todo set ref */
    ft->paths_view = (Rohana_FT_FileTree_PathsView_Object) {PyObject_HEAD_INIT(&Rohana_FT_FileTree_PathsView_Type)};
    Py_REFCNT(&(ft->paths_view)) = 0;
    ft->values_view = (Rohana_FT_FileTree_ValuesView_Object) {PyObject_HEAD_INIT(&Rohana_FT_FileTree_ValuesView_Type)};
    Py_REFCNT(&(ft->values_view)) = 0;
    ft->items_view = (Rohana_FT_FileTree_ItemsView_Object) {PyObject_HEAD_INIT(&Rohana_FT_FileTree_ItemsView_Type)};
    Py_REFCNT(&(ft->items_view)) = 0;
    ft->dirs_view = (Rohana_FT_FileTree_ItemsView_Object) {PyObject_HEAD_INIT(&Rohana_FT_FileTree_DirsView_Type)};
    Py_REFCNT(&(ft->dirs_view)) = 0;

    ft->dirs_count = dirs_count;
    ft->dirs = dirs;

    ft->root.key = path_initializer;
    ft->root.key.parent = NULL;
    ft->root.key.name = NULL;
    ft->root.key._cached_path = root_name;

    ft->dirs[0] = dir_initializer;
    ft->dirs[0].master = ft;
    ft->dirs[0].root = &(ft->root);

    i = 0;
    depth = 0;
    k = 1;
    for (j = 0; j < builder->used; j++)
    {
        if (builder->list[j].name == NULL)
        {
            depth++;
        }
        else
        {
            *(PyObject *) &(ft->cells[i]) = *(PyObject *) &path_initializer;
            ft->cells[i].key.parent = (struct Rohana_FT_FileTree_Path_Object *) &(ft->cells[i - 1]);
            while (depth-- > 0 && ft->cells[i].key.parent->parent != NULL)
            {
                ft->cells[i].key.parent = ft->cells[i].key.parent->parent;
            }
            Py_INCREF(builder->list[j].name);
            ft->cells[i].key.name = builder->list[j].name;
            ft->cells[i].key._cached_path = NULL;
            if (builder->list[j].value != NULL)
            {
                Py_INCREF(builder->list[j].value);
                ft->cells[i].value = builder->list[j].value;
                depth = 1;
            }
            else
            {
                ft->cells[i].value = NULL;
                depth = 0;
                dirs[k] = dir_initializer;
                dirs[k].master = ft;
                dirs[k].root = &(ft->cells[i]);

                k++;
            }

            i++;
        }
    }

    return ft;
}

static Rohana_FT_FileTree_PathsView_Object *Rohana_FT_FileTree_GetPathView(Rohana_FT_FileTree_Object *self)
{
    if (Py_REFCNT(&(self->paths_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->paths_view));
    return &(self->paths_view);
}

static Rohana_FT_FileTree_ValuesView_Object *Rohana_FT_FileTree_GetValueView(Rohana_FT_FileTree_Object *self)
{
    if (Py_REFCNT(&(self->values_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->values_view));
    return &(self->values_view);
}

static Rohana_FT_FileTree_ItemsView_Object *Rohana_FT_FileTree_GetItemsView(Rohana_FT_FileTree_Object *self)
{
    if (Py_REFCNT(&(self->items_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->items_view));
    return &(self->items_view);
}

static Rohana_FT_FileTree_DirsView_Object *Rohana_FT_FileTree_GetDirsView(Rohana_FT_FileTree_Object *self)
{
    if (Py_REFCNT(&(self->dirs_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->dirs_view));
    return &(self->dirs_view);
}

static Rohana_FT_FileTree_ItemsIterator_Object *Rohana_FT_FileTree_Iter(Rohana_FT_FileTree_Object *self)
{
    Rohana_FT_FileTree_ItemsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_ItemsIterator_Object *) Rohana_FT_FileTree_ItemsIterator_Type.tp_alloc(&Rohana_FT_FileTree_ItemsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    iterator->master = self;
    Py_INCREF(iterator->master);
    iterator->pos = 0;
    return iterator;
}

static void Rohana_FT_FileTree_Dealloc(Rohana_FT_FileTree_Object *self)
{


    Py_ssize_t i;

    PyMem_Free(self->dirs);
    Py_DECREF(self->root.key._cached_path);
    for (i = Py_SIZE(self) - 1; i > 0 /* first elem hasn't any refs */; i--)
    {
        Py_DECREF(self->cells[i].key.name);
        Py_XDECREF(self->cells[i].key._cached_path);
        Py_XDECREF(self->cells[i].value);
    }

    Py_TYPE(self)->tp_free(self);

}

static PyObject *Rohana_FT_FileTree_Path_Str(Rohana_FT_FileTree_Path_Object *self)
{
    Rohana_FT_FileTree_Path_Object *p = self;
    Py_ssize_t string_size = 0;
    Py_UCS4 max_char = '/';
    if (self->_cached_path == NULL)
    {
        do
        {
            string_size += PyUnicode_GetLength(p->name) + 1;
            max_char = max(max_char, PyUnicode_MAX_CHAR_VALUE(p));
        } while ((p = p->parent)->parent != NULL);
        self->_cached_path = PyUnicode_New(string_size, max_char);
        if (self->_cached_path == NULL)
        {
            return NULL;
        }

        p = self;
        do
        {
            if (
                    PyUnicode_CopyCharacters(
                            self->_cached_path,
                            string_size -= PyUnicode_GetLength(p->name),
                            p->name,
                            0,
                            PyUnicode_GetLength(p->name)
                    ) == -1
                    )
            {
                Py_DECREF(self->_cached_path);
                return NULL;
            }
            PyUnicode_WriteChar(self->_cached_path, --string_size, '/');
        } while ((p = p->parent)->parent != NULL);
    }
    Py_INCREF(self->_cached_path);
    return self->_cached_path;
}

PyObject *Rohana_FT_FileTree_Path_Repr(Rohana_FT_FileTree_Path_Object *self)
{
    PyObject *str_path;
    PyObject *repr;

    str_path = Rohana_FT_FileTree_Path_Str(self);
    if (str_path == NULL)
    {
        return NULL;
    }

    repr = PyUnicode_FromFormat("<rohana path object %A>", str_path);
    Py_DECREF(str_path);
    if (repr == NULL)
    {
        return NULL;
    }
    return repr;
}

#define Rohana_FT_FileTree_Path_GetFT(SELF) ((Rohana_FT_FileTree_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Object, root)))

static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_Path_NewRef(Rohana_FT_FileTree_Path_Object *ref)
{
    Rohana_FT_FileTree_Path_Object *pr = ref;
    while (pr->parent != NULL && Py_REFCNT(pr) <= 0)
    {
        Py_INCREF(pr);
        pr = pr->parent;
    }

    if (Py_REFCNT(pr) == 0)
    {
        Py_INCREF(pr);
        Py_INCREF(Rohana_FT_FileTree_Path_GetFT(pr));
    }
    return ref;
}

static void Rohana_FT_FileTree_Path_Dealloc(Rohana_FT_FileTree_Path_Object *self)
{


    if (self->parent == NULL)
    {

        Py_DECREF(Rohana_FT_FileTree_Path_GetFT(self));
    }
    else
    {


        Py_DECREF(self->parent);
    }

}

#define Rohana_FT_FileTree_PathsView_GetMaster(SELF) ((Rohana_FT_FileTree_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Object, paths_view)))

static Rohana_FT_FileTree_PathsIterator_Object *Rohana_FT_FileTree_PathsView_Iter(Rohana_FT_FileTree_PathsView_Object *self)
{
    Rohana_FT_FileTree_PathsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_PathsIterator_Object *) Rohana_FT_FileTree_PathsIterator_Type.tp_alloc(&Rohana_FT_FileTree_PathsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    iterator->master = Rohana_FT_FileTree_PathsView_GetMaster(self);
    Py_INCREF(iterator->master);
    iterator->pos = 0;
    return iterator;
}

static void Rohana_FT_FileTree_PathsView_Dealloc(Rohana_FT_FileTree_PathsView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_PathsView_GetMaster(self));

}

#define Rohana_FT_FileTree_ValuesView_GetMaster(SELF) ((Rohana_FT_FileTree_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Object, values_view)))


static Rohana_FT_FileTree_ValuesIterator_Object *Rohana_FT_FileTree_ValuesView_Iter(Rohana_FT_FileTree_ValuesView_Object *self)
{
    Rohana_FT_FileTree_ValuesIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_ValuesIterator_Object *) Rohana_FT_FileTree_ValuesIterator_Type.tp_alloc(&Rohana_FT_FileTree_ValuesIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    iterator->master = Rohana_FT_FileTree_ValuesView_GetMaster(self);
    Py_INCREF(iterator->master);
    iterator->pos = 0;
    return iterator;
}


static void Rohana_FT_FileTree_ValuesView_Dealloc(Rohana_FT_FileTree_ValuesView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_ValuesView_GetMaster(self));

}

#define Rohana_FT_FileTree_ItemsView_GetMaster(SELF) ((Rohana_FT_FileTree_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Object, items_view)))

static Rohana_FT_FileTree_ItemsIterator_Object *Rohana_FT_FileTree_ItemsView_Iter(Rohana_FT_FileTree_ItemsView_Object *self)
{
    Rohana_FT_FileTree_ItemsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_ItemsIterator_Object *) Rohana_FT_FileTree_ItemsIterator_Type.tp_alloc(&Rohana_FT_FileTree_ItemsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    iterator->master = Rohana_FT_FileTree_ItemsView_GetMaster(self);
    Py_INCREF(iterator->master);
    iterator->pos = 0;
    return iterator;
}

static void Rohana_FT_FileTree_ItemsView_Dealloc(Rohana_FT_FileTree_ItemsView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_ItemsView_GetMaster(self));

}


static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_PathsIterator_Next(Rohana_FT_FileTree_PathsIterator_Object *self)
{
    while (self->pos < Py_SIZE(self->master) && self->master->cells[self->pos].value == NULL)
    {
        self->pos++;
    }

    if (self->pos >= Py_SIZE(self->master))
    {
        return NULL;
    }
    return Rohana_FT_FileTree_Path_NewRef(&(self->master->cells[self->pos++].key));
}

static void Rohana_FT_FileTree_PathsIterator_Dealloc(Rohana_FT_FileTree_PathsIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

static PyObject *Rohana_FT_FileTree_ValuesIterator_Next(Rohana_FT_FileTree_ValuesIterator_Object *self)
{
    while (self->pos < Py_SIZE(self->master) && self->master->cells[self->pos].value == NULL)
    {
        self->pos++;
    }

    if (self->pos >= Py_SIZE(self->master))
    {
        return NULL;
    }

    Py_INCREF(self->master->cells[self->pos].value);
    return self->master->cells[self->pos++].value;
}


static void Rohana_FT_FileTree_ValuesIterator_Dealloc(Rohana_FT_FileTree_ValuesIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

static Rohana_FT_FileTree_Item_Object *Rohana_FT_FileTree_Item_Create(Rohana_FT_FileTree_Object *master, Rohana_FT_FileTree_Cell *data)
{
    Rohana_FT_FileTree_Item_Object *item;
    item = (Rohana_FT_FileTree_Item_Object *) Rohana_FT_FileTree_Item_Type.tp_alloc(&Rohana_FT_FileTree_Item_Type, 0);
    if (item == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    item->master = master;
    Py_INCREF(master);
    item->data = data;
    return item;
}

static Rohana_FT_FileTree_Item_Object *Rohana_FT_FileTree_ItemsIterator_Next(Rohana_FT_FileTree_ValuesIterator_Object *self)
{
    while (self->pos < Py_SIZE(self->master) && self->master->cells[self->pos].value == NULL)
    {
        self->pos++;
    }

    if (self->pos >= Py_SIZE(self->master))
    {
        return NULL;
    }

    return Rohana_FT_FileTree_Item_Create(self->master, &(self->master->cells[self->pos++]));
}

static void Rohana_FT_FileTree_ItemsIterator_Dealloc(Rohana_FT_FileTree_ItemsIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}


static Rohana_FT_FileTree_Item_Iterator_Object *Rohana_FT_FileTree_Item_Iter(Rohana_FT_FileTree_Item_Object *self)
{
    Rohana_FT_FileTree_Item_Iterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_Item_Iterator_Object *) Rohana_FT_FileTree_Item_Iterator_Type.tp_alloc(&Rohana_FT_FileTree_Item_Iterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    iterator->state = Rohana_FT_FileTree_Item_Iterator_PATH;
    Py_INCREF(self);
    iterator->data = self;

    return iterator;
}

static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_Item_GetPath(Rohana_FT_FileTree_Item_Object *self)
{
    return Rohana_FT_FileTree_Path_NewRef(&(self->data->key));
}

static PyObject *Rohana_FT_FileTree_Item_GetValue(Rohana_FT_FileTree_Item_Object *self)
{
    Py_INCREF(self->data->value);
    return self->data->value;
}

static int Rohana_FT_FileTree_Item_SetValue(Rohana_FT_FileTree_Item_Object *self, PyObject *value)
{
    if (value == NULL)
    {
        PyErr_Format(PyExc_TypeError, "can't delete files");
        return -1;
    }

    Py_DECREF(self->data->value);
    self->data->value = value;
    Py_INCREF(value);
    return 0;
}

static void Rohana_FT_FileTree_Item_Dealloc(Rohana_FT_FileTree_Item_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

static PyObject *Rohana_FT_FileTree_Item_Iterator_Next(Rohana_FT_FileTree_Item_Iterator_Object *self)
{
    switch (self->state)
    {
        case Rohana_FT_FileTree_Item_Iterator_PATH:
            self->state = Rohana_FT_FileTree_Item_Iterator_VALUE;
            return (PyObject *) Rohana_FT_FileTree_Path_NewRef(&(self->data->data->key));
        case Rohana_FT_FileTree_Item_Iterator_VALUE:
            self->state = Rohana_FT_FileTree_Item_Iterator_ENDED;
            Py_INCREF(self->data->data->value);
            return self->data->data->value;
        case Rohana_FT_FileTree_Item_Iterator_ENDED:
            return NULL;
    }
}


static void Rohana_FT_FileTree_Item_Iterator_Dealloc(Rohana_FT_FileTree_Item_Iterator_Object *self)
{


    Py_DECREF(self->data);
    Py_TYPE(self)->tp_free(self);

}

#define Rohana_FT_FileTree_DirsView_GetMaster(SELF) ((Rohana_FT_FileTree_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Object, dirs_view)))

static Rohana_FT_FileTree_DirsIterator_Object *Rohana_FT_FileTree_DirsView_Iter(Rohana_FT_FileTree_ItemsView_Object *self)
{
    Rohana_FT_FileTree_DirsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_ItemsIterator_Object *) Rohana_FT_FileTree_DirsIterator_Type.tp_alloc(&Rohana_FT_FileTree_DirsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    iterator->master = Rohana_FT_FileTree_DirsView_GetMaster(self);
    Py_INCREF(iterator->master);
    iterator->pos = 0;
    return iterator;
}

static void Rohana_FT_FileTree_DirsView_Dealloc(Rohana_FT_FileTree_DirsView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_DirsView_GetMaster(self));

}

static Rohana_FT_FileTree_Dir_Object *Rohana_FT_FileTree_Dir_NewRef(Rohana_FT_FileTree_Dir_Object *o)
{
    if (Py_REFCNT(o) == 0)
    {
        Py_INCREF(o->master);
    }
    Py_INCREF(o);
    return o;
}

static Rohana_FT_FileTree_Dir_Object *Rohana_FT_FileTree_DirsIterator_Next(Rohana_FT_FileTree_DirsIterator_Object *self)
{

    if (self->pos >= self->master->dirs_count)
    {
        return NULL;
    }


    return Rohana_FT_FileTree_Dir_NewRef(&(self->master->dirs[self->pos++]));

}

static void Rohana_FT_FileTree_DirsIterator_Dealloc(Rohana_FT_FileTree_ItemsIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

static Rohana_FT_FileTree_Dir_PathsView_Object *Rohana_FT_FileTree_Dir_GetPathView(Rohana_FT_FileTree_Dir_Object *self)
{
    if (Py_REFCNT(&(self->paths_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->paths_view));
    return &(self->paths_view);
}

static Rohana_FT_FileTree_Dir_ValuesView_Object *Rohana_FT_FileTree_Dir_GetValueView(Rohana_FT_FileTree_Dir_Object *self)
{
    if (Py_REFCNT(&(self->values_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->values_view));
    return &(self->values_view);
}

static Rohana_FT_FileTree_Dir_ItemsView_Object *Rohana_FT_FileTree_Dir_GetItemsView(Rohana_FT_FileTree_Dir_Object *self)
{
    if (Py_REFCNT(&(self->items_view)) == 0)
    {
        Py_INCREF(self);
    }
    Py_INCREF(&(self->items_view));
    return &(self->items_view);
}

static Rohana_FT_FileTree_Dir_ItemsIterator_Object *Rohana_FT_FileTree_Dir_Iter(Rohana_FT_FileTree_Dir_Object *self)
{
    Rohana_FT_FileTree_Dir_ItemsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_Dir_ItemsIterator_Object *) Rohana_FT_FileTree_Dir_ItemsIterator_Type.tp_alloc(&Rohana_FT_FileTree_Dir_ItemsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    iterator->master = Rohana_FT_FileTree_Dir_NewRef(self);
    iterator->pos = iterator->master->root + 1;

    return iterator;
}

static void Rohana_FT_FileTree_Dir_Dealloc(Rohana_FT_FileTree_Dir_Object *self)
{


    Py_DECREF(self->master);

}

#define Rohana_FT_FileTree_Dir_PathsView_GetMaster(SELF) ((Rohana_FT_FileTree_Dir_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Dir_Object, paths_view)))

static Rohana_FT_FileTree_Dir_PathsIterator_Object *Rohana_FT_FileTree_Dir_PathsView_Iter(Rohana_FT_FileTree_Dir_PathsView_Object *self)
{
    Rohana_FT_FileTree_Dir_PathsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_Dir_PathsIterator_Object *) Rohana_FT_FileTree_Dir_PathsIterator_Type.tp_alloc(&Rohana_FT_FileTree_Dir_PathsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    iterator->master = Rohana_FT_FileTree_Dir_NewRef(Rohana_FT_FileTree_Dir_PathsView_GetMaster(self));
    iterator->pos = iterator->master->root + 1;

    return iterator;
}

static void Rohana_FT_FileTree_Dir_PathsView_Dealloc(Rohana_FT_FileTree_Dir_PathsView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_Dir_PathsView_GetMaster(self));

}

#define Rohana_FT_FileTree_CheckCellPtr(SELF, PTR) (((uintptr_t) (PTR)) < ((uintptr_t) &(((Rohana_FT_FileTree_Object *)(SELF))->cells[Py_SIZE((SELF))])))

static Rohana_FT_FileTree_Path_Object *Rohana_FT_FileTree_Dir_PathsIterator_Next(Rohana_FT_FileTree_Dir_PathsIterator_Object *self)
{

    while (Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos) && ((self->pos->key.parent != &(self->master->root->key)) || (self->pos->value == NULL)))
    {
        self->pos++;
    }

    if (!Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos))
    {
        return NULL;
    }
    return Rohana_FT_FileTree_Path_NewRef(&((self->pos++)->key));
}

static void Rohana_FT_FileTree_Dir_PathsIterator_Dealloc(Rohana_FT_FileTree_Dir_PathsIterator_Object *self)
{

    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

#define Rohana_FT_FileTree_Dir_ValuesView_GetMaster(SELF) ((Rohana_FT_FileTree_Dir_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Dir_Object, values_view)))

static Rohana_FT_FileTree_Dir_ValuesIterator_Object *Rohana_FT_FileTree_Dir_ValuesView_Iter(Rohana_FT_FileTree_Dir_ValuesView_Object *self)
{
    Rohana_FT_FileTree_Dir_ValuesIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_Dir_ValuesIterator_Object *) Rohana_FT_FileTree_Dir_ValuesIterator_Type.tp_alloc(&Rohana_FT_FileTree_Dir_ValuesIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    iterator->master = Rohana_FT_FileTree_Dir_NewRef(Rohana_FT_FileTree_Dir_ValuesView_GetMaster(self));
    iterator->pos = iterator->master->root + 1;

    return iterator;
}

static void Rohana_FT_FileTree_Dir_ValuesView_Dealloc(Rohana_FT_FileTree_Dir_ValuesView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_Dir_ValuesView_GetMaster(self));

}

#define Rohana_FT_FileTree_CheckCellPtr(SELF, PTR) (((uintptr_t) (PTR)) < ((uintptr_t) &(((Rohana_FT_FileTree_Object *)(SELF))->cells[Py_SIZE((SELF))])))

static PyObject *Rohana_FT_FileTree_Dir_ValuesIterator_Next(Rohana_FT_FileTree_Dir_ValuesIterator_Object *self)
{
    while (Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos) && (self->pos->key.parent != &(self->master->root->key) || self->pos->value == NULL))
    {
        self->pos++;
    }

    if (!Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos))
    {
        return NULL;
    }

    Py_INCREF((self->pos)->value);
    return (self->pos++)->value;
}

static void Rohana_FT_FileTree_Dir_ValuesIterator_Dealloc(Rohana_FT_FileTree_Dir_ValuesIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

#define Rohana_FT_FileTree_Dir_ItemsView_GetMaster(SELF) ((Rohana_FT_FileTree_Dir_Object *) (((uintptr_t) (SELF)) - offsetof(Rohana_FT_FileTree_Dir_Object, items_view)))

static Rohana_FT_FileTree_Dir_ItemsIterator_Object *Rohana_FT_FileTree_Dir_ItemsView_Iter(Rohana_FT_FileTree_Dir_ItemsView_Object *self)
{
    Rohana_FT_FileTree_Dir_ItemsIterator_Object *iterator;

    iterator = (Rohana_FT_FileTree_Dir_ItemsIterator_Object *) Rohana_FT_FileTree_Dir_ItemsIterator_Type.tp_alloc(&Rohana_FT_FileTree_Dir_ItemsIterator_Type, 0);
    if (iterator == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }

    iterator->master = Rohana_FT_FileTree_Dir_NewRef(Rohana_FT_FileTree_Dir_ItemsView_GetMaster(self));
    iterator->pos = iterator->master->root + 1;

    return iterator;
}

static void Rohana_FT_FileTree_Dir_ItemsView_Dealloc(Rohana_FT_FileTree_Dir_ItemsView_Object *self)
{


    Py_DECREF(Rohana_FT_FileTree_Dir_ItemsView_GetMaster(self));

}

#define Rohana_FT_FileTree_CheckCellPtr(SELF, PTR) (((uintptr_t) (PTR)) < ((uintptr_t) &(((Rohana_FT_FileTree_Object *)(SELF))->cells[Py_SIZE((SELF))])))

static Rohana_FT_FileTree_Item_Object *Rohana_FT_FileTree_Dir_ItemsIterator_Next(Rohana_FT_FileTree_Dir_ItemsIterator_Object *self)
{
    while (Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos) && (self->pos->key.parent != &(self->master->root->key) || self->pos->value == NULL))
    {
        self->pos++;
    }

    if (!Rohana_FT_FileTree_CheckCellPtr(self->master->master, self->pos))
    {
        return NULL;
    }

    return Rohana_FT_FileTree_Item_Create(self->master->master, self->pos++);
}

static void Rohana_FT_FileTree_Dir_ItemsIterator_Dealloc(Rohana_FT_FileTree_Dir_ItemsIterator_Object *self)
{


    Py_DECREF(self->master);
    Py_TYPE(self)->tp_free(self);

}

static PyMethodDef module_functions[] = {
        {"compile", (PyCFunction) Rohana_FT_Compile_Func, METH_O},
        {NULL}
};

static PyModuleDef module_def = {
        .m_name = "rohana.file_tree",
        .m_methods = module_functions
};

PyMODINIT_FUNC PyInit_file_tree(void)
{
    PyObject *module = NULL;

    Rohana_FT_FileTree_Path_Type.tp_dict = NULL;

    if (PyType_Ready(&Rohana_FT_Builder_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Path_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Path_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Path_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_PathsView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_PathsView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_PathsView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_ValuesView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_ValuesView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_ValuesView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_ItemsView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_ItemsView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_ItemsView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_PathsIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_PathsIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_PathsIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_ValuesIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_ValuesIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_ValuesIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_ItemsIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_ItemsIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_ItemsIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Item_Iterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Item_Iterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Item_Iterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Item_Type.tp_dict = Py_BuildValue(
            "{sssO}",
            "__module__",
            module_def.m_name,
            "iterator",
            &Rohana_FT_FileTree_Item_Iterator_Type
    );
    if (Rohana_FT_FileTree_Item_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Item_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_DirsView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_DirsView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_DirsView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_DirsIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_DirsIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_DirsIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_PathsView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_PathsView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_PathsView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_PathsIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_PathsIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_PathsIterator_Type))
    {
        goto err;
    }
    Rohana_FT_FileTree_Dir_ValuesView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_ValuesView_Type.tp_dict == NULL)
    {
        goto err;
    }

    if (PyType_Ready(&Rohana_FT_FileTree_Dir_ValuesView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_ValuesIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_ValuesIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_ValuesIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_ItemsView_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_ItemsView_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_ItemsView_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_ItemsIterator_Type.tp_dict = Py_BuildValue("{ss}", "__module__", module_def.m_name);
    if (Rohana_FT_FileTree_Dir_ItemsIterator_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_ItemsIterator_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Dir_Type.tp_dict = Py_BuildValue(
            "{sssOsOsOsOsOsO}",
            "__module__",
            module_def.m_name,
            "paths_view",
            &Rohana_FT_FileTree_Dir_PathsView_Type,
            "paths_iterator",
            &Rohana_FT_FileTree_Dir_PathsIterator_Type,
            "values_view",
            &Rohana_FT_FileTree_Dir_ValuesView_Type,
            "values_iterator",
            &Rohana_FT_FileTree_Dir_ValuesIterator_Type,
            "items_view",
            &Rohana_FT_FileTree_Dir_ItemsView_Type,
            "items_iterator",
            &Rohana_FT_FileTree_Dir_ItemsIterator_Type
    );
    if (Rohana_FT_FileTree_Dir_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Dir_Type))
    {
        goto err;
    }

    Rohana_FT_FileTree_Type.tp_dict = Py_BuildValue(
            "{sOsOsOsOsOsOsOsOsOsOsO}",
            "path",
            &Rohana_FT_FileTree_Path_Type,
            "paths_view",
            &Rohana_FT_FileTree_PathsView_Type,
            "values_view",
            &Rohana_FT_FileTree_ValuesView_Type,
            "items_view",
            &Rohana_FT_FileTree_ItemsView_Type,
            "paths_iterator",
            &Rohana_FT_FileTree_PathsIterator_Type,
            "values_iterator",
            &Rohana_FT_FileTree_ValuesIterator_Type,
            "items_iterator",
            &Rohana_FT_FileTree_ItemsIterator_Type,
            "item",
            &Rohana_FT_FileTree_Item_Type,
            "dirs_view",
            &Rohana_FT_FileTree_DirsView_Type,
            "dirs_iterator",
            &Rohana_FT_FileTree_DirsIterator_Type,
            "dir",
            &Rohana_FT_FileTree_Dir_Type
    );
    if (Rohana_FT_FileTree_Type.tp_dict == NULL)
    {
        goto err;
    }
    if (PyType_Ready(&Rohana_FT_FileTree_Type))
    {
        goto err;
    }


    module = PyModule_Create(&module_def);
    if (module == NULL)
    {
        goto err;
    }

    if (PyModule_AddObject(module, "builder", (PyObject *) &Rohana_FT_Builder_Type))
    {
        goto err;
    }
    if (PyModule_AddObject(module, "file_tree", (PyObject *) &Rohana_FT_FileTree_Type))
    {
        goto err;
    }

    return module;

    err:
    Py_XDECREF(module);
    Py_XDECREF(Rohana_FT_FileTree_Path_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_PathsView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_ValuesView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_ItemsView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_PathsIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_ValuesIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_ItemsIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Item_Iterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Item_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_DirsView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_DirsIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_PathsView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_PathsIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_ValuesView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_ValuesIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_ItemsView_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_ItemsIterator_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Dir_Type.tp_dict);
    Py_XDECREF(Rohana_FT_FileTree_Type.tp_dict);
    return NULL;
}
