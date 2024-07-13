from PIL import Image
import pickle

path = 'C:/tmp/'

def png_to_rgb_array(image_path):
    img = Image.open(image_path)
    
    if img.size != (20, 20):
        raise ValueError("The image must be 20x20 pixels")
    
    pixels = list(img.getdata())
    
    color_array = [pixels[i:i + 20] for i in range(0, len(pixels), 20)]
    
    return [color for row in color_array for color in row]

for i in range(1, 61):
        img = png_to_rgb_array(path + str(i).zfill(4) + '.png')
        tab = []
        for pix in img:
            (r,g,b,a) = pix
            if r <= 1 and g <= 1 and b <= 1:
                r,g,b,a = 0, 0, 0, 0
            tab.append((r/255, g/255, b/255))
        img = [(round(r/255, 2), round(g/255, 2), round(b/255, 2)) for (r,g,b,a) in img]
        with open("out/" + str(i-1) + ".dat", "wb") as f:
            f.write(pickle.dumps(img))