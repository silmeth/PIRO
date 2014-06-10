import numpy as np
import cv2
from matplotlib import pyplot as plt

cap = cv2.VideoCapture(1)

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
    min_mat = cv2.min(otsu_gray, otsu_v)
    # Works nice!
    # Find contours
    contours, hierarchy = cv2.findContours(min_mat, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    #print contours
    #print len(contours)
    cv2.drawContours(frame, contours, -1, (0, 0,255), 3)

    cv2.imshow('frame', min_mat)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()