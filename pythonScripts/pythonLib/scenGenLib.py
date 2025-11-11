# import trimesh
import numpy as np
# import pywavefront
import json

from pxr import Usd, UsdGeom, UsdShade, Sdf, Gf
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



def generate_camera_json(camera_position, look_at_point, filename="camera_config.json",detector_width=None, detector_height=None, delay_mean=None, delay_std=None):
    """
    Generate a camera JSON file with variable camera_position and look_at_point.

    Args:
        camera_position (list or tuple): [x, y, z] camera coordinates.
        look_at_point (list or tuple): [x, y, z] target coordinates to look at.
        filename (str): Output JSON filename.
    """
    # Validate inputs
    for name, value in [("camera_position", camera_position), ("look_at_point", look_at_point)]:
        if not (isinstance(value, (list, tuple)) and len(value) == 3):
            raise ValueError(f"{name} must be a list or tuple of three floats (x, y, z).")

    # Structure data
    data = {
        "camera_position": list(map(float, camera_position)),
        "look_at_point": list(map(float, look_at_point))
    }
    
    if detector_width is not None:
        data["detector_width"] = float(detector_width)
    if detector_height is not None:
        data["detector_height"] = float(detector_height)
    if delay_mean is not None:
        data["delay_mean"] = float(delay_mean)
    if delay_std is not None:
        data["delay_std"] = float(delay_std)

    # Write JSON to file
    with open(filename, "w") as f:
        json.dump(data, f, indent=4)
    print(f"[INFO] JSON file '{filename}' generated successfully.")