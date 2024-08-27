from PIL import Image

path = 'C:/tmp/'

def convert_image(image_path):
    img = Image.open(image_path)
    img = img.convert('RGB')
    img = list(img.getdata())
    img = [img[offset:offset+20] for offset in range(0, len(img), 20)]
    
    return img


def main():
    for i in range(1, 60):
        img = convert_image(path + str(i).zfill(4) + '.png')
        with open("image.dat", "ab") as f:
            for row in img:
                for led in row:
                    hex_from_rgb = '{:02x}{:02x}{:02x}'.format(led[0], led[1], led[2])
                    f.write(bytes.fromhex(hex_from_rgb))

if __name__ == '__main__':
    main()