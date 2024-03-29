#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "symnmf.h"

static PyObject* symnmf(PyObject *self, PyObject *args);
static PyObject* sym(PyObject *self, PyObject *args);
static PyObject* ddg(PyObject *self, PyObject *args);
static PyObject* norm(PyObject *self, PyObject *args);
double** convert_to_matrix(PyObject* inputList, int rows, int cols);
PyObject* convert_to_PyObject(double** matrix, int rows, int cols);


PyObject* symnmf_ops(PyObject *self, PyObject *args, int command){ /*command == 0 means sym, command == 1 means ddg, command == 2 means norm */ 
    int num_of_coordinates;
    int N;
    double** datapoints_arr;
    PyObject* inputList;
    PyObject* result;
    double** matrix;
    if (!PyArg_ParseTuple(args, "O", &inputList)) {
        return NULL;  /* Parsing failed, return an error */
    }
    /* Get dimensions */
    N =  (int)PySequence_Size(inputList);
    num_of_coordinates = (int)PySequence_Size(PySequence_GetItem(inputList, 0));
    datapoints_arr = convert_to_matrix(inputList, N, num_of_coordinates);
    if (!datapoints_arr){ 
        return NULL; /* Conversion falied*/
    }
    switch(command){
        case 0:
            matrix = sym_c(datapoints_arr, num_of_coordinates, N);
            break;
        case 1:
            matrix = ddg_c(datapoints_arr, num_of_coordinates, N);
            break;
        case 2:
            matrix = norm_c(datapoints_arr, num_of_coordinates, N);
            break;
        default:
            return NULL;
    }
    /* Convert to PyObject, free memory and return */
    result = convert_to_PyObject(matrix, N, N);
    free_matrix(datapoints_arr, N);
    free_matrix(matrix, N);
    return result;
}




static PyObject* symnmf(PyObject *self, PyObject *args){
    int k, N;
    PyObject *inputList1, *inputList2, *result;

    double **symnmf_matrix, **H, **W;
    if (!PyArg_ParseTuple(args, "OOii", &inputList1, &inputList2, &N, &k)) {
        return NULL;  /* Parsing failed, return an error */
    }
    H = convert_to_matrix(inputList1, N, k);
    W = convert_to_matrix(inputList2, N, N);
    symnmf_matrix = symnmf_c(H, W, N, k);
    result = convert_to_PyObject(symnmf_matrix, N, k);
    free_matrix(W, N);
    free_matrix(symnmf_matrix, N);
    return result;
}


static PyObject* sym(PyObject *self, PyObject *args){
    return symnmf_ops(self, args, 0);
}


static PyObject* ddg(PyObject *self, PyObject *args){
    return symnmf_ops(self, args, 1);
}


static PyObject* norm(PyObject *self, PyObject *args){
    return symnmf_ops(self, args, 2);
}


static PyMethodDef matrixMethods[] = {
    {"symnmf",
    (PyCFunction) symnmf,
    METH_VARARGS,
    PyDoc_STR("Executing the SymNMF algoritm")},

    {"sym",
    (PyCFunction) sym,
    METH_VARARGS,
    PyDoc_STR("Computing the similarity matrix")},

    {"ddg",
    (PyCFunction) ddg,
    METH_VARARGS,
    PyDoc_STR("Computing the diagonal degerr matrix")},

    {"norm",
    (PyCFunction) norm,
    METH_VARARGS,
    PyDoc_STR("Computing the normalized similarity matrix")},
    {NULL, NULL, 0, NULL}

};


static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT,
    "SymNMF",
    NULL,
    -1,
    matrixMethods
};


PyMODINIT_FUNC PyInit_SymNMF(void){
    PyObject *m;
    m = PyModule_Create(&symnmfmodule);
    if (!m){
        return NULL;
    }
    return m;
}


double** convert_to_matrix(PyObject* inputList, int rows, int cols){
    Py_ssize_t i, j;
    PyObject* sublist;
    double** datapoints_arr = allocate_matrix(rows, cols, 1);
    if (!datapoints_arr){
        return NULL;
    }
    for(i=0; i<rows; i++){
        sublist = PySequence_GetItem(inputList, i);
        for(j=0; j<cols; j++){
            datapoints_arr[i][j] = PyFloat_AsDouble(PySequence_GetItem(sublist, j));
        }
    }
    return datapoints_arr;
}


PyObject* convert_to_PyObject(double** matrix, int rows, int cols){
    PyObject* result;
    Py_ssize_t i, j;
    PyObject* innerList;
    if (matrix == NULL){
        return NULL;
    }
    result = PyList_New(rows);
    if (result == NULL){
        return NULL;
    }
    for(i=0; i< rows; i++){
        innerList = PyList_New(cols);
        if (!innerList){
            Py_DECREF(result);
            return NULL;
        }
        for(j=0; j< cols; j++){
            PyObject* floatObj = PyFloat_FromDouble(matrix[i][j]);
            if (!floatObj){
                Py_DECREF(innerList);
                Py_DECREF(result);
                return NULL;
            }
            PyList_SET_ITEM(innerList, j, floatObj);
        }
        PyList_SET_ITEM(result, i, innerList);
    }
    
    return result;
}
