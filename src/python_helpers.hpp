#ifndef NPI_PYTHON_HELPERS_HPP
#define NPI_PYTHON_HELPERS_HPP

#include <Python.h>

namespace NPI
{
    class PythonEnsureGil
    {
        public:
            PythonEnsureGil();

            ~PythonEnsureGil();

        private:
            PyGILState_STATE m_state;
    };

    class PythonThreadContext
    {
        public:
            PythonThreadContext();

            ~PythonThreadContext();

        private:
            PyGILState_STATE m_state;

            PyThreadState* m_thread_state;
    };
}

NPI::PythonEnsureGil::PythonEnsureGil()
{
    m_state = PyGILState_Ensure();
}

NPI::PythonEnsureGil::~PythonEnsureGil()
{
    PyGILState_Release(m_state);
}

NPI::PythonThreadContext::PythonThreadContext()
{
    m_state = PyGILState_Ensure();
}

NPI::PythonThreadContext::~PythonThreadContext()
{
    PyGILState_Release(m_state);

    if (PyGILState_Check() == 1)
    {
        m_thread_state = PyEval_SaveThread();
    }
}

#endif
