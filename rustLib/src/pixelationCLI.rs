mod pixlelationLib;
use clap::Parser;
use pixlelationLib::{read_file_parameter, read_raw_data, Detector};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
pub struct Args {
    #[arg(long = "input_file", help = "Path to input .h5 file")]
    pub input_file: String,

    #[arg(short = 'o', long = "output_file", help = "Output .h5 file name")]
    pub output_file: Option<String>,

    #[arg(long = "fov", help = "Field of view")]
    pub fov: Option<f64>,

    #[arg(long = "fov_x", help = "Horizontal field of view")]
    pub fov_x: Option<f64>,

    #[arg(long = "fov_y", help = "Vertical field of view")]
    pub fov_y: Option<f64>,

    #[arg(long = "image_width", help = "Width of the image in pixels")]
    pub image_width: Option<usize>,

    #[arg(long = "image_height", help = "Height of the image in pixels")]
    pub image_height: Option<usize>,
}

fn main() -> hdf5::Result<()> {
    let args = Args::parse();

    let mut input_file_path = PathBuf::from("./test_raw.h5");
    let mut fov_x = 50.0;
    let mut fov_y = 50.0;
    let mut image_height = 500;
    let mut image_width = 500;

    // Override input file if provided
    if !args.input_file.is_empty() {
        input_file_path = PathBuf::from(&args.input_file);

        // Try to extract parameters from the file
        if let Ok((input_fov_x, input_fov_y, input_h, input_w)) =
            read_file_parameter(&input_file_path)
        {
            fov_x = input_fov_x;
            fov_y = input_fov_y;
            image_height = input_h;
            image_width = input_w;
        }
    }

    if let Some(val) = args.fov {
        fov_x = val;
        fov_y = val;
    }
    if let Some(val) = args.fov_x {
        fov_x = val;
    }
    if let Some(val) = args.fov_y {
        fov_y = val;
    }
    if let Some(val) = args.image_height {
        image_height = val;
    }
    if let Some(val) = args.image_width {
        image_width = val;
    }

    let output_file = args.output_file.unwrap_or_else(|| {
        let base = input_file_path
            .file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("output");
        format!("{}_len.h5", base)
    });

    let mut detector = Detector::new(0.01, fov_x, fov_y, image_width, image_height);

    let (photons, failed_lines) = read_raw_data(&input_file_path);

    if !failed_lines.is_empty() {
        eprintln!("⚠️ {} lines failed to parse", failed_lines.len());
    }

    for photon in &photons {
        detector.flashLidar_photon_to_detector(photon);
    }
    detector.generate_depth_image();
    detector.output_to_file(output_file.clone())?;

    println!("\u{1F4CF} Min Distance: {:.3}", detector.min_distance);
    println!("\u{1F4CF} Max Distance: {:.3}", detector.max_distance);
    println!("\u{2705} Output File: {:?}", output_file);

    Ok(())
}
