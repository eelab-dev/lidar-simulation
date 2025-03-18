import numpy as np
import re
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import sys
import os
import warnings







class ray:
    
    def __init__(self, x, y, z, dx, dy, dz, collosion, distance,line_index):
        self.x = x
        self.y = y
        self.z = z
        self.directioinlength = np.sqrt(dx*dx + dy*dy + dz*dz)


        self.dx = dx/self.directioinlength
        self.dy = dy/self.directioinlength
        self.dz = dz/self.directioinlength

        self.collosion = collosion
        self.distance = distance
        



class detector:
    def __init__(self, focal_length, field_of_view, resolution_width, resolution_height):
        self.focal_length = focal_length
        self.field_of_view = field_of_view
        self.resolution_width = resolution_width
        self.resolution_height = resolution_height
        self.pixel_array_count = np.zeros((resolution_width, resolution_height))
        self.pixel_array = np.zeros((resolution_width, resolution_height))
        self.depthImage = np.zeros((resolution_width, resolution_height))

        self.detectorWidth = focal_length * np.tan(np.deg2rad(field_of_view/2)) * 2 * (resolution_width / resolution_height)
        self.detectorHeight = focal_length * np.tan(np.deg2rad(field_of_view/2)) * 2
        self.origin_x = -self.detectorWidth / 2
        self.origin_y = -self.detectorHeight / 2
        self.widthResolution = self.detectorWidth / resolution_width
        self.heightResolution = self.detectorHeight / resolution_height

        self.minDistance = float('inf')
        self.maxDistance = 0

        self.pixel_output_array = [[[] for i in range(self.resolution_height)] for j in range(self.resolution_width)]

        print(f"depthImage shape: {self.depthImage.shape}")
        print(f"pixel_output_array size: {len(self.pixel_output_array)}x{len(self.pixel_output_array[0])}")



    def photon_to_dector(self,income_photon):

        # if income_photon.collosion == 2:
        #     return
        if income_photon.collosion == 0:
            return
        # calculate the intersection point
        t = np.abs(self.focal_length / income_photon.dz)
        x = t * income_photon.dx
        y = t * income_photon.dy
        # calculate the pixel  

        pixel_x =   int((x - self.origin_x) / self.widthResolution)
        pixel_y = self.resolution_height - int((y - self.origin_y) / self.heightResolution)
        self.minDistance = min(self.minDistance, income_photon.distance)
        self.maxDistance = max(self.maxDistance, income_photon.distance)
        if pixel_x >= 0 and pixel_x < self.resolution_width and pixel_y >= 0 and pixel_y < self.resolution_height:
            self.pixel_array_count[pixel_x][pixel_y] += 1
            self.pixel_array[pixel_x][pixel_y] += income_photon.distance
            # print(pixel_x, pixel_y, income_photon.distance, income_photon.collosion, self.resolution_width, self.resolution_height)
            self.pixel_output_array[pixel_x][pixel_y].append((income_photon.distance, income_photon.collosion))

    def generateDepthImage(self):
        for i in range(self.resolution_width):
            for j in range(self.resolution_height):
                # print(self.pixel_array[i][j])
                if self.pixel_array_count[i][j] != 0:
                    self.depthImage[i][j] = self.pixel_array[i][j] / self.pixel_array_count[i][j]
 
                else:
                    self.depthImage[i][j] = 0


        

    def output_to_file(self, file_name):
        # Ensure the file is created if it does not exist
        os.makedirs(os.path.dirname(file_name), exist_ok=True)

        with open(file_name, 'w') as file:
            file.write(f"{self.resolution_width}\n")
            file.write(f"{self.resolution_height}\n")

            for i in range(self.resolution_width):
                for j in range(self.resolution_height):
                    pixel_data = '['
                    self.pixel_output_array[i][j].sort(key=lambda x: x[0])

                    pixel_data += ",".join(f"({i},{j},{item[0]},{item[1]})" for item in self.pixel_output_array[i][j])
                    pixel_data += ']\n'
                    
                    file.write(pixel_data)  # Write each row instead of storing all in memory

# decode the file from the output_to_file function and get the depth information and the collosion information
def decode_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    resolution_width = int(lines[0])
    resolution_height = int(lines[1])
    pixel_output_array = [[[] for i in range(resolution_height)] for j in range(resolution_width)]
    line_index = 2
    for i in range(resolution_width):
        for j in range(resolution_height):
            line = lines[line_index]
            line = line[1:-2]
            line = line.split('),(')
            for item in line:
                item = item.split(',')
                if len(item) == 4:
                    pixel_output_array[i][j].append((float(item[2]), int(item[3])))
            line_index += 1
    return pixel_output_array

    


    





# open a file and read the content line by line

def read_file(file_name):
    """Read the content of the file and return lines as a list."""
    with open(file_name, "r") as file:
        return file.readlines()

def parse_line(pattern, line):
    """Helper function to extract data using regex."""
    match = re.search(pattern, line)
    return match.groups() if match else None

def read_raw_data(file_name):
    """Reads raw photon data from a file and returns a list of Ray objects."""
    lines = read_file(file_name)
    photons = []
    failed_lines = []
    line_index = 0

    # Regular expressions for parsing
    collison_pattern = r"\((\d+)\)\s*"
    distance_pattern = r"\{(\d+(\.\d+)?)\}\s*"
    vector_pattern = r"(-?\d+(?:\.\d+)?(?:[eE]-?\d+)?)\s+" \
                     r"(-?\d+(?:\.\d+)?(?:[eE]-?\d+)?)\s+" \
                     r"(-?\d+(?:\.\d+)?(?:[eE]-?\d+)?)"

    while line_index + 3 < len(lines):  # Ensure we have enough lines to process
        try:
            # Parse collision count
            collison_match = parse_line(collison_pattern, lines[line_index])
            if not collison_match:
                raise ValueError("Failed to parse collision count")
            collison = int(collison_match[0])

            # Parse distance
            line_index += 1
            distance_match = parse_line(distance_pattern, lines[line_index])
            if not distance_match:
                raise ValueError("Failed to parse distance")
            pdistance = float(distance_match[0])

            # Parse location (x, y, z)
            line_index += 1
            location_match = parse_line(vector_pattern, lines[line_index])
            if not location_match:
                raise ValueError("Failed to parse location")
            x, y, z = map(float, location_match)

            # Parse direction (dx, dy, dz)
            line_index += 1
            direction_match = parse_line(vector_pattern, lines[line_index])
            if not direction_match:
                raise ValueError("Failed to parse direction")
            dx, dy, dz = map(float, direction_match)

            if collison != 0:
                curPhoton = ray(x, y, z, dx, dy, dz, collison, pdistance,line_index)
                photons.append(curPhoton)
            

        except ValueError as e:
            failed_lines.append(line_index)
            print(f"Error parsing data at line {line_index + 1}: {e}")

        line_index += 1  # Move to the next set

    return photons, failed_lines


# mydetector.show_depth()
# mydetector.show_histogram()



if __name__ == "__main__":

    input_file_path = './positiveResult/rawData/test1.txt'
    # extract the name of the file generated a new file with the same name
    output_file_name = input_file_path.split('/')[-1]
    output_file_name = output_file_name.split('.')[0]
    output_file_name = output_file_name + '_len.txt'

    if (len(sys.argv) == 3):
        input_file_path = sys.argv[1]
        output_file_name = sys.argv[2]
    # read the file from parent directory

    mydetector = detector(0.01, 50, 500, 500)
    photons, failed_lines = read_raw_data(input_file_path)
    for photon in photons:
        mydetector.photon_to_dector(photon)
        
    print(output_file_name)




    print(mydetector.minDistance)
    print(mydetector.maxDistance)


    mydetector.output_to_file(output_file_name)
        
         
    



