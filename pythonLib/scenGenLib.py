import trimesh

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
        # box.visual.material.name = material_name
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
        loaded = trimesh.load(obj_file, force='scene')

        # Initialize an empty material dictionary
        materials = {}
        scene = trimesh.load(obj_file, force='scene')

        print("Object | Geometry | Material")

        for geometry_name, geom in scene.geometry.items():
            print(f"\n[Geometry: {geometry_name}]")
            mat = geom.visual.material
            print(f"Material class: {type(mat)}")
            print(f"Material name: {getattr(mat, 'name', 'None')}")
            print(f"Material diffuse: {getattr(mat, 'diffuse', 'N/A')}")

        # Collect materials properly
        if isinstance(loaded, trimesh.Scene):
            for name, geom in loaded.geometry.items():
                mat = geom.visual.material
                mat_name = getattr(mat, 'name', None) or f"default_{name}"
                # Store material properties (avoid duplication)
                if mat_name not in materials:
                    materials[mat_name] = {
                        "Ka": getattr(mat, 'ambient', [0, 0, 0]),
                        "Kd": getattr(mat, 'diffuse', [1, 1, 1]),
                        "Ks": getattr(mat, 'specular', [0, 0, 0]),
                        "Ke": getattr(mat, 'emissive', [0, 0, 0]),
                    }
        elif isinstance(loaded, trimesh.Trimesh):
            mat = loaded.visual.material
            mat_name = getattr(mat, 'name', "default_material")
            materials[mat_name] = {
                "Ka": getattr(mat, 'ambient', [0, 0, 0]),
                "Kd": getattr(mat, 'diffuse', [1, 1, 1]),
                "Ks": getattr(mat, 'specular', [0, 0, 0]),
                "Ke": getattr(mat, 'emissive', [0, 0, 0]),
            }

        # Create the SceneWrapper instance
        instance = cls(materials)
        instance._scene = loaded if isinstance(loaded, trimesh.Scene) else trimesh.Scene(loaded)

        return instance