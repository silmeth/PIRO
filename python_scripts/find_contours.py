import numpy as np
import cv2

def get_biggest_contour(contours, fill, max_area):
    area = -1
    if contours:
        for cnt in contours:
            if cv2.contourArea(cnt) > area:
                biggest_cnt = cnt
        if cv2.contourArea(biggest_cnt) > fill*max_area:
            return [biggest_cnt]
        else:
            return []
    else:
        return []

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
    kernel = np.ones((5,5),np.uint8)
    min_mat = cv2.erode(min_mat, kernel, iterations=3)
    min_mat = cv2.dilate(min_mat, kernel, iterations=3)
    min_mat = cv2.Canny(min_mat, 10, 20)
    # Works nice!
    # Find contours
    contours, hierarchy = cv2.findContours(min_mat, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
    max_cnt_area = 640*480
    # Check if contour has area of 25% or more
    biggest_contour = get_biggest_contour(contours, 0.15, max_cnt_area)
    if biggest_contour:
        # Approx boundary of the page
        approx_bnd = cv2.approxPolyDP(biggest_contour[0], 50, True)
        if len(approx_bnd) == 4:
            cv2.drawContours(frame, [approx_bnd], -1, (0, 0,255), 0)
        #cv2.drawContours(frame, biggest_contour, -1, (0, 0,255), 0)

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()

