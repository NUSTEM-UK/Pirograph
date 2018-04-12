#!/usr/bin/env python

"""Pirograph Image processing - multiprocessing test

"""

import io, time, sys
# import pygame
from PIL import Image, ImageStat, ImageOps, ImageDraw
import numpy as np
import os.path

from multiprocessing import Pool, current_process

# Set working frame size. Stick with multiples of 32:
# eg. 736, 800, 864, 896, 960, 1024, 1056.
# A good compromise for a 1080-line HD display is 854 px square.
size = width, height = 512, 512

# Default image processing settings
threshold_low = 40
threshold_high = 230

frame_count = 1

# Initialise PIL image to black background
#composite = Image.frombytes('RGB', size, "\x00" * width * height * 3)
#composite = composite.convert('RGBA')
#raw_str = composite.tobytes("raw", 'RGBA')

# Set up overlay mask image
# Oversize so it anti-aliases on scaledown
overmask_size = (width * 3, height * 3)
overmask_centre = [ overmask_size[0] / 2 , overmask_size[1] / 2 ]
overmask_radius = overmask_size[0] / 2

def drawOvermask():
    global overmask
    global overmask_size
    global overmask_radius
    global overmask_centre
    overmask = Image.new('L', overmask_size, 0)
    draw = ImageDraw.Draw(overmask)
    draw.ellipse (  (
        (overmask_centre[0] - overmask_radius),
        (overmask_centre[1] - overmask_radius),
        (overmask_centre[0] + overmask_radius),
        (overmask_centre[1] + overmask_radius) ), fill = 255)
    overmask = overmask.resize(size, Image.ANTIALIAS)

# Set up mask image
drawOvermask()

# Note that array size arguments are the other way around to the camera resolution. Just to catch you out.
# rawCapture = PiRGBArray(camera, size=(height, width))
#thisFrame1 = Image.open('test_image.jpeg')
#rawCapture = PiYUVArray(camera, size=(height, width))

time_begin = time.time()
time_start = time.time()
frame_count = 1

def process_frame(frame):
    current = current_process()
    time_startframe = time.time()
    frame_rgb_image = Image.new('RGBA', frame.size)
    frame_rgb_image.paste(frame)

    # BEGIN Image processing code 
    
    # Create YUV conversion for luminosity mask processing
    frame_y = np.array(frame_rgb_image.convert("YCbCr"))[0:width, 0:height, 0]

    # ***** MASK PROCESSING *****
    # Clip low values to black (transparent)
    # First index the low values...
    low_clip_indices = frame_y < threshold_low
    # ...then set values at those indices to zero
    frame_y[low_clip_indices] = 0

    # Clip high values to white (solid)
    # First index the high values...
    high_clip_indices = frame_y > threshold_high
    # ...then set values at those indices to 255
    frame_y[high_clip_indices] = 255
    
    # Make mask image from Numpy array frame_y
    mask = Image.fromarray(frame_y, "L")
    
    # ***** COMPOSITE NEW FRAME *****
    # Convert captured frame to RGBA
    frame_rgb_image = frame_rgb_image.convert("RGBA")

    
    print("Thread %s complete in: %s" %(current.name, time.time()-time_startframe))
    return mask

x = 0
compImage = Image.new('RGBA', size)

thisFrame1 = Image.open('test_image.jpeg')
thisFrame2 = Image.open('test_image.jpeg')
thisFrame3 = Image.open('test_image.jpeg')
thisFrame4 = Image.open('test_image.jpeg')
thisFrame5 = Image.open('test_image.jpeg')
frames = [thisFrame1, thisFrame1, thisFrame1, thisFrame1, thisFrame1,
          thisFrame2, thisFrame2, thisFrame2, thisFrame2, thisFrame2,
          thisFrame4, thisFrame3, thisFrame3, thisFrame3, thisFrame3,
          thisFrame3, thisFrame4, thisFrame4, thisFrame4, thisFrame4,
          thisFrame5, thisFrame5, thisFrame5, thisFrame5, thisFrame5]

if __name__ == '__main__':
    print("Starting...")
    time_start = time.time()
    pool = Pool(processes=4)
    outputs = pool.map(process_frame, frames)
    # pool.close()
    print("complete in: %s" %(time.time() - time_start))
