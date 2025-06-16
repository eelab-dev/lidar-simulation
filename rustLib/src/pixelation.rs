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
    pub line_index: usize,
}

impl Ray {
    pub fn new(x: f64, y: f64, z: f64,
               dx: f64, dy: f64, dz: f64,
               collision: i32, distance: f64, line_index: usize) -> Self {
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
            line_index,
        }
    }
}


#[derive(H5Type, Clone, Debug)]
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
    pub pixel_output_array: Vec<Vec<Vec<(f32, i32)>>>, // (distance, collision)

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

    pub fn photon_to_detector(&mut self, photon: &Ray) {
        if photon.collision == 0 {
            return;
        }

        let t = self.focal_length.abs() / photon.dz;
        let mut x = t * photon.dx;
        let mut y = t * photon.dy;

        std::mem::swap(&mut x, &mut y);
        std::mem::swap(&mut self.origin_x, &mut self.origin_y);

        let pixel_x = ((x - self.origin_x) / self.width_resolution) as isize;
        let pixel_y = (self.resolution_height as isize - ((y - self.origin_y) / self.height_resolution) as isize);

        if pixel_x >= 0 && pixel_x < self.resolution_width as isize && pixel_y >= 0 && pixel_y < self.resolution_height as isize {
            let x = pixel_x as usize;
            let y = pixel_y as usize;

            self.pixel_array_count[(x, y)] += 1;
            self.pixel_array[(x, y)] += photon.distance;

            self.pixel_output_array[x][y].push((photon.distance as f32, photon.collision));
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

        let mut data: Vec<Vec<VarLenArray<PhotonRecord>>> = Vec::with_capacity(self.resolution_width);
        for x in 0..self.resolution_width {
            let mut row = Vec::with_capacity(self.resolution_height);
            for y in 0..self.resolution_height {
                let photons = &self.pixel_output_array[x][y];
                let varray: VarLenArray<PhotonRecord> = VarLenArray::from_slice(&photons);
                row.push(varray);
            }
            data.push(row);
        }

        let dataset = file.new_dataset::<VarLenArray<PhotonRecord>>()
            .shape(shape)
            .create("photon_data")?;

        for x in 0..self.resolution_width {
            for y in 0..self.resolution_height {
                dataset.write_slice(&data[x][y], (x, y))?;
            }
        }

        dataset.new_attr::<u32>().create("width")?.write_scalar(&(self.resolution_width as u32))?;
        dataset.new_attr::<u32>().create("height")?.write_scalar(&(self.resolution_height as u32))?;

        Ok(())
    }
}