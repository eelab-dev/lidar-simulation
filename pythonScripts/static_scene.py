import numpy as np
import random 
import sys
from pythonLib.scenGenLib import*

if __name__ == "__main__":

    # Material properties
    materials = {
        "white": {"Ka": [0, 0, 0], "Kd": [0.725, 0.71, 0.68], "Ks": [0, 0, 0]},
        "red": {"Ka": [0, 0, 0], "Kd": [0.63, 0.065, 0.05], "Ks": [0, 0, 0]},
        "green": {"Ka": [0, 0, 0], "Kd": [0.14, 0.45, 0.091], "Ks": [0, 0, 0]},
        "blue": {"Ka": [0, 0, 0], "Kd": [0, 0, 1], "Ks": [0, 0, 0]},
        "detector": {"Ka": [47.7688, 38.5664, 31.0928], "Kd": [0.65, 0.65, 0.65], "Ks": [0, 0, 0]},
    }

    box_width, box_height, box_depth = 540, 540, 540
    
    wall_width = 14 
    # Create scene object
    scene_obj = scene(materials)

    # Add floor
    floor = scene_obj.create_box_with_material([box_width, wall_width, box_depth], [0, 0.5* wall_width, 0], "white")
    scene_obj.add_geometry(floor, geom_name="floor")

    # Add ceiling
    ceiling = scene_obj.create_box_with_material([box_width, wall_width, box_depth], [0, box_height + 0.5 * wall_width, 0], "white")
    scene_obj.add_geometry(ceiling, geom_name="ceiling")

    # Add back wall
    back_wall = scene_obj.create_box_with_material([box_width, box_height + wall_width, wall_width], [0, box_height / 2 + 0.5 * wall_width, -box_depth / 2], "white")
    scene_obj.add_geometry(back_wall, geom_name="back_wall")

    # Add left wall (Red)
    left_wall = scene_obj.create_box_with_material([wall_width, box_height + wall_width, box_depth], [-box_width / 2, box_height / 2 + 0.5 * wall_width, 0], "red")
    scene_obj.add_geometry(left_wall, geom_name="left_wall")

    # Add right wall (Green)
    right_wall = scene_obj.create_box_with_material([wall_width, box_height + wall_width, box_depth], [box_width / 2, box_height / 2 + 0.5 * wall_width, 0], "green")
    scene_obj.add_geometry(right_wall, geom_name="right_wall")

    # ground plate
    ground = scene_obj.create_box_with_material([box_width*5, 1, box_depth*5],[0, 0, 0], "white")
    scene_obj.add_geometry(ground, geom_name="ground")

    # wall plate
    wall = scene_obj.create_box_with_material([box_width*5, box_height*5,1], [0,box_height*5/2 , -box_depth/2 - 300], "white")
    scene_obj.add_geometry(wall, geom_name="wall")

    short_block_width, short_block_depth, short_block_height = 158 , 158 , 168

    tall_block_width, tall_block_depth, tall_block_height = 200 , 160 , 380

    tall_block = scene_obj.create_box_with_material([tall_block_width, tall_block_height, tall_block_depth],
                                                    [0, tall_block_height / 2 + 0.5*wall_width,120], "white")
    
    scene_obj.add_geometry(tall_block, geom_name="tall_box")

    short_block = scene_obj.create_box_with_material(
        [short_block_width, short_block_height, short_block_depth],
        [0, short_block_height / 2 + 0.5 * wall_width, -121],
        "white"
    )

    scene_obj.add_geometry(short_block, geom_name = "short_box")
    
    # plate  = scene_obj.create_box_with_material([500, 500,15], [0,250, -7.5 + 250 - (1200 * 3)], "white")
    # scene_obj.add_geometry(plate, geom_name = "short_box")

    

    output_file = "static_obj_1.obj"
    # Print scene parameters
    print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")
    # print(f"📌 Short block: {short_block_width}, {short_block_height}, {short_block_depth}")
    # print(f"📌 Tall block: {tall_block_width}, {tall_block_height}, {tall_block_depth}")

    # Export the scene
    scene_obj.export_scene(output_file)