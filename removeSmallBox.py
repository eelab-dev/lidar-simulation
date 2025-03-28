import sys
from pythonLib.scenGenLib import*
import argparse
# Main execution
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Modify an input .obj file (e.g. remove geometry by name) and export a new .obj file."
    )

    parser.add_argument("--input_file", "-i", required=True, help="Path to the input .obj file")
    parser.add_argument("--output_file", "-o", required=True, help="Path to the output .obj file")

    input_file = None 
    output_file = None 
    args = parser.parse_args()

    if args.input_file:
        input_file = args.input_file

    if args.output_file:
        output_file = args.output_file

    if input_file is None or output_file is None:
        print("⚠️  Error: Both --input_file and --output_file must be provided.")
        parser.print_help()
        sys.exit(1)
    obj_to_remove = "short_box"

    myScene = scene.from_obj(input_file)
    myScene.remove_geometry_by_name(obj_to_remove)
    myScene.export_scene(output_file)