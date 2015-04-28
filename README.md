# SeniorDesignProject
UCD EEC 181AB Winter/Spring 2015 Senior Design Project
Eduard Alfonso
Jack Chen
Jimmy Yu


Using an Altera DE1-SoC FPGA, we try to create a Hardware-Software camera system 
that will capture images with handwritten digits. We will process these images by
extracting the region-of-interest (ROI), segmenting the ROI into digits, and put
each digit through a neural network where it will guess the most probable digit.

Our goal is to speedup our system to process an image in under 100 ms so we can
process an average of 10 frames-per-second.
