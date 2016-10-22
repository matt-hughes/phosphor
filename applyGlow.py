import scipy.ndimage
import scipy.signal
from matplotlib.pyplot import *
from numpy import *
import PIL

inFile = 'PetASCII4_mono.tga'
outFile = 'PetASCII4_mono_glow.tga'
cols,rows = 16,16
glowLevels = [(3,0.1,'gaussian'),(21,1.1,'gaussian')]

def mkKernel(kdesc):
    sz = kdesc[0]
    amt = kdesc[1]
    h = hanning(sz)
    k = outer(h,h)
    k = k*(amt/sum(k))
    k[sz/2,sz/2] = 1.
    return k

def convolveImgSet(imgs,kern):
    rows,cols = imgs.shape[0:2]
    imgH,imgW = imgs.shape[2:4]
    chans = imgs.shape[4]
    kernH,kernW = kern.shape[0:2]
    out = zeros((rows,cols) + (imgH+kernH-1,imgW+kernW-1) + (chans,))
    for row in xrange(rows):
        for col in xrange(cols):
            for chan in xrange(chans):
                out[row,col,:,:,chan] = scipy.signal.convolve2d(imgs[row,col,:,:,chan], kernel)
    return out

img = scipy.ndimage.imread(inFile)

glyphW = img.shape[1] / cols
glyphH = img.shape[0] / rows
channels = img.shape[2]

glyphs = zeros((rows,cols,glyphH,glyphW,channels))
for y in xrange(rows):
    for x in xrange(cols):
        glyphs[y,x] = img[glyphH*y:glyphH*(y+1),glyphW*x:glyphW*(x+1)] * (1./255)


for kdesc in glowLevels:
    kernel = mkKernel(kdesc)
    #kernel = kernel.reshape((1,1,)+kernel.shape+(1,))
    glyphs = convolveImgSet(glyphs, kernel)

#glyphs = clip(glyphs*255,0,255).astype('uint8')
glyphs = (glyphs*255./glyphs.max()).astype('uint8')

outGlyphH = glyphs.shape[2]
outGlyphW = glyphs.shape[3]
outImg = zeros((rows*outGlyphH,cols*outGlyphW,channels)).astype('uint8')
for y in xrange(rows):
    for x in xrange(cols):
        outImg[outGlyphH*y:outGlyphH*(y+1),outGlyphW*x:outGlyphW*(x+1)] = glyphs[y,x]

im = PIL.Image.fromarray(outImg)
im.save(outFile, 'TGA')
imshow(outImg)