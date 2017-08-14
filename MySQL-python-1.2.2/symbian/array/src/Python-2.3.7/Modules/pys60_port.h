/* Define macros for inline documentation. */
#define PyDoc_VAR(name) static char name[]
#define PyDoc_STRVAR(name,str) PyDoc_VAR(name) = PyDoc_STR(str)
#ifdef WITH_DOC_STRINGS
#define PyDoc_STR(str) str
#else
#define PyDoc_STR(str) ""
#endif

#define PyMODINIT_FUNC EXPORT_C void
#define PyAPI_FUNC(RTYPE) RTYPE

PyAPI_FUNC(PyObject *) PyObject_SelfIter(PyObject *);

PyObject *
PyObject_SelfIter(PyObject *obj)
{
	Py_INCREF(obj);
	return obj;
}


#undef PyCore_MALLOC_FUNC
#undef PyCore_REALLOC_FUNC
#undef PyCore_FREE_FUNC

#define PyCore_MALLOC_FUNC      malloc
#define PyCore_REALLOC_FUNC     realloc
#define PyCore_FREE_FUNC        free

int
PySlice_GetIndicesEx(PySliceObject *r, int length,
		     int *start, int *stop, int *step, int *slicelength)
{
	/* this is harder to get right than you might think */

	int defstart, defstop;

	if (r->step == Py_None) {
		*step = 1;
	} 
	else {
		if (!_PyEval_SliceIndex(r->step, step)) return -1;
		if (*step == 0) {
			PyErr_SetString(PyExc_ValueError,
					"slice step cannot be zero");
			return -1;
		}
	}

	defstart = *step < 0 ? length-1 : 0;
	defstop = *step < 0 ? -1 : length;

	if (r->start == Py_None) {
		*start = defstart;
	}
	else {
		if (!_PyEval_SliceIndex(r->start, start)) return -1;
		if (*start < 0) *start += length;
		if (*start < 0) *start = (*step < 0) ? -1 : 0;
		if (*start >= length) 
			*start = (*step < 0) ? length - 1 : length;
	}

	if (r->stop == Py_None) {
		*stop = defstop;
	}
	else {
		if (!_PyEval_SliceIndex(r->stop, stop)) return -1;
		if (*stop < 0) *stop += length;
		if (*stop < 0) *stop = -1;
		if (*stop > length) *stop = length;
	}

	if ((*step < 0 && *stop >= *start) 
	    || (*step > 0 && *start >= *stop)) {
		*slicelength = 0;
	}
	else if (*step < 0) {
		*slicelength = (*stop-*start+1)/(*step)+1;
	}
	else {
		*slicelength = (*stop-*start-1)/(*step)+1;
	}

	return 0;
}
