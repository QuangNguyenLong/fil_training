import my_package.my_class as cls

plypath = "toibingu.ply"

pc = cls.pointcloud()

pc.read(plypath)

pc.render()

print(pc.size, pc.positions, pc.rgb)