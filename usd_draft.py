from pxr import Usd, UsdGeom, UsdShade, Sdf, Gf
import numpy as np
import argparse
import sys
import random 

class material_info:

    def __init__(self,material_type,material_name,properties):
        self._material_type = material_type 
        self._material_name = material_name
        self._properties = properties


class scene:

    def __init__(self,file_name,materials):
        self._stage = Usd.Stage.CreateNew(file_name)
        self._geometries = UsdGeom.Xform.Define(self._stage, Sdf.Path("/geometries"))
        self._materials  = UsdGeom.Xform.Define(self._stage, Sdf.Path("/materials"))

        self._geometryPath = "/geometries"
        self._materialPath = "/materials"
        self._materialMap = {}


        self.add_material(materials)

    def add_material(self,materials):


        for material in materials:

            if(material._material_type):
                mat_path = self._materialPath + "/" + material._material_name
                mat = UsdShade.Material.Define(self._stage, Sdf.Path(mat_path))
                mat.CreateInput("materialType", Sdf.ValueTypeNames.Token).Set("lambert")
                # float inputs:reflectivity = 0.5
                mat.CreateInput("reflectivity", Sdf.ValueTypeNames.Float).Set(material._properties["reflectivity"])
                self._materialMap[material._material_name] = mat_path 
        return 


 

    def create_box_with_material(self, size, position, material_name, geometry_name):

        mat_path=self._materialMap[material_name]
        mat = UsdShade.Material.Get(self._stage, Sdf.Path(mat_path))
        if mat_path == None:
            print(f"[warn] Material not found at {mat_path}; mesh left unbound.")
            return False
        
        w, h, d = map(float, size)
        cx, cy, cz = map(float, position)
        hx, hy, hz = w/2, h/2, d/2
        # t = Gf.Vec3f(tx, ty, tz)
        pts = [        
            Gf.Vec3f(cx - hx, cy - hy, cz + hz),  # (-250, 0,   250)
            Gf.Vec3f(cx - hx, cy + hy, cz + hz),  # (-250, 5,   250)
            Gf.Vec3f(cx - hx, cy - hy, cz - hz),  # (-250, 0,  -250)
            Gf.Vec3f(cx + hx, cy - hy, cz - hz),  # ( 250, 0,  -250)
            Gf.Vec3f(cx - hx, cy + hy, cz - hz),  # (-250, 5,  -250)
            Gf.Vec3f(cx + hx, cy + hy, cz + hz),  # ( 250, 5,   250)
            Gf.Vec3f(cx + hx, cy - hy, cz + hz),  # ( 250, 0,   250)
            Gf.Vec3f(cx + hx, cy + hy, cz - hz),  # ( 250, 5,  -250)

        ]

        faceVertexCounts = [3]*12
        faceVertexIndices = [
            0, 1, 2,  3, 0, 2,  2, 1, 4,  4, 3, 2,
            0, 5, 1,  6, 0, 3,  6, 5, 0,  1, 5, 4,
            7, 3, 4,  4, 5, 7,  7, 6, 3,  5, 6, 7
        ]
        obj_path = self._geometryPath + "/" + geometry_name
        geometryObj =  UsdGeom.Xform.Define(self._stage, Sdf.Path(obj_path))
        mesh_path = obj_path + "/" + geometry_name + "_mesh"
        mesh = UsdGeom.Mesh.Define(self._stage, Sdf.Path(mesh_path))
        mesh.CreatePointsAttr(pts)
        mesh.CreateFaceVertexCountsAttr(faceVertexCounts)
        mesh.CreateFaceVertexIndicesAttr(faceVertexIndices)
        UsdShade.MaterialBindingAPI(mesh.GetPrim()).Bind(mat)
        return True



    def save(self):
        self._stage.GetRootLayer().Save()       




# ---------------- example usage ----------------
if __name__ == "__main__":




    materials = [
        material_info(material_type="lambert", material_name="white",  properties={"reflectivity":0.5}),
        material_info(material_type="lambert", material_name="red",    properties={"reflectivity":0.5}),
        material_info(material_type="lambert", material_name="green",  properties={"reflectivity":0.5}),
    ]

    # dims
    box_width, box_height, box_depth = 512, 512, 512
    wall_width = 7

    # Create scene object
    scene_obj = scene("box_mesh.usda",materials)

    # Floor
    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, 0.5*wall_width, 0],
        "white",
        "floor"
    )

    # Ceiling
    scene_obj.create_box_with_material(
        [box_width, wall_width, box_depth],
        [0, box_height + 0.5*wall_width, 0],
        "white",
        "ceiling"
    )

    # Back wall
    scene_obj.create_box_with_material(
        [box_width, box_height + wall_width, wall_width],
        [0, box_height/2 + 0.5*wall_width, -box_depth/2],
        "white",
        "back_wall"
    )

    # Left wall (Red)
    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [-box_width/2, box_height/2 + 0.5*wall_width, 0],
        "red",
        "left_wall"
    )

    # Right wall (Green)
    scene_obj.create_box_with_material(
        [wall_width, box_height + wall_width, box_depth],
        [ box_width/2, box_height/2 + 0.5*wall_width, 0],
        "green",
        "right_wall"
    )

    # Ground plate
    scene_obj.create_box_with_material(
        [box_width*5, 1, box_depth*5],
        [0, 0, 0],
        "white",
        "ground"
    )

    # Wall plate
    scene_obj.create_box_with_material(
        [box_width*5, box_height*5, 1],
        [0, (box_height*5)/2, -box_depth/2 - 300],
        "white",
        "wall"
    )




    short_block_width, short_block_depth, short_block_height = 160, 165, 160
    tall_block_width  = short_block_width * (1 + 0.3 * random.random())
    tall_block_depth  = short_block_depth * (1 + 0.2 * random.random())
    tall_block_height = 330 * (1 + 0.1 * random.uniform(-1, 1))
    distance          = 300 * (1 + 0.2 * random.uniform(-1, 1))  # if you use it later

    block_x = random.uniform(-box_width/2 + tall_block_width/2,
                            box_width/2 - tall_block_width/2)

    short_block_z = random.uniform(-265 + 0.5*short_block_depth,
                                260 - tall_block_depth - 0.5*short_block_depth)

    tall_block_z  = random.uniform(short_block_z + 0.5*short_block_depth + 5 + 0.5*tall_block_depth,
                                265 - 0.5*tall_block_depth)

    # Tall block
    scene_obj.create_box_with_material(
        [tall_block_width, tall_block_height, tall_block_depth],
        [block_x, tall_block_height/2 + 0.5*wall_width, tall_block_z],
        "white",
        "tall_box"
    )

    # Short block
    scene_obj.create_box_with_material(
        [short_block_width, short_block_height, short_block_depth],
        [block_x, short_block_height/2 + 0.5*wall_width, short_block_z],
        "white",
        "short_box"
    )


    # Print scene parameters
    print(f"📌 Box dimensions: {box_width}, {box_height}, {box_depth}")
    print(f"📌 Short block: {short_block_width}, {short_block_height}, {short_block_depth}")
    print(f"📌 Tall block: {tall_block_width}, {tall_block_height}, {tall_block_depth}")
    print(f"📌 Distance between blocks: {tall_block_z - short_block_z}")
    print(f"📌 Block X position: {block_x}")
    scene_obj.save()
    print("Wrote box_mesh.usda")







