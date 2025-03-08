# A sample libcurl tutorial

## I. Introduction
The main purpose of this tutorial is to help the reader understand how to link libraries to a C program. `libcurl` is used here as an example, but any other library will work. The reader just needs to modify the following line in [CMakeLists.txt](CMakeLists.txt) accordingly:

```
target_link_libraries(curl_get curl)
```

For example, if you want to link the math library (`libm`) in addition to libcurl (`libm` provides functions like `sqrt()`, `sin()`, ...):
```
target_link_libraries(curl_get curl m)
```

## II. Build the project

```shell
mkdir build
cd build 
cmake ..
make
```
