#include <Python.h>

#include <config.h>

#include <mucharmap/mucharmap.h>

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>

#include <pygtk/pygtk.h>

void pymucharmap_register_classes(PyObject* d);
void pymucharmap_add_constants(PyObject* module, const gchar* strip_prefix);

extern PyMethodDef pymucharmap_functions[];

DL_EXPORT(void) initmucharmap(void);

DL_EXPORT(void) initmucharmap(void)
{
	PyObject* m;
	PyObject* d;

	init_pygobject();
	init_pygtk();

	m = Py_InitModule("mucharmap", pymucharmap_functions);
	d = PyModule_GetDict(m);

	pymucharmap_register_classes(d);
	pymucharmap_add_constants(m, "GUCHARMAP_");
}
