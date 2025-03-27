import numpy as np
import random 
import sys
from pythonLib.scenGenLib import*
# Main execution
if __name__ == "__main__":
    output_file = "cornell_box.obj"
    if len(sys.argv) > 1:
        output_file = sys.argv[1]

    # Material properties
    materials = {
        "white": {"Ka": [0, 0, 0], "Kd": [0.725, 0.71, 0.68], "Ks": [0, 0, 0]},
        "red": {"Ka": [0, 0, 0], "Kd": [0.63, 0.065, 0.05], "Ks": [0, 0, 0]},
        "green": {"Ka": [0, 0, 0], "Kd": [0.14, 0.45, 0.091], "Ks": [0, 0, 0]},
        "blue": {"Ka": [0, 0, 0], "Kd": [0, 0, 1], "Ks": [0, 0, 0]},
        "detector": {"Ka": [47.7688, 38.5664, 31.0928], "Kd": [0.65, 0.65, 0.65], "Ks": [0, 0, 0]},
    }

    # Dimensions
    box_width, box_height, box_depth = 552.8, 548.8, 559.2

    # Create scene object
    scene_obj = scene(materials)

    # Add floor
    floor = scene_obj.create_box_with_material([box_width, 1, box_depth], [0, -0.5, 0], "white")
    scene_obj.add_geometry(floor, geom_name="floor")

    # Add ceiling
    ceiling = scene_obj.create_box_with_material([box_width, 1, box_depth], [0, box_height - 0.5, 0], "white")
    scene_obj.add_geometry(ceiling, geom_name="ceiling")

    # Add back wall
    back_wall = scene_obj.create_box_with_material([box_width, box_height, 1], [0, box_height / 2 - 0.5, -box_depth / 2], "white")
    scene_obj.add_geometry(back_wall, geom_name="back_wall")

    # Add left wall (Red)
    left_wall = scene_obj.create_box_with_material([1, box_height, box_depth], [-box_width / 2, box_height / 2 - 0.5, 0], "red")
    scene_obj.add_geometry(left_wall, geom_name="left_wall")

    # Add right wall (Green)
    right_wall = scene_obj.create_box_with_material([1, box_height, box_depth], [box_width / 2, box_height / 2 - 0.5, 0], "green")
    scene_obj.add_geometry(right_wall, geom_name="right_wall")

    # Light source (detector)
    detector = scene_obj.create_box_with_material([box_width / 10, box_depth / 10, 0], [-30, box_height / 2, box_depth / 2 + 1000], "detector")
    scene_obj.add_geometry(detector, geom_name="detector")

    # Randomized block properties
    short_block_width, short_block_depth, short_block_height = 160, 165, 160
    tall_block_width = short_block_width + random.random() * 0.2 * short_block_width
    tall_block_depth = short_block_depth + random.random() * 0.2 * short_block_depth
    tall_block_height = 330 + random.uniform(-1, 1) * 0.1 * 330
    distance = 300 + random.uniform(-1, 1) * 0.1 * 300
    block_x = random.uniform(-box_width / 2 + tall_block_width / 2, box_width / 2 - tall_block_width / 2)

    # Add tall block
    tall_block = scene_obj.create_box_with_material([tall_block_width, tall_block_height, tall_block_depth],
                                                    [block_x, tall_block_height / 2, -150 + distance], "white")
    scene_obj.add_geometry(tall_block, geom_name="tall_box")

    # Print scene parameters
    print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")
    print(f"📌 Short block: {short_block_width}, {short_block_height}, {short_block_depth}")
    print(f"📌 Tall block: {tall_block_width}, {tall_block_height}, {tall_block_depth}")
    print(f"📌 Distance between blocks: {distance}")
    print(f"📌 Block X position: {block_x}")

    # Export the scene
    scene_obj.export_scene(output_file)













