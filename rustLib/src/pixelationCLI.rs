mod pixlelationLib;
use clap::Parser;
use std::path::PathBuf;
use std::fs;
use pixlelationLib::{Detector, read_file_parameter, read_raw_data};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
pub struct Args {
    #[arg(long = "input_file", help = "Path to input .h5 file")]
    pub input_file: String,

    #[arg(short = 'o', long = "output_file", help = "Output .h5 file name")]
    pub output_file: Option<String>,

    #[arg(long = "fov", help = "Field of view")]
    pub fov: Option<f64>,

    #[arg(long = "image_width", help = "Width of the image in pixels")]
    pub image_width: Option<usize>,

    #[arg(long = "image_height", help = "Height of the image in pixels")]
    pub image_height: Option<usize>,
}

fn main() -> hdf5::Result<()> {
    let args = Args::parse();

    let mut input_file_path = PathBuf::from("./test_raw.h5");
    let mut fov = 50.0;
    let mut image_height = 500;
    let mut image_width = 500;

    // Override input file if provided
    if !args.input_file.is_empty() {
        input_file_path = PathBuf::from(&args.input_file);

        // Try to extract parameters from the file
        if let Ok((input_fov, input_h, input_w)) = read_file_parameter(&input_file_path) {
            fov = input_fov;
            image_height = input_h;
            image_width = input_w;
        }
    }

    if let Some(val) = args.fov {
        fov = val;
    }
    if let Some(val) = args.image_height {
        image_height = val;
    }
    if let Some(val) = args.image_width {
        image_width = val;
    }

    let output_file = args.output_file.unwrap_or_else(|| {
        let base = input_file_path.file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("output");
            format!("{}_len.h5", base)
    });

    let mut detector = Detector::new(0.01, fov, image_width, image_height);

    // Instead of this (which errors)
    let photons = read_raw_data(&input_file_path);

    // Do this:
    let (photons, failed_lines) = read_raw_data(&input_file_path);

    if !failed_lines.is_empty() {
        eprintln!("⚠️ {} lines failed to parse", failed_lines.len());
    }


    for photon in &photons {
        detector.scanLidar_photon_to_detector(photon);
    }
    detector.generate_depth_image();
    detector.output_to_file(output_file.clone())?;

    println!("\u{1F4CF} Min Distance: {:.3}", detector.min_distance);
    println!("\u{1F4CF} Max Distance: {:.3}", detector.max_distance);
    println!("\u{2705} Output File: {:?}", output_file);

    Ok(())
}
