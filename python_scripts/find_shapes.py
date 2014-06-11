import numpy as np
import cv2
import cv2.cv # here

def get_biggest_contour(contours):
    area = -1
    if contours:
        for cnt in contours:
            if cv2.contourArea(cnt) > area:
                biggest_cnt = cnt
            return [biggest_cnt]
    else:
        return []

cap = cv2.VideoCapture(1)

folder_path = "/home/jacek/Studia/PIRO/face_rec_test/benchmark_images/"
triagle_f_name = "triangle.png"
rectangle_f_name = "rectangle.png"

triangle = cv2.imread(folder_path + triagle_f_name, 0)
rectangle = cv2.imread(folder_path + rectangle_f_name, 0)
triangle_copy = triangle.copy()
rectangle_copy = rectangle.copy()
contours,hierarchy = cv2.findContours(triangle_copy, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
contours2,hierarchy2 = cv2.findContours(rectangle_copy, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
triangle_contour = get_biggest_contour(contours)
rectangle_contour = get_biggest_contour(contours)

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # Apply blur
    blur_gray = cv2.GaussianBlur(gray, (5,5), 0)
    h, s, v = cv2.split(hsv)
    #blur_h = cv2.GaussianBlur(h, (5, 5), 0)
    #blur_s = cv2.GaussianBlur(s, (5, 5), 0)
    blur_v = cv2.GaussianBlur(v, (5, 5), 0)
    th_otsu_gray, otsu_gray = cv2.threshold(blur_gray,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    th_otsu_v, otsu_v = cv2.threshold(blur_v, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    contours, hierarchy = cv2.findContours(otsu_gray, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
    for cnt in contours:
        if not (cv2.contourArea(cnt) > 0.1*640*480):
            if cv2.matchShapes(cnt, triangle_contour[0], 1, 0.0) > 0.25:
                cv2.drawContours(frame, [cnt], -1, (0, 255, 0), 1, 1, maxLevel=0)
            elif cv2.matchShapes(cnt, rectangle_contour[0], 1, 0.0) > 0.2:
                cv2.drawContours(frame, [cnt], -1, (0, 0, 255), 1, 1, maxLevel=0)
    circles = cv2.HoughCircles(otsu_gray,cv2.cv.CV_HOUGH_GRADIENT,1,20,
                            param1=50,param2=30,minRadius=0,maxRadius=0)
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()
