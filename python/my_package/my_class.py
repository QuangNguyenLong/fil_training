from my_package.my_io import read_ply
from my_package.my_render import render_points

class pointcloud:
    def __init__(self):
        self.size      = 0
        self.positions = []
        self.rgb       = []
        return
    
    # similar to pointcloud_import in C
    def read(self, filepath):
        ret            = read_ply(filepath)
        self.positions = ret[0]
        self.rgb       = ret[1]
        self.size      = ret[2]
        return
    
    def render(self):
        return render_points(self.positions, 
                             self.rgb,
                             self.size)
    
    