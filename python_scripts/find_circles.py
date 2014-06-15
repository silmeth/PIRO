import cv2

cap = cv2.VideoCapture(1)

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # Apply blur
    blur_gray = cv2.GaussianBlur(gray, (5,5), 0)
    th_otsu_gray, otsu_gray = cv2.threshold(blur_gray,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    otsu_gray_temp = otsu_gray.copy()
    contours, hierarchy = cv2.findContours(otsu_gray_temp, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)
    circles = []
    circles = cv2.HoughCircles(otsu_gray, cv2.cv.CV_HOUGH_GRADIENT, 2, 20, maxRadius = 200, param1=100, param2=120)
    if not circles == None:
        for c in circles[0]:
            print c
            cv2.circle(frame, (c[0], c[1]), c[2], (255, 255, 255), 4) 
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()