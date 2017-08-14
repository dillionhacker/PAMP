/* Copyright 2006 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "mod_python.h"


/*
 * See mod_python.h for the reason for this.
 */ 

#undef PyObject_GenericGetAttr
#undef PyObject_GenericSetAttr

PyObject* _PyObject_GenericGetAttr(PyObject* a1, PyObject* a2)
{
    return PyObject_GenericGetAttr(a1, a2);
}

int _PyObject_GenericSetAttr(PyObject* a1, PyObject* a2, PyObject* a3)
{
    return PyObject_GenericSetAttr(a1, a2, a3);
}
