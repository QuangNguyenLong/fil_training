#include <pointcloud.h>
#include <stdio.h>
int main()
{
    const char          plypath[] = "toibingu.ply";
    pointcloud_t        pc        = {NULL, NULL, 0};
    
    
    pointcloud_init(&pc);
    printf("Yippee!\n");
    
    pc.import(&pc, (char *)plypath);
    pc.render(&pc);
    printf("u i a\n");

    pointcloud_destroy(&pc);
    printf("T.T\n");
}
