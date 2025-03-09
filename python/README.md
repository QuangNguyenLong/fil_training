# A sample Python project tutorial

## I. INTRODUCTION
The objective of this tutorial is similar to the [C tutorial](../c_cpp/c/README.md) but focus more on Python.

## II. SETTING UP THE PROJECT

### 1. Project directory structure
A well-structured project makes code easier to navigate and maintain. 
Below is the typical structure:

```
   my_project/
    ├── my_package        # Your modules
    │   ├── __init__.py
    │   ├── my_class.py
    │   ├── my_io.py
    │   └── my_render.py
    ├── my_program.py     # Example program
    ├── README.md
    └── requirements.txt  # Dependencies
```

### 2. Build the project

```shell
# (SHOULD) setup virtual environment `venv`
python3 -m venv venv

# (SHOULD) activate virtual environment `venv`
source ./venv/bin/activate 

# install dependencies
pip install -r requirements.txt

# run program
python my_program.py
```

**IMPORTANT FOR LINUX USERS**

NEVER mess with **SYSTEM PYTHON**, breaking it can wreck your entire system.  

ALWAYS use a **VIRTUAL ENVIRONMENT** (`virtualenv` or `venv`) for your applications. It keeps dependencies ISOLATED and prevents your libraries from POLLUTING system directories.
