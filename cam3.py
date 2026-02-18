import cv2
import numpy as np
import requests
import time

# ========= TELEGRAM SETTINGS =========
# แนะนำให้เก็บ Token เป็นความลับนะครับ (Environment Variable)
BOT_TOKEN = "8287688254:AAGUwKSUO6zXYhdbq0blx_vtfGHoC6sQlSk"
CHAT_ID   = "8572457986"

def send_telegram(text, image=None):
    try:
        url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
        requests.post(url, data={"chat_id": CHAT_ID, "text": text})

        if image is not None:
            url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendPhoto"
            _, img_encoded = cv2.imencode('.jpg', image)
            requests.post(url,
                          data={"chat_id": CHAT_ID},
                          files={"photo": img_encoded.tobytes()})
    except Exception as e:
        print(f"Error sending telegram: {e}")

# =====================================

cap = cv2.VideoCapture(0)

parking_areas = [
    np.array([[10,150],[130,150],[130,300],[10,300]]),
    np.array([[170,150],[300,150],[300,300],[170,300]]),
    np.array([[320,150],[450,150],[450,300],[320,300]]),
    np.array([[480,150],[630,150],[630,300],[480,300]])
]

# store previous status
prev_status = [False]*len(parking_areas)

# stability filter
stable_count = [0]*len(parking_areas)
STABLE_FRAMES = 8   # prevent flicker

while True:
    ret, frame = cap.read()
    if not ret:
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # detect non-floor colors
    lower = np.array([0, 40, 40])
    upper = np.array([180, 255, 255])
    mask = cv2.inRange(hsv, lower, upper)

    # ===== noise filtering =====
    kernel = np.ones((5,5), np.uint8)
    mask = cv2.medianBlur(mask, 5)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    status_text = []
    
    # ตัวแปรเช็คว่ารอบนี้มีการเปลี่ยนแปลงไหม
    update_detected = False 

    for i, area in enumerate(parking_areas):
        zone_mask = np.zeros(mask.shape, dtype=np.uint8)
        cv2.fillPoly(zone_mask, [area], 255)

        zone_pixels = cv2.bitwise_and(mask, mask, mask=zone_mask)
        white_pixels = cv2.countNonZero(zone_pixels)

        occupied = white_pixels > 2000 

        # ===== stability filter =====
        if occupied == prev_status[i]:
            stable_count[i] = 0
        else:
            stable_count[i] += 1
            if stable_count[i] >= STABLE_FRAMES:
                prev_status[i] = occupied
                stable_count[i] = 0
                update_detected = True # เจอการเปลี่ยนแปลง! (แต่ยังไม่ส่ง รอจบ Loop ก่อน)

        color = (0,0,255) if prev_status[i] else (0,255,0)
        cv2.polylines(frame, [area], True, color, 3)
        status_text.append(f"{i+1}:{'X' if prev_status[i] else 'OK'}")

    # ===== ส่ง Telegram เมื่อจบลูปพื้นที่ทั้งหมด และมีการเปลี่ยนแปลง =====
    if update_detected:
        msg_lines = ["🚗 อัพเดทสถานะจอดรถ:"]
        for idx, is_occupied in enumerate(prev_status):
            status_str = "❌ ไม่ว่าง" if is_occupied else "✅ ว่าง"
            msg_lines.append(f"ช่อง {idx+1}: {status_str}")
        
        full_msg = "\n".join(msg_lines)
        print(full_msg) # ปริ้นดูในคอมด้วย
        send_telegram(full_msg, frame)

    # =============================================================

    cv2.putText(frame, " ".join(status_text),
                (20,40),
                cv2.FONT_HERSHEY_SIMPLEX,
                1,(255,255,255),2)

    cv2.imshow("mask", mask)
    cv2.imshow("parking", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
