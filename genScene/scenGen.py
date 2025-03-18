import trimesh
import numpy as np
import random 
import sys

# Function to create a wall or box with material
def create_box_with_material(size, position, material_name, materials):
    """
    Create a trimesh box with a specific material.
    :param size: (width, height, depth) of the box
    :param position: (x, y, z) position of the box center
    :param material_name: Name of the material to assign
    :param materials: Dictionary of material properties
    :return: trimesh.Mesh object
    """
    box = trimesh.creation.box(extents=size)
    box.apply_translation(position)  # Translate box to position
    # Retrieve material properties
    material = materials[material_name]
    diffuse_color = material["Kd"]
    box.visual = trimesh.visual.TextureVisuals(material=material)
    # Set visual properties and link material
    box.visual.face_colors = [int(c * 255) for c in diffuse_color] + [255]  # RGBA
    box.visual.material = trimesh.visual.material.SimpleMaterial(
        name=material_name,
        ambient=material.get("Ka", [0, 0, 0]),
        diffuse=material.get("Kd", [1, 1, 1]),
        specular=material.get("Ks", [0, 0, 0]),
        emissive=material.get("Ke", [0, 0, 0]),
    )
    box.visual.material.name = material_name
    
    return box




# Colors
colors = {
    "white": [255, 255, 255, 255],
    "red": [255, 0, 0, 255],
    "green": [0, 255, 0, 255],
    "light": [255, 255, 200, 255],  # Slightly yellowish light
    "gray": [200, 200, 200, 255],
}



# Function to write the material file
def write_mtl_file(filename, materials):
    """
    Generate a material file (.mtl) for the Cornell Box.
    """
    with open(filename, "w") as f:
        for name, properties in materials.items():
            f.write(f"newmtl {name}\n")
            for prop, values in properties.items():
                values_str = " ".join(map(str, values))
                f.write(f"{prop} {values_str}\n")
            f.write("\n")


# Function to export the scene with materials
def export_scene_with_materials(scene, obj_filename, mtl_filename, materials):
    """
    Export the OBJ file with the linked MTL file.
    """
    # Write the MTL file
    write_mtl_file(mtl_filename, materials)

    # Write the OBJ file with an `mtllib` directive
    with open(obj_filename, "w") as f:
        # Link the material file
        f.write(f"mtllib {mtl_filename}\n")

        # Add each geometry to the OBJ file
        for geometry in scene.geometry:
            name = geometry.name or "default"
            f.write(f"o {name}\n")  # Object name
            f.write(f"usemtl {name}\n")  # Material name

            # Write vertices
            vertices = geometry.vertices + geometry.translation
            for vertex in vertices:
                f.write(f"v {vertex[0]} {vertex[1]} {vertex[2]}\n")

            # Write faces
            faces = geometry.faces + 1  # OBJ uses 1-based indexing
            for face in faces:
                f.write(f"f {face[0]} {face[1]} {face[2]}\n")


if __name__ == "__main__":

    outputfile = "cornell_box.obj"
    if (len(sys.argv) > 1):
        outputfile = sys.argv[1]

        


    # Material properties
    materials = {
        "white": {"Ka": [0, 0, 0], "Kd": [0.725, 0.71, 0.68], "Ks": [0, 0, 0]},
        "red": {"Ka": [0, 0, 0], "Kd": [0.63, 0.065, 0.05], "Ks": [0, 0, 0]},
        "green": {"Ka": [0, 0, 0], "Kd": [0.14, 0.45, 0.091], "Ks": [0, 0, 0]},
        "blue": {"Ka": [0, 0, 0], "Kd": [0, 0, 1], "Ks": [0, 0, 0]},
        "detector": {"Ka": [47.7688, 38.5664, 31.0928], "Kd": [0.65, 0.65, 0.65], "Ks": [0, 0, 0]},
    }

    # Dimensions (in millimeters)
    box_width, box_height, box_depth = 552.8, 548.8, 559.2

    # Create Cornell Box scene
    scene = trimesh.Scene()

    # Add floor
    floor = create_box_with_material(
        size=[box_width, 1, box_depth],
        position=[0, -0.5, 0],
        material_name="white",
        materials=materials,
    )
    scene.add_geometry(floor, geom_name = "floor")

    # Add ceiling
    ceiling = create_box_with_material(
        size=[box_width, 1, box_depth],
        position=[0, box_height - 0.5, 0],
        material_name="white",
        materials=materials,
    )
    scene.add_geometry(ceiling,geom_name = "celling")
    # Add back wall
    back_wall = create_box_with_material(
        size=[box_width, box_height, 1],
        position=[0, box_height / 2 - 0.5, -box_depth / 2],
        material_name="white",
        materials=materials,
    )
    scene.add_geometry(back_wall, geom_name="back_wall")


    # Add left wall (Red)
    left_wall = create_box_with_material(
        size=[1, box_height, box_depth],
        position=[-box_width / 2, box_height / 2 - 0.5, 0],
        material_name="red",
        materials=materials,
    )
    scene.add_geometry(left_wall, geom_name="left_wall")

    # Add right wall (Green)
    right_wall = create_box_with_material(
        size=[1, box_height, box_depth],
        position=[box_width / 2, box_height / 2 - 0.5, 0],
        material_name="green",
        materials=materials,
    )
    scene.add_geometry(right_wall, geom_name= "right_wall")

    # Light source (rectangle on the ceiling)
    detector = create_box_with_material(
        size=[box_width/10, box_depth/10, 0],
        position=[-30, box_height/2, box_depth / 2 + 1000],
        materials=materials,
        material_name = "detector",
    )
    scene.add_geometry(detector, geom_name = "detector")

    # Dimensions for blocks
    short_block_width = 160
    short_block_depth = 165
    short_block_height = 160

    tall_block_width = short_block_width + random.random() * 0.2 * short_block_width
    tall_block_depth = short_block_depth + random.random() * 0.2 * short_block_depth
    tall_block_height = 330 + random.uniform(-1, 1) * 0.1 * 330
    distance = 300 + random.uniform(-1, 1) * 0.1 * 300

    block_x = random.uniform(-box_width / 2 + tall_block_width / 2, box_width / 2 - tall_block_width / 2)


    # Short block
    # short_block = create_box_with_material(
    #     size=[short_block_width, short_block_height, short_block_depth],
    #     position=[block_x, short_block_height / 2, -150],
    #     material_name="white",
    #     materials=materials,
    # )
    # scene.add_geometry(short_block, geom_name = "short_box")

    # Tall block
    tall_block = create_box_with_material(
        size=[tall_block_width, tall_block_height, tall_block_depth],
        position=[block_x, tall_block_height / 2, -150 + distance],
        material_name="white",
        materials=materials,
    )
    scene.add_geometry(tall_block, geom_name = "tall_box")

    # write_mtl_file("cornell_box.mtl",materials)

    # Print the necessary parameters
    print(f"Box dimensions (width, height, depth): {box_width}, {box_height}, {box_depth}")
    print(f"Colors: {colors}")
    print(f"Short block dimensions (width, height, depth): {short_block_width}, {short_block_height}, {short_block_depth}")
    print(f"Tall block dimensions (width, height, depth): {tall_block_width}, {tall_block_height}, {tall_block_depth}")
    print(f"Distance between blocks: {distance}")
    print(f"Block X position: {block_x}")
    # Export to OBJ
    scene.export(outputfile)
    print("Cornell Box exported")
