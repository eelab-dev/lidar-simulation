import numpy as np
import random 
import sys
from pythonLib.scenGenLib import*
import argparse
# Main execution
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Export a Cornell Box scene to OBJ format.")
    parser.add_argument("-o", "--output_file", type=str, default="cornell_box.obj", help="Output OBJ file path")
    parser.add_argument("--detectorDistance", type=float, default = 1000 , help = "distance from detector to the ")
    parser.add_argument("--generate_camera", action="store_true", help="Flag to indicate camera generation")
    parser.add_argument("--camera_location", nargs=3, type=float, help="Camera location coordinates (x y z)")
    parser.add_argument("--look_at", nargs=3, type=float, help="Camera look-at coordinates (x y z)")
    parser.add_argument("--camera_file", type=str, help="Path to generated camera JSON file")    
    output_file = "cornell_box.obj"
    detector_distance = 1000
    args = parser.parse_args()
    if args.output_file:
        output_file = args.output_file
    if args.generate_camera:
        camera_output_file = args.camera_file
        camear_location = args.camera_location
        camear_look = args.look_at
        generate_camera_json(camera_position=camear_location,look_at_point=camear_look,filename=camera_output_file)



    materials = [
        material_info(material_type="lambert", material_name="white",  properties={"reflectivity":0.8}),
        material_info(material_type="lambert", material_name="red",    properties={"reflectivity":0.5}),
        material_info(material_type="lambert", material_name="green",  properties={"reflectivity":0.3}),
    ]
    material_names = [m._material_name for m in materials]

    def random_material():
        return random.choice(material_names)

    # dims
    box_width, box_height, box_depth = 512, 512, 512
    wall_width = 7

    # Create scene object
    scene_obj = scene(materials)

    # Floor
    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, 0.5*wall_width, 0],
        random_material(),
        "floor"
    )

    # Ceiling
    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, box_height + 0.5*wall_width, 0],
        random_material(),
        "ceiling"
    )

    # Back wall
    scene_obj.create_box_with_material(
        [box_width, box_height + wall_width, wall_width],
        [0, box_height/2 + 0.5*wall_width, -box_depth/2],
        random_material(),
        "back_wall"
    )

    # Left wall (Red)
    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [-box_width/2, box_height/2 + 0.5*wall_width, 0],
        random_material(),
        "left_wall"
    )

    # Right wall (Green)
    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [ box_width/2, box_height/2 + 0.5*wall_width, 0],
        random_material(),
        "right_wall"
    )

    # Ground plate
    scene_obj.create_box_with_material(
        [box_width*5, 1, box_depth*5],
        [0, 0, 0],
        random_material(),
        "ground"
    )

    # Wall plate
    scene_obj.create_box_with_material(
        [box_width*5, box_height*5, 1],
        [0, (box_height*5)/2, -box_depth/2 - 200],
        random_material(),
        "wall"
    )


    short_block_width, short_block_depth, short_block_height = 160, 165, 160 
    tall_block_width = short_block_width + random.random() * 0.3 * short_block_width 
    tall_block_depth = short_block_depth + random.random() * 0.2 * short_block_depth 
    tall_block_height = 330 + random.uniform(-1, 1) * 0.1 * 330 
    distance = 300 + random.uniform(-1, 1) * 0.2 * 300 
    block_x = random.uniform(-box_width / 2 + tall_block_width / 2, box_width / 2 - tall_block_width / 2) 
    short_block_z = random.uniform(-265+0.5*short_block_depth, 260 - tall_block_depth -0.5*short_block_depth) 
    tall_block_z = random.uniform(short_block_z + 0.5*short_block_depth + 5 + 0.5 * tall_block_depth, 265 - 0.5*tall_block_depth)

    # Tall block
    scene_obj.create_box_with_material(
        [tall_block_width, tall_block_height, tall_block_depth],
        [block_x, tall_block_height/2 + 0.5*wall_width, tall_block_z],
        random_material(),
        "tall_box"
    )

    # Short block
    scene_obj.create_box_with_material(
        [short_block_width, short_block_height, short_block_depth],
        [block_x, short_block_height/2 + 0.5*wall_width, short_block_z],
        random_material(),
        "short_box"
    )

    # Print scene parameters
    print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")
    print(f"📌 Short block: {short_block_width}, {short_block_height}, {short_block_depth}")
    print(f"📌 Tall block: {tall_block_width}, {tall_block_height}, {tall_block_depth}")
    print(f"📌 Distance between blocks: {tall_block_z - short_block_z}")
    print(f"📌 Block X position: {block_x}")

    # scene_obj.save()
    scene_obj.export_scene(file_name=output_file)
    print("Wrote box_mesh.usda")











