#!/usr/bin/python

from optparse import OptionParser
import Image
import numpy as np
import matplotlib.pyplot as plt


def load_image(fname):
    """Load an image file"""
    return np.array(Image.open(fname))


def load_points(fname):
    """Load a text file with 2D points.

    The file should contain X and Y location of each point on a new line
    separated by wspace.
    """
    return np.loadtxt(fname)


def main():
    usage = "usage: %prog [options] image points \n\n"
    usage += "  Draw 2D points on an image.\n"
    parser = OptionParser(usage=usage)

    (options, args) = parser.parse_args()
    if len(args) != 2:
        print "Expected two input arguments"
        parser.print_usage()
        return

    img_file = args[0]
    pts_file = args[1]

    img = load_image(img_file)
    pts = load_points(pts_file)

    plt.imshow(img)
    plt.axis('off')
    plt.scatter(pts[:, 0], pts[:, 1], c='r')
    plt.show()


if __name__ == '__main__':
    main()
