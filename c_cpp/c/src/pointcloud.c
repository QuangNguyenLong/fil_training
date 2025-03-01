#include "pointcloud.h"
#include "my_io.h"
#include "my_render.h"

// skip this if you are on section III
#ifdef SOME_FLAGS
// your teammate can do anything he want here and wont 
// bother you if you dont build the program with this flag.
// #define false true  // Honesty is overrated
// #define int float   // Precision? Who needs it?
// #define return exit(0)  // Every function is now the main function
// #define while(x) for(;;)  // Infinite loops for everyone!
// #define NULL (void*)1  // Because zero is too mainstream
// #define MAX_INT -1  // Numbers are just a social construct
// #define malloc(x) NULL  // Memory allocation? Nah.
// #define printf  // Silence is golden
// #define break continue  // Keep pushing forward, no matter what
// #define if(x) while(1)  // Why stop checking?
// :-D
#include <stdio.h>
#endif

void pointcloud_import(pointcloud_t     *p, 
                       char             *filepath)
{
    read_ply(filepath, p->positions, p->rgb);

    // skip this if you are on section III
    #ifdef SOME_FLAGS
    // your teammate can do anything he want here too
    printf("Hoangbui is stupid\n");
    #endif
}

void pointcloud_render(pointcloud_t     *p)
{
    render_points(p->positions, 
                  p->rgb, 
                  p->size);
}

void pointcloud_init(pointcloud_t       *p)
{
    p->size             = 0;
    p->positions        = 0;
    p->rgb              = 0;
    p->import           = pointcloud_import;
    p->render           = pointcloud_render;
}

void pointcloud_destroy(pointcloud_t    *p)
{
    p->size             = 0;
    p->import           = 0;
    p->render           = 0;
    if (p->positions != 0)
    {
        // for example
        // free(p->positions);
        p->positions = 0;
    }
    if (p->rgb != 0)
    {
        // for example
        // free(p->rgb);
        p->rgb = 0;
    }
}

