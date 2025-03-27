import trimesh

class scene:
    def __init__(self, materials):
        self._scene = trimesh.Scene()
        self._materials = materials

    def add_geometry(self, geom, geom_name):
        geom.metadata["name"] = geom_name  # Assign name explicitly
        self._scene.add_geometry(geom, node_name=geom_name)

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