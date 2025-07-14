use ndarray::Array2;
use hdf5::{File, types::VarLenArray, H5Type};
use std::f64;
use std::path::Path;

#[derive(Debug, Clone)]
pub struct Ray {
    pub x: f64,
    pub y: f64,
    pub z: f64,
    pub dx: f64,
    pub dy: f64,
    pub dz: f64,
    pub direction_length: f64,
    pub distance: f64,
    pub collision: i32,
    pub camera_x: i32,
    pub camera_y: i32,
    pub line_index: usize,
}

impl Ray {
    pub fn new(x: f64, y: f64, z: f64,
               dx: f64, dy: f64, dz: f64,
               collision: i32, distance: f64, camera_x:i32, camera_y:i32, line_index: usize) -> Self {
        let length = (dx*dx + dy*dy + dz*dz).sqrt();
        Self {
            x,
            y,
            z,
            dx: dx / length,
            dy: dy / length,
            dz: dz / length,
            direction_length: length,
            distance,
            collision,
            camera_x,
            camera_y,
            line_index,
        }
    }
}

#[derive(H5Type, Clone, Debug, Copy)]
#[repr(C)]
pub struct PhotonRecord {
    pub distance: f32,
    pub collision_count: i32,
}

#[derive(Debug)]
pub struct Detector {
    pub focal_length: f64,
    pub field_of_view: f64,
    pub resolution_width: usize,
    pub resolution_height: usize,

    pub pixel_array_count: Array2<u32>,
    pub pixel_array: Array2<f64>,
    pub depth_image: Array2<f64>,
    pub pixel_output_array: Vec<Vec<Vec<(f32, i32)>>>,

    pub detector_width: f64,
    pub detector_height: f64,
    pub origin_x: f64,
    pub origin_y: f64,
    pub width_resolution: f64,
    pub height_resolution: f64,

    pub min_distance: f64,
    pub max_distance: f64,
}

impl Detector {
    pub fn new(focal_length: f64, fov: f64, width: usize, height: usize) -> Self {
        let angle_rad = (fov / 2.0).to_radians();
        let detector_height = focal_length * (angle_rad.tan()) * 2.0;
        let detector_width = detector_height * (width as f64 / height as f64);
        let origin_x = -detector_width / 2.0;
        let origin_y = -detector_height / 2.0;
        let width_resolution = detector_width / width as f64;
        let height_resolution = detector_height / height as f64;

        let pixel_array_count = Array2::<u32>::zeros((width, height));
        let pixel_array = Array2::<f64>::zeros((width, height));
        let depth_image = Array2::<f64>::zeros((width, height));
        let pixel_output_array = vec![vec![vec![]; height]; width];

        Self {
            focal_length,
            field_of_view: fov,
            resolution_width: width,
            resolution_height: height,

            pixel_array_count,
            pixel_array,
            depth_image,
            pixel_output_array,

            detector_width,
            detector_height,
            origin_x,
            origin_y,
            width_resolution,
            height_resolution,

            min_distance: f64::INFINITY,
            max_distance: 0.0,
        }
    }

    pub fn flashLidar_photon_to_detector(&mut self, photon: &Ray) {
        if photon.collision == 0 {
            return;
        }

        if (photon.dz > 0.0)
        {
            return;
        }

        let t = (self.focal_length / photon.dz).abs();
        let mut x = t * photon.dx;
        let mut y = t * photon.dy;

        let pixel_x = ((x - self.origin_x) / self.width_resolution) as isize;
        let pixel_y =  ((y - self.origin_y) / self.height_resolution) as isize;

        if pixel_x >= 0 && pixel_x < self.resolution_width  as isize && pixel_y >= 0 && pixel_y < self.resolution_height as isize {
            let x = pixel_x as usize;
            let y = pixel_y as usize;

            self.pixel_array_count[(x, y)] += 1;
            self.pixel_array[(x, y)] += photon.distance;

            self.pixel_output_array[x][y].push((photon.distance as f32, photon.collision));
            self.min_distance = self.min_distance.min(photon.distance);
            self.max_distance = self.max_distance.max(photon.distance);
        }

    }

    pub fn scanLidar_photon_to_detector(&mut self, photon: &Ray) {
            if photon.collision == 0 {
                return;
            }

            if (photon.dz > 0.0)
            {
                return;
            }

            let x = photon.camera_x;
            let y = photon.camera_y;

            if x >= 0 && y >= 0 &&
            x < self.resolution_width as i32 && y < self.resolution_height as i32 {

                let index_x = x as usize;
                let index_y = y as usize;

                self.pixel_array_count[(index_x, index_y)] += 1;
                self.pixel_array[(index_x, index_y)] += photon.distance;
                self.pixel_output_array[index_x][index_y].push((photon.distance as f32, photon.collision));

                self.min_distance = self.min_distance.min(photon.distance);
                self.max_distance = self.max_distance.max(photon.distance);
            }
    }

    




    pub fn generate_depth_image(&mut self) {
        for x in 0..self.resolution_width {
            for y in 0..self.resolution_height {
                let count = self.pixel_array_count[(x, y)];
                self.depth_image[(x, y)] = if count > 0 {
                    self.pixel_array[(x, y)] / count as f64
                } else {
                    0.0
                };
            }
        }
    }

    pub fn output_to_file<P: AsRef<Path>>(&self, filename: P) -> hdf5::Result<()> {
        let file = File::create(filename)?;
        let shape = (self.resolution_width, self.resolution_height);

        // Initialize Array2 of VarLenArray<PhotonRecord>
        let mut data: Array2<VarLenArray<PhotonRecord>> = Array2::from_shape_simple_fn(shape, || {
            VarLenArray::from_slice(&[])
        });

        // Fill the array with actual data
        for x in 0..self.resolution_width {
            for y in 0..self.resolution_height {
                let records: Vec<PhotonRecord> = self.pixel_output_array[x][y]
                    .iter()
                    .map(|(d, c)| PhotonRecord {
                        distance: *d,
                        collision_count: *c,
                    })
                    .collect();
                data[(x, y)] = VarLenArray::from_slice(&records);
            }
        }

        // Create the dataset and write the full 2D array
        let dataset = file
            .new_dataset::<VarLenArray<PhotonRecord>>()
            .shape(shape)
            .create("photon_data")?;

        dataset.write(&data)?;

        // Add metadata attributes
        file.new_attr::<u32>().create("width")?.write_scalar(&(self.resolution_width as u32))?;
        file.new_attr::<u32>().create("height")?.write_scalar(&(self.resolution_height as u32))?;
        Ok(())
    }
}

#[derive(Debug)]
pub struct FailedRayRecord {
    pub index: usize,
    pub error: String,
}


#[derive(H5Type, Clone, Debug)]
#[repr(C)]
pub struct CollisionRecord {
    pub CollisionCount: i32,
    pub Distance: f64,
    pub CollisionLocation: [f64; 3],
    pub CollisionDirection: [f64; 3],
    pub Camera_x: i32,
    pub Camera_y: i32,
}

pub fn read_raw_data<P: AsRef<std::path::Path>>(file_name: P) -> (Vec<Ray>, Vec<FailedRayRecord>) {
    let mut photons = Vec::new();
    let mut failed_lines = Vec::new();

    let file = match File::open(&file_name) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Error opening HDF5 file: {}", e);
            return (photons, failed_lines);
        }
    };

    let dataset = match file.dataset("CollisionData") {
        Ok(ds) => ds,
        Err(e) => {
            eprintln!("Error accessing CollisionData dataset: {}", e);
            return (photons, failed_lines);
        }
    };

    let records: Vec<CollisionRecord> = match dataset.read_raw::<CollisionRecord>() {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Error reading CollisionData as compound struct: {}", e);
            return (photons, failed_lines);
        }
    };

    for (i, rec) in records.iter().enumerate() {
        if rec.CollisionCount != 0 {
            match std::panic::catch_unwind(|| {
                Ray::new(
                    rec.CollisionLocation[0],
                    rec.CollisionLocation[1],
                    rec.CollisionLocation[2],
                    rec.CollisionDirection[0],
                    rec.CollisionDirection[1],
                    rec.CollisionDirection[2],
                    rec.CollisionCount,
                    rec.Distance,
                    rec.Camera_x,
                    rec.Camera_y,
                    i,
                )
            }) {
                Ok(ray) => photons.push(ray),
                Err(_) => failed_lines.push(FailedRayRecord {
                    index: i,
                    error: "Panic during Ray creation".to_string(),
                }),
            }
        }
    }

    (photons, failed_lines)
}

pub fn read_file_parameter<P: AsRef<std::path::Path>>(file_name: P) -> hdf5::Result<(f64, usize, usize)> {
    let file = File::open(file_name)?;
    let fov = file.attr("FOV")?.read_scalar::<f64>()?;
    let height = file.attr("ImageHeight")?.read_scalar::<u32>()? as usize;
    let width = file.attr("ImageWidth")?.read_scalar::<u32>()? as usize;
    Ok((fov, height, width))
}