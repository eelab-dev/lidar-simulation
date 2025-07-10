
import numpy as np
import os
import h5py

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

        if income_photon.collosion == 0:
            return
        # calculate the intersection point

        if (income_photon.dz > 0):
            return 
        t = np.abs(self.focal_length / income_photon.dz)
        x = t * income_photon.dx
        y = t * income_photon.dy
        # calculate the pixel  
        pixel_x =  int((x - self.origin_x) / self.widthResolution)
        pixel_y =  int((y - self.origin_y) / self.heightResolution)
        # pixel_x,pixel_y = pixel_y,pixel_x
        # pixel_y = int((y - self.origin_y) / self.heightResolution)

        if pixel_x >= 0 and pixel_x < self.resolution_width and pixel_y >= 0 and pixel_y < self.resolution_height:
            self.pixel_array_count[pixel_x][pixel_y] += 1
            self.pixel_array[pixel_x][pixel_y] += income_photon.distance
            # print(pixel_x, pixel_y, income_photon.distance, income_photon.collosion, self.resolution_width, self.resolution_height)
            self.pixel_output_array[pixel_x][pixel_y].append((income_photon.distance, income_photon.collosion))
            self.minDistance = min(self.minDistance, income_photon.distance)
            self.maxDistance = max(self.maxDistance, income_photon.distance)
            
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
        # Define the compound dtype for a photon: (distance, collision_count)
        photon_dtype = np.dtype([('distance', np.float32), ('collision_count', np.int32)])

        # Create a variable-length array of photons
        vlen_dtype = h5py.vlen_dtype(photon_dtype)

        # Create a 2D array (width x height) of vlen photon arrays
        data = np.empty((self.resolution_width, self.resolution_height), dtype=object)

        for i in range(self.resolution_width):
            for j in range(self.resolution_height):
                # photons = sorted(self.pixel_output_array[i][j], key=lambda x: x[0])
                photons = self.pixel_output_array[i][j]
                photon_array = np.array(photons, dtype=photon_dtype)
                data[i, j] = photon_array

        # Save to HDF5
        with h5py.File(file_name, "w") as h5file:
            h5file.create_dataset("photon_data", data=data, dtype=vlen_dtype)
            h5file.attrs["width"] = self.resolution_width
            h5file.attrs["height"] = self.resolution_height       



    def output_to_array(self):
        photon_dtype = np.dtype([('distance', np.float32), ('collision_count', np.int32)])
        # Create a 2D array (width x height) of vlen photon arrays
        data = np.empty((self.resolution_width, self.resolution_height), dtype=object)

        for i in range(self.resolution_width):
            for j in range(self.resolution_height):
                # photons = sorted(self.pixel_output_array[i][j], key=lambda x: x[0])
                photons = self.pixel_output_array[i][j]
                photon_array = np.array(photons, dtype=photon_dtype)
                data[i, j] = photon_array
        return data, self.resolution_width, self.resolution_height 

def read_raw_data(file_name):
    """Reads raw photon data from an HDF5 file and returns a list of Ray objects."""
    photons = []
    failed_lines = []

    try:
        with h5py.File(file_name, 'r') as h5file:
            # Check if dataset exists
            if "CollisionData" not in h5file:
                raise ValueError("Dataset 'CollisionData' is missing in the HDF5 file")

            dataset = h5file["CollisionData"]

            # Read structured data
            collision_counts = dataset["CollisionCount"][:]
            distances = dataset["Distance"][:]
            collision_locations = dataset["CollisionLocation"][:]
            collision_directions = dataset["CollisionDirection"][:]

            # Iterate and create Ray objects
            for i, (count, dist, loc, dir_) in enumerate(zip(collision_counts, distances, collision_locations, collision_directions)):
                try:
                    if count != 0:  # Create Ray object only if collision count is non-zero
                        Ray = ray(loc[0], loc[1], loc[2], dir_[0], dir_[1], dir_[2], count, dist, i)
                        photons.append(Ray)
                
                except Exception as e:
                    failed_lines.append(i)
                    print(f"Error processing record {i}: {e}")

    except Exception as e:
        print(f"Error opening HDF5 file: {e}")

    return photons, failed_lines
 
def read_file_parameter(h5_filename):
    with h5py.File(h5_filename, 'r') as f:
        height = int(f.attrs['ImageHeight'])
        width = int(f.attrs['ImageWidth'])
        fov = float(f.attrs['FOV'])

    return fov, height, width