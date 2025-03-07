# A sample C project tutorial

--- 

## I. INTRODUCTION
This project is meant to help the reader excel in C programming by teaching them how to structure, write, and manage a clean, maintainable and efficient C project.

The reader is expected to have basic knowledge of C programming. By the end of this tutorial, the reader will have a strong understanding of:
- Organizing a C project into multiple files and modules
- Writing reusable, maintainable, and well-documented code
- Handling memory management and avoiding common pitfalls
- Debugging, testing, and automating builds using CMake

--- 

## II. SETTING UP THE PROJECT

### 1. PROJECT DIRECTORY STRUCTURE
A well-structured project makes code easier to navigate and maintain. 
Below is the typical structure:

```
   my_project/
   ├── src/             # Source files
   │   ├── my_io.c
   │   ├── my_render.c
   │   ├── pointcloud.c
   ├── include/         # Header files
   │   ├── my_io.h
   │   └── my_render.h
   │   ├── pointcloud.h
   ├── build/           # Compiled binaries
   ├── my_program/      # example program
   │   └── main.c
   └── CMakeLists.txt   # Build automation
```

The project structure is simple, yet effective. "Why?", you ask. If you want to implement a C program, the usual way is to do it all in a `foo.c` source file and compile it with `gcc foo.c -o foo`.

Of course, if you do not want to reuse any of those line of codes, nothing is wrong. But normally that is not the case. Suppose you are developing a program to view point cloud. You write a function to import a PLY file.

```
void read_ply(const char *filepath, float *positions, unsigned char *rgb)
{
    // your dumb-ass code with memory leak.
}
```

Later on, after have done COPY-PASTE-ing this function accross multiple program, you need to make another program to view point cloud, this time, a DYNAMIC POINT CLOUD. You are lazy, you will copy-paste the function. Now, since your code have memory leak, the leaked memory will surely stacks over time, and your computer crash. You realized it and come back to fix all your code. What a "good" deal.

"Well, just write new code then. I have plenty of time.", you said at the age of 20 with no achievements.

The better approach is to build your own header files, reuse it, whenever you found a bug, fix it (once only). You can do this by putting your prototype functions in a header, and declare the functions in a source file. For example check this [header](include/my_io.h) and [source file](src/my_io.c).

### 2. Build the project

To compile and run the project, you need:
- GCC    : sudo apt install gcc
- Make   : sudo apt install make
- CMake  : sudo apt install cmake
- GDB    : sudo apt install gdb

`gdb` is only meant for debugging, if you never have bugs in your code, forget this.

`cmake` is used to generate Makefiles.

`make` tells the compiler what to do with your trash code using Makefiles.

`gcc` (really ? you have to read this ?)

So to build a project, you will need a file like [this](CMakeLists.txt).

Then build the project:
```sh
mkdir build
cd build
cmake ..
make
```

Done, you have successfully built yourself a C project.

--- 

## III. MODULE YOUR CODE

### 1. Separating Concerns with Modules

Let’s go back to our point cloud viewer example. Since you also have to render the point cloud to view it, instead of declaring all header functions in one messy file, split it into mutiple header and source files, which called modules. By so, you can separate each module focuses on different job, so if you failed to view a point cloud, you know to check [my_render.c](src/my_render.c) and [my_render.h](include/my_render.h) instead of sifting through a 2000-line toibingu.c or something idk I am not dumb.

###  2. OOP in C

C is not an object-oriented language like C++ or Java, but using a module, you can still mimic object-oriented principles like encapsulation, abstraction, and modularity using structs and function pointers. Take a look at these: [header](include/pointcloud.h) and [source file](src/pointcloud.c). Notice I declare a `pointcloud_t` class.
 
### 3. Usage

Check out how I render a point cloud in my main function [here](my_program/main.c). 


--- 


## IV. WRITING CLEAN CODE

Why are you reading this ? Because your code is trash.

---

## V. FEATURE TOGGLES  

At some point, you'll need to enable or disable certain features—maybe for debugging, platform-specific behavior, or experimental functionality. Instead of manually changing the source code every time, you can use **CMake compile definitions** to toggle features dynamically.  

### Using CMake to Enable Features  

With **CMake options**, you can define feature flags at compile time:  

```cmake
option(SOME_FEATURE "Enable some feature" OFF)
if(SOME_FEATURE)
    # Define SOME_FEATURE in the program my_program
    target_compile_definitions(my_program PRIVATE SOME_FEATURE)
endif()
```

Now, when configuring the build, you (or anyone else) can toggle this feature without touching the code:  

```sh
cmake .. -DSOME_FEATURE=ON
```

If `SOME_FEATURE` is `ON`, the macro `SOME_FEATURE` will be defined in `my_program`. You can then use it in your code to conditionally enable specific functionality. Check out [this example](src/pointcloud.c).  

### Building and Running with the Feature Enabled  

To build the project with the feature flag:  

```sh
mkdir build
cd build
cmake .. -DSOME_FEATURE=ON
make
./my_program
```  

---