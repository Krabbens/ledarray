from PIL import Image, ImageChops
import os, struct

path = '/home/kuose/render/'

files = [f for f in os.listdir(path) if f.endswith(('png', 'jpg', 'jpeg'))]
#files.sort()

diff = bytes()

previous_image = None

def convert_to_palette(pixel):
    return pixel[0]

for file in files:
    current_image = Image.open(os.path.join(path, file))
    
    if previous_image is not None:
        for index, image in enumerate(zip(previous_image.getdata(), current_image.getdata())):
            if convert_to_palette(image[0]) != convert_to_palette(image[1]):
                diff += struct.pack('hB', index, convert_to_palette(image[1]))

        print(f'Compared {file} with {previous_image.filename}')
    
    previous_image = current_image

with open('diff.dat', 'wb') as f:
    f.write(diff)