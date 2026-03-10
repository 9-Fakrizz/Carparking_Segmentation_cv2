import cv2
import numpy as np
import requests

# ========= TELEGRAM SETTINGS =========
BOT_TOKEN = "8287688254:AAGUwKSUO6zXYhdbq0blx_vtfGHoC6sQlSk"
CHAT_ID   = "8572457986"

def send_telegram(text, image=None):

    try:
        url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
        requests.post(url, data={"chat_id": CHAT_ID, "text": text})

        if image is not None:

            url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendPhoto"

            _, img = cv2.imencode(".jpg", image)

            requests.post(
                url,
                data={"chat_id": CHAT_ID},
                files={"photo": img.tobytes()}
            )

    except Exception as e:
        print(e)

# =====================================


# ===== COLOR DETECTION =====
def detect_color(img):

    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    color_ranges = {

        "Blue": ([60,10,50],[200,255,255]),

        "Green": ([30,20,50],[100,255,255]),

        "Yellow": ([20,100,100],[35,255,255]),

        "Red": ([5,150,150],[20,255,255])

    }

    max_pixels = 0
    detected_color = "Unknown"

    for color,(lower,upper) in color_ranges.items():

        lower = np.array(lower)
        upper = np.array(upper)

        mask = cv2.inRange(hsv,lower,upper)

        pixels = cv2.countNonZero(mask)

        if pixels > max_pixels:

            max_pixels = pixels
            detected_color = color

    return detected_color

# =====================================


cap = cv2.VideoCapture(0)

parking_areas = [

np.array([[10,150],[130,150],[130,300],[10,300]],dtype=np.int32),
np.array([[170,150],[300,150],[300,300],[170,300]],dtype=np.int32),
np.array([[320,150],[450,150],[450,300],[320,300]],dtype=np.int32),
np.array([[480,150],[630,150],[630,300],[480,300]],dtype=np.int32)

]

prev_status = [False]*len(parking_areas)
stable_count = [0]*len(parking_areas)

car_colors = ["None"]*len(parking_areas)

STABLE_FRAMES = 8


while True:

    ret,frame = cap.read()

    if not ret:
        break

    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)

    lower = np.array([0,40,40])
    upper = np.array([180,255,255])

    mask = cv2.inRange(hsv,lower,upper)

    kernel = np.ones((5,5),np.uint8)

    mask = cv2.medianBlur(mask,5)
    mask = cv2.morphologyEx(mask,cv2.MORPH_OPEN,kernel)
    mask = cv2.morphologyEx(mask,cv2.MORPH_CLOSE,kernel)

    update_detected = False


    for i,area in enumerate(parking_areas):

        zone_mask = np.zeros(mask.shape,dtype=np.uint8)

        cv2.fillPoly(zone_mask,[area],255)

        zone_pixels = cv2.bitwise_and(mask,mask,mask=zone_mask)

        white_pixels = cv2.countNonZero(zone_pixels)

        occupied = white_pixels > 1900

        if occupied == prev_status[i]:

            stable_count[i] = 0

        else:

            stable_count[i] += 1

            if stable_count[i] >= STABLE_FRAMES:

                prev_status[i] = occupied
                stable_count[i] = 0
                update_detected = True

                if occupied:

                    x,y,w,h = cv2.boundingRect(area)

                    crop = frame[y:y+h,x:x+w]

                    car_color = detect_color(crop)

                    car_colors[i] = car_color

                else:

                    car_colors[i] = "None"



    # ===== DRAW PARKING AREA =====
    for i,area in enumerate(parking_areas):

        color = (0,0,255) if prev_status[i] else (0,255,0)

        cv2.polylines(frame,[area],True,color,3)

        label = f"{i+1}"

        if prev_status[i]:

            label += f" {car_colors[i]}"

        cv2.putText(frame,label,(area[0][0],area[0][1]-10),
                    cv2.FONT_HERSHEY_SIMPLEX,0.6,color,2)



    # ===== TELEGRAM ALERT =====
    if update_detected:

        msg = "📊 สถานะลานจอดรถ\n\n"

        for i in range(len(parking_areas)):

            if prev_status[i]:

                msg += f"ช่องที่ {i+1} รถสี {car_colors[i]}\n"

            else:

                msg += f"ช่องที่ {i+1} ว่าง\n"

        print(msg)

        send_telegram(msg,frame)



    cv2.imshow("parking",frame)
    cv2.imshow("mask",mask)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break


cap.release()
cv2.destroyAllWindows()
