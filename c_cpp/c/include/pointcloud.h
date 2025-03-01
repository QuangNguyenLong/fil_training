#ifndef POINTCLOUD_H
#define POINTCLOUD_H

typedef struct pointcloud_t pointcloud_t;

struct pointcloud_t
{
    float               *positions;
    unsigned char       *rgb;
    unsigned long       size;

    // This is the pointer that point to a function.
    // If you do not know this, ask chatGPT, it is 
    // smarter than you. 
    void (*import)(struct pointcloud_t  *, 
                   char                 *);
    void (*render)(struct pointcloud_t  *);
};

void pointcloud_init(pointcloud_t       *p);
void pointcloud_destroy(pointcloud_t    *p);

void pointcloud_import(pointcloud_t     *p, 
                       char             *filepath);
void pointcloud_render(pointcloud_t     *p);
#endif
