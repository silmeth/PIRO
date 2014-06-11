import numpy as np
import cv2

folder_path = "/home/jacek/Studia/PIRO/face_rec_test/img/"
triagle_f_name = "triangle.png"

def get_biggest_contour(contours):
    area = -1
    if contours:
        for cnt in contours:
            if cv2.contourArea(cnt) > area:
                biggest_cnt = cnt
            return [biggest_cnt]
        else:
            return []
    else:
        return []

triangle = cv2.imread(folder_path + triagle_f_name, 0)
triangle_copy = triangle.copy()
triangle_copy = cv2.Canny(triangle, 1, 2)
#thresh, ret = cv2.threshold(triangle, 5, 255, 0)
#triangle = cv2.Canny(triangle, 10, 20)
contours,hierarchy = cv2.findContours(triangle_copy, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
biggest_contour = get_biggest_contour(contours)

cv2.drawContours(triangle, biggest_contour, -1, (127, 127, 127), thickness=3, lineType = 8)
cv2.imshow('frame', triangle)
cv2.waitKey(0)&0xFF
cv2.destroyAllWindows()
#else:
#    print 'found nothing'