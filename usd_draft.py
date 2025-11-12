from pxr import Usd, UsdGeom, UsdShade, Sdf, Gf
import numpy as np
import argparse
import sys
import random 
from  pythonScripts.pythonLib.scenGenLib import*   




# ---------------- example usage ----------------
if __name__ == "__main__":




#     materials = [
#         material_info(material_type="lambert", material_name="white",  properties={"reflectivity":0.5}),
#         material_info(material_type="lambert", material_name="red",    properties={"reflectivity":0.5}),
#         material_info(material_type="lambert", material_name="green",  properties={"reflectivity":0.5}),
#     ]

#     # dims
#     box_width, box_height, box_depth = 512, 512, 512
#     wall_width = 7

#     # Create scene object
#     scene_obj = scene(materials)

#     # Floor
#     scene_obj.create_box_with_material(
#         [box_width, wall_width, box_depth],
#         [0, 0.5*wall_width, 0],
#         "white",
#         "floor"
#     )

#     # Ceiling
#     scene_obj.create_box_with_material(
#         [box_width, wall_width, box_depth],
#         [0, box_height + 0.5*wall_width, 0],
#         "white",
#         "ceiling"
#     )

#     # Back wall
#     scene_obj.create_box_with_material(
#         [box_width, box_height + wall_width, wall_width],
#         [0, box_height/2 + 0.5*wall_width, -box_depth/2],
#         "white",
#         "back_wall"
#     )

#     # Left wall (Red)
#     scene_obj.create_box_with_material(
#         [wall_width, box_height + wall_width, box_depth],
#         [-box_width/2, box_height/2 + 0.5*wall_width, 0],
#         "red",
#         "left_wall"
#     )

#     # Right wall (Green)
#     scene_obj.create_box_with_material(
#         [wall_width, box_height + wall_width, box_depth],
#         [ box_width/2, box_height/2 + 0.5*wall_width, 0],
#         "green",
#         "right_wall"
#     )

#     # Ground plate
#     scene_obj.create_box_with_material(
#         [box_width*5, 1, box_depth*5],
#         [0, 0, 0],
#         "white",
#         "ground"
#     )

#     # Wall plate
#     scene_obj.create_box_with_material(
#         [box_width*5, box_height*5, 1],
#         [0, (box_height*5)/2, -box_depth/2 - 300],
#         "white",
#         "wall"
#     )




#     short_block_width, short_block_depth, short_block_height = 160, 165, 160
#     tall_block_width  = short_block_width * (1 + 0.3 * random.random())
#     tall_block_depth  = short_block_depth * (1 + 0.2 * random.random())
#     tall_block_height = 330 * (1 + 0.1 * random.uniform(-1, 1))
#     distance          = 300 * (1 + 0.2 * random.uniform(-1, 1))  # if you use it later

#     block_x = random.uniform(-box_width/2 + tall_block_width/2,
#                             box_width/2 - tall_block_width/2)

#     short_block_z = random.uniform(-265 + 0.5*short_block_depth,
#                                 260 - tall_block_depth - 0.5*short_block_depth)

#     tall_block_z  = random.uniform(short_block_z + 0.5*short_block_depth + 5 + 0.5*tall_block_depth,
#                                 265 - 0.5*tall_block_depth)

#     # Tall block
#     scene_obj.create_box_with_material(
#         [tall_block_width, tall_block_height, tall_block_depth],
#         [block_x, tall_block_height/2 + 0.5*wall_width, tall_block_z],
#         "white",
#         "tall_box"
#     )

#     # Short block
#     scene_obj.create_box_with_material(
#         [short_block_width, short_block_height, short_block_depth],
#         [block_x, short_block_height/2 + 0.5*wall_width, short_block_z],
#         "white",
#         "short_box"
#     )


#     # Print scene parameters
#     print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")
#     print(f"📌 Short block: {short_block_width}, {short_block_height}, {short_block_depth}")
#     print(f"📌 Tall block: {tall_block_width}, {tall_block_height}, {tall_block_depth}")
#     print(f"📌 Distance between blocks: {tall_block_z - short_block_z}")
#     print(f"📌 Block X position: {block_x}")
#     print("Wrote box_mesh.usda")
#     scene_obj.export_scene("box_mesh.usda")


    input_file = "regenerated_box_mesh.usda"
    repeat_scene = scene().from_usd_file("box_mesh.usda")
    # repeat_scene.from_obj(input_file)
    repeat_scene.remove_geometry_by_name("short_box")
    repeat_scene.export_scene(input_file)









