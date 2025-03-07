import matplotlib.pyplot as plt

POINT_SIZE = 100
RENDERED_PATH = "tmp.png"

def render_points(positions, rgb, size):
    
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    for i in range(size):
        ax.scatter(positions[i][0], 
                    positions[i][1], 
                    positions[i][2], 
                    c=[tuple(v / 255 for v in rgb[i])],
                    s=POINT_SIZE)
    plt.savefig(RENDERED_PATH, dpi=300)
    plt.close(fig)
    
    return