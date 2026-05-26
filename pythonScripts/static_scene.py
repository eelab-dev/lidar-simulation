import numpy as np
import random
import sys
from pythonLib.scenGenLib import *


if __name__ == "__main__":
    materials = [
        material_info(material_type="lambert", material_name="white", properties={"reflectivity": 0.8}),
        material_info(material_type="lambert", material_name="red", properties={"reflectivity": 0.5}),
        material_info(material_type="lambert", material_name="green", properties={"reflectivity": 0.3}),
        material_info(material_type="lambert", material_name="blue", properties={"reflectivity": 0.2}),
        material_info(material_type="lambert", material_name="detector", properties={"reflectivity": 0.65}),
    ]

    box_width, box_height, box_depth = 535, 540, 516
    wall_width = 14

    scene_obj = scene(materials)

    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, 0.5 * wall_width, 0],
        "white",
        "floor",
    )

    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, box_height + 0.5 * wall_width, 0],
        "white",
        "ceiling",
    )

    scene_obj.create_box_with_material(
        [box_width, box_height + wall_width, wall_width],
        [0, box_height / 2 + 0.5 * wall_width, -box_depth / 2],
        "white",
        "back_wall",
    )

    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [-box_width / 2, box_height / 2 + 0.5 * wall_width, 0],
        "red",
        "left_wall",
    )

    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [box_width / 2, box_height / 2 + 0.5 * wall_width, 0],
        "green",
        "right_wall",
    )

    scene_obj.create_box_with_material(
        [box_width * 5, 1, box_depth * 5],
        [0, 0, 0],
        "white",
        "ground",
    )

    scene_obj.create_box_with_material(
        [box_width * 5, box_height * 5, 1],
        [0, box_height * 5 / 2, -box_depth / 2 - 300],
        "white",
        "wall",
    )

    short_block_width, short_block_depth, short_block_height = 158, 158, 168
    tall_block_width, tall_block_depth, tall_block_height = 200, 155, 375

    scene_obj.create_box_with_material(
        [tall_block_width, tall_block_height, tall_block_depth],
        [37.5, tall_block_height / 2 + 0.5 * wall_width, 70],
        "white",
        "tall_box",
    )

    # scene_obj.create_box_with_material(
    #     [short_block_width, short_block_height, short_block_depth],
    #     [0, short_block_height / 2 + 0.5 * wall_width, -121],
    #     "white",
    #     "short_box",
    # )

    output_file = "static_scene.usda"

    print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")

    scene_obj.export_scene(output_file)
