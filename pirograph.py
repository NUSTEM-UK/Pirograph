"""Pirograph image processing.

Pygame implementation, based heavily on earlier Technology Wishing Well
code and Mike Cook's Kaleido Cam from the Mag Pi, December 2017.

Note that this code will only work on V4L2 systems: ie. Linux and Raspberry Pi.
Sorry, Mac and Windows users.
"""

import pygame, pygame.camera
import os, time
from tkinter import Tk
from tkinter.filedialog import asksaveasfilename
from PIL import Image, ImageStat, ImageOps, ImageDraw

# os.system("sydo modprobe bcm2835-v4l2") # needed for Pi camera
Tk().withdraw()
pygame.init()
pygame.camera.init()
os.environ['SDL_VIDEO_WINDOW_POS'] = 'center'
pygame.display.set_caption("Pirograph")
pygame.event.set_allowed(None)
pygame.event.set_allowed([pygame.KEYDOWN, pygame.QUIT])

imagesize = 800 # basic image size.
screen = pygame.display.set_mode([imagesize, imagesize], 0, 32)

# find, open and start camera
cam_list = pygame.camera.list_cameras()
print(cam_list)
webcam = pygame.camera.Camera(cam_list[0], (1920, 1080))
webcam.start()

preRot = 0.0
autoRotate = False
savePath = ""
frameNumber = 0
saveSource = False

# Config variables (can adapt at runtime)
full_screen = 0
video_framerate = 0
threshold_low = 40
threshold_high = 230
frame_count = 1


def main():
    while True:
        checkForEvent()
        showScreen()

def showScreen():
    global camFrame, preRot
    camFrame = webcam.get_image()
    frame_count += 1
    if autoRotate:
        preRot += 0.5
        if preRot > 360:
            preRot -= 360
        rotFrame = pygame.transform.scale(camFrame, (imagesize, imagesize)) # ensure square
        rotFrame = rot_center(rotFrame, preRot) # Rotate
        sqFrame = pygame.Surface((imagesize, imagesize))
        sqFrame.blit(rotFrame, (0, 0))
    else:
        sqFrame = pygame.transform.scale(camFrame, (imagesize, imagesize))
    screen.blit(sqFrame, (0, 0))
    pygame.display.update()


def rot_center(image, angle):
    # rotate an image while keeping its center and size
    orig_rect = image.get_rect()
    rot_image = pygame.transform.rotate(image, angle)
    rot_rect = orig_rect.copy()
    rot_rect.center = rot_image.get_rect().center
    rot_image = rot_image.subsurface(rot_rect).copy()
    return rot_image


def terminate():
    webcam.stop()
    pygame.quit()
    os._exit(1)


def greyscale(self, img):
    """See https://stackoverflow.com/questions/10261440/how-can-i-make-a-greyscale-copy-of-a-surface-in-pygame/10693616#10693616."""
    arr = pygame.surfarray.pixels3d(img)
    avgs = [[(r*0.298+ g*0.587 + b*0.114) for (r, g, b) in col] for col in arr]
    arr = arr.dot([0.298, 0.587, 0.114])[:,:,None].repeat(3, axis=2)
    return pygame.surfarray.make_surface(arr)


def checkForEvent():
    global savePath, autoRotate, saveSource, preRot
    event = pygame.event.poll()
    if event.type == pygame.QUIT:
        terminate()
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
            terminate()
        if event.key == pygame.K_r:
            autoRotate = not autoRotate
            print("Autorotate: ", autoRotate)
            if autoRotate:
                preRot = 0


if __name__ == '__main__':
    main()