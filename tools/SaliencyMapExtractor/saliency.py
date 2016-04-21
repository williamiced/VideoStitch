#-------------------------------------------------------------------------------
# Name:        main
# Purpose:     Testing the package pySaliencyMap
#
# Author:      Akisato Kimura <akisato@ieee.org>
#
# Created:     May 4, 2014
# Copyright:   (c) Akisato Kimura 2014-
# Licence:     All rights reserved
#-------------------------------------------------------------------------------

import cv2
import pySaliencyMap
import scipy.misc

# main
if __name__ == '__main__':

	cap = cv2.VideoCapture('results/pano_960_480.avi')

	counter = 0	

	while (cap.isOpened()):
		ret, img = cap.read()
		imgsize = img.shape
		img_width = imgsize[1]
		img_height = imgsize[0]
		sm = pySaliencyMap.pySaliencyMap(img_width, img_height)
		map = sm.SMGetSM(img)
		#cv2.imwrite("img/out_" + str(counter) + ".png", img)
		filename = 'img/out_' + str(counter) + '.png'
		#scipy.misc.toimage(map, cmin=0.0, cmax=999).save(filename)
		scipy.misc.imsave(filename, map)
		#cv2.imshow("output", map)
		#cv2.waitKey(0)
		#cv2.destroyAllWindows()
		counter += 1
		print "Counter: " + str(counter)

	print "Done"	
