#!/usr/bin/env python

"""Pirogrpah Image processing

Implemented loading an image from disk, to avoid camera faffing while we try
to benchmark and performance-tune the image processing.
"""

import io, time, sys
import pygame

from PIL import Image, ImageStat, ImageOps, ImageDraw
import numpy as np
import os.path

# Set working frame size. Stick with multiples of 32:
# eg. 736, 800, 864, 896, 960, 1024, 1056.
# A good compromise for a 1080-line HD display is 854 px square.
size = width, height = 736, 736

# Set up some configuration variables
video_framerate = 8
# Default image processing settings
threshold_low = 40
threshold_high = 230

frame_count = 1

# Initialise PyGame surface
pygame.init()
screen = pygame.display.set_mode(size)
screen.fill((0, 0, 0))
pygame.display.flip()

# Initialise PIL image to black background
composite = Image.frombytes('RGB', size, "\x00" * width * height * 3)
composite = composite.convert('RGBA')
raw_str = composite.tobytes("raw", 'RGBA')
pygame.surface = pygame.image.fromstring(raw_str, size, 'RGBA')

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


def get_brightness(image):
    """Return overall brightness value for image"""
    stat = ImageStat.Stat(image)
    return stat.rms[0]


def handlePygameEvents():
    global threshold_low
    global threshold_high
    global composite
    global frame_count
    global overmask_size
    global overmask_centre
    global overmask_radius
    for event in pygame.event.get():
        if event.type == pygame.QUIT: sys.exit()
        elif event.type is pygame.KEYDOWN:
            key_press = event.key
            # key_press = pygame.key.name(event.key)
            # print key_press # For diagnostic purposes, but messes up output
            if key_press == pygame.K_e:
                if (threshold_low + 1) < 256:
                    threshold_low += 1
                print("threshold_low set to %i" % threshold_low)
            elif key_press == pygame.K_d:
                if (threshold_low - 1) >= 0:
                    threshold_low -= 1
                print("threshold_low set to %i" % threshold_low)
            elif key_press == pygame.K_r:
                if (threshold_high + 1) < 256:
                    threshold_high += 1
                print("threshold_high set to %i" % threshold_high)
            elif key_press == pygame.K_f:
                if (threshold_high -1) >= 0:
                    threshold_high -= 1
                print("threshold_high set to %i" % threshold_high)
            
            # Check for left shift and allow rapid threshold changes
            if pygame.key.get_mods() & pygame.KMOD_LSHIFT:
                if key_press == pygame.K_q:
                    sys.exit()
                if key_press == pygame.K_e:
                    if (threshold_low + 10) < 256:
                        threshold_low += 10
                    print("threshold_low set to %i" % threshold_low)
                elif key_press == pygame.K_d:
                    if (threshold_low - 10) >= 0:
                        threshold_low -= 10
                    print("threshold_low set to %i" % threshold_low)
                elif key_press == pygame.K_r:
                    if (threshold_high + 10) < 256:
                        threshold_high += 10
                    print("threshold_high set to %i" % threshold_high)
                elif key_press == pygame.K_f:
                    if (threshold_high -10) >= 0:
                        threshold_high -= 10
                    print("threshold_high set to %i" % threshold_high)
                # Check for SHIFT+P and if found, set working image to pure black again
                elif key_press == pygame.K_p:
                    print("*** STARTING OVER ***")
                    composite = Image.frombytes('RGB', size, "\x00" * width * height * 3)
                    composite = composite.convert('RGBA')

# Set up mask image
drawOvermask()

# Note that array size arguments are the other way around to the camera resolution. Just to catch you out.
# rawCapture = PiRGBArray(camera, size=(height, width))
thisFrame = Image.open('test_image.jpeg')
#rawCapture = PiYUVArray(camera, size=(height, width))

time_begin = time.time()
time_start = time.time()
frame_count = 1

# Work through the stream of images from the camera

x = 0
while x in range(10):    
    # frame_new = Image.frombytes('RGB', size, frame.array)
    # frame_yuv = Image.frombytes('yuv', size, frame.array)
    #frame_rgb_array = frame.array
    #frame_rgb_image = Image.fromarray(frame_rgb_array)
    frame_rgb_image = Image.new('RGBA', size)
    frame_rgb_image.paste(thisFrame)

    # BEGIN Image processing code 
    
    # Create YUV conversion for luminosity mask processing
    #frame_yuv = frame_rgb_image.convert("YCbCr")
    #frame_yuv_array = np.array(frame_yuv)
    #frame_yuv_array = np.array(frame_rgb_image.convert("YCbCr"))
    #frame_y = frame_yuv_array[0:width, 0:height, 0]
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
    
    # Combine captured frame with rolling composite, via computed mask
    # TODO: Check this is really doing what we think it is
    composite.paste(frame_rgb_image, (0,0), mask)
    
    # Apply overlay mask
    composite.paste(overmask, (0,0), ImageOps.invert(overmask))
    
    # ***** DISPLAY NEW FRAME *****    
    raw_str = composite.tobytes("raw", 'RGBA')
    pygame_surface = pygame.image.fromstring(raw_str, size, 'RGBA')
    
    # Finally, update the window
    screen.blit(pygame_surface, (0,0))
    pygame.display.flip()

    # Handle PyGame events (ie. keypress controls)
    handlePygameEvents()

    time_taken = time.time() - time_start
    time_since_begin = time.time() - time_begin
    print("Frame %d in %.3f secs, at %.2f fps, Low: %d High: %d" % (frame_count, time_taken, (frame_count/time_since_begin), threshold_low, threshold_high))
    
    time_start = time.time()
    frame_count += 1
