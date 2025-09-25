import trimesh
import numpy as np
import pywavefront
import json
class scene:
    def __init__(self, materials):
        self._scene = trimesh.Scene()
        self._materials = materials

    def add_geometry(self, geom, geom_name):
        geom.metadata["name"] = geom_name  # Assign name explicitly
        self._scene.add_geometry(geom, node_name = geom_name)

    def create_box_with_material(self, size, position, material_name):
        """
        Create a trimesh box with a specific material.
        :param size: (width, height, depth) of the box
        :param position: (x, y, z) position of the box center
        :param material_name: Name of the material to assign
        :return: trimesh.Mesh object
        """
        box = trimesh.creation.box(extents=size)
        box.apply_translation(position)

        # Retrieve material properties
        material = self._materials.get(material_name, self._materials["white"])
        diffuse_color = material["Kd"]

        # Set visual properties and link material
        box.visual = trimesh.visual.TextureVisuals(material=material)
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

    def export_scene(self, output_file="cornell_box.obj"):
        """Exports the scene to an OBJ file."""
        self._scene.export(output_file)


    def remove_geometry_by_name(self, name: str):
        """
        Removes a geometry by its name from the scene.
        """
        if name in self._scene.geometry:
            self._scene.delete_geometry(name)

    @classmethod
    def from_obj(cls, obj_file: str):
    # Load the OBJ and MTL using pywavefront
        wavefront_scene = pywavefront.Wavefront(
            obj_file,
            collect_faces=True,
            create_materials=True,
            parse=True,
        )

        # Extract materials
        materials = {}
        for mat in wavefront_scene.materials.values():
            materials[mat.name] = {
                "Ka": mat.ambient or [0, 0, 0],
                "Kd": mat.diffuse or [1, 1, 1],
                "Ks": mat.specular or [0, 0, 0],
                "Ke": [0, 0, 0],
            }

        # Create scene instance
        instance = cls(materials)


        # Global vertex pool
        global_vertices = np.array(wavefront_scene.vertices, dtype=np.float32)

        # Process each mesh
        for mesh in wavefront_scene.mesh_list:
            name = mesh.name
            if not mesh.faces:
                continue

            faces = np.array(mesh.faces, dtype=np.int32)

            # Step 1: Find used vertex indices
            used = np.unique(faces.flatten())

            # Step 2: Extract only used vertices
            vertices = global_vertices[used]

            # Step 3: Remap face indices to local vertex array
            index_map = {old: new for new, old in enumerate(used)}
            remapped_faces = np.array([[index_map[i] for i in face] for face in faces], dtype=np.int32)

            # Step 4: Create trimesh geometry
            geom = trimesh.Trimesh(vertices=vertices, faces=remapped_faces, process=False)

            # Step 5: Apply material
            mat_name = mesh.materials[0].name if mesh.materials else "white"
            mat_data = materials.get(mat_name, materials["white"])
            geom.visual = trimesh.visual.TextureVisuals(material=mat_data)
            geom.visual.material = trimesh.visual.material.SimpleMaterial(
                name=mat_name,
                ambient=mat_data["Ka"],
                diffuse=mat_data["Kd"],
                specular=mat_data["Ks"],
                emissive=mat_data["Ke"],
            )
            geom.visual.material.name = mat_name
            # Step 6: Add to scene
            instance.add_geometry(geom, geom_name=name)

        return instance



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