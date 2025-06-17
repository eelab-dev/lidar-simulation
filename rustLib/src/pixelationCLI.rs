mod pixlelationLib;
use clap::Parser;
use std::path::PathBuf;
use std::fs;
use pixlelationLib::{Detector, read_file_parameter, read_raw_data};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Path to input .h5 file
    #[arg(long)]
    input_file: Option<PathBuf>,

    /// Optional output file name
    #[arg(short, long)]
    output_file: Option<PathBuf>,

    /// Field of view of the image formation
    #[arg(long)]
    fov: Option<f64>,

    /// Height of the image in pixels
    #[arg(long)]
    image_height: Option<usize>,

    /// Width of the image in pixels
    #[arg(long)]
    image_width: Option<usize>,
}

fn main() -> hdf5::Result<()> {
    let args = Args::parse();

    let mut input_file_path = PathBuf::from("./test_raw.h5");
    let mut fov = 50.0;
    let mut image_height = 500;
    let mut image_width = 500;

    if let Some(ref path) = args.input_file {
        input_file_path = path.clone();
        let (input_fov, input_height, input_width) = read_file_parameter(&input_file_path)?;
        fov = input_fov;
        image_height = input_height;
        image_width = input_width;
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
        PathBuf::from(format!("{}_len.h5", base))
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
        detector.photon_to_detector(photon);
    }
    detector.generate_depth_image();
    detector.output_to_file(output_file.clone())?;

    println!("\u{1F4CF} Min Distance: {:.3}", detector.min_distance);
    println!("\u{1F4CF} Max Distance: {:.3}", detector.max_distance);
    println!("\u{2705} Output File: {:?}", output_file);

    Ok(())
}
