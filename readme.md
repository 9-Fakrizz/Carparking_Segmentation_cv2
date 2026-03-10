# Smart Parking Detection with Telegram Notification 🚗

## Overview

โปรเจกต์นี้เป็นระบบตรวจจับสถานะช่องจอดรถโดยใช้ **Computer Vision ด้วย OpenCV** จากกล้องเว็บแคม และแจ้งเตือนสถานะไปยัง **Telegram Bot** เมื่อมีการเปลี่ยนแปลง เช่น รถเข้าจอดหรือรถออกจากช่อง

ระบบสามารถ

* ตรวจจับว่าช่องจอด **ว่างหรือมีรถ**
* ตรวจจับ **สีของรถ**
* แจ้งเตือนสถานะ **ทุกช่องพร้อมกัน**
* ส่ง **ภาพจากกล้องไปยัง Telegram**

---

# System Workflow

1. กล้องอ่านภาพจาก webcam
2. แปลงภาพเป็น **HSV color space**
3. ใช้ **threshold segmentation** เพื่อแยกวัตถุ
4. ตรวจสอบ pixel ในพื้นที่ช่องจอด
5. ถ้า pixel มากกว่า threshold → ถือว่ามีรถ
6. ตรวจจับสีของรถ
7. เมื่อสถานะเปลี่ยน → ส่งข้อความ Telegram

---

# Code Structure

## 1. Import Library

```python
import cv2
import numpy as np
import requests
```

ใช้สำหรับ

| Library  | Purpose                  |
| -------- | ------------------------ |
| OpenCV   | ประมวลผลภาพ              |
| Numpy    | คำนวณ array              |
| Requests | ส่งข้อมูลไป Telegram API |

---

# Telegram Notification System

```python
def send_telegram(text, image=None)
```

ฟังก์ชันนี้ใช้ส่งข้อมูลไป Telegram Bot

ทำงานโดย

1. ส่งข้อความผ่าน API

```
https://api.telegram.org/bot<TOKEN>/sendMessage
```

2. ถ้ามีภาพจะส่งผ่าน

```
sendPhoto
```

ข้อดี

* แจ้งเตือนแบบ **Real-time**
* ส่งทั้ง **ข้อความ + รูปภาพ**

---

# Color Detection

```python
def detect_color(img)
```

ใช้ตรวจสอบสีของรถโดยใช้ **HSV Color Segmentation**

หลักการ

1. แปลงภาพจาก

```
BGR → HSV
```

2. กำหนดช่วงสี

```
Blue
Green
Yellow
Red
```

3. สร้าง mask

```
cv2.inRange()
```

4. นับจำนวน pixel

```
cv2.countNonZero()
```

5. สีที่ pixel มากที่สุดคือสีของรถ

ข้อดี

HSV ทนต่อ **แสงที่เปลี่ยนแปลงได้ดีกว่า RGB**

---

# Parking Area Definition

```python
parking_areas = [
polygon1,
polygon2,
polygon3,
polygon4
]
```

กำหนดตำแหน่งช่องจอดเป็น **Polygon**

ตัวอย่าง

```
ช่องที่ 1
(10,150)
(130,150)
(130,300)
(10,300)
```

ข้อดี

สามารถกำหนดพื้นที่ **รูปทรงใดก็ได้**

---

# Car Detection Algorithm

ระบบใช้ **pixel counting method**

ขั้นตอน

1. สร้าง mask เฉพาะพื้นที่ช่องจอด

```
cv2.fillPoly()
```

2. ตัดเฉพาะพื้นที่ช่อง

```
cv2.bitwise_and()
```

3. นับ pixel

```
cv2.countNonZero()
```

4. เปรียบเทียบกับ threshold

```
white_pixels > 1900
```

ถ้ามากกว่า → มีรถ

---

# Noise Reduction

ใช้เทคนิค

```
medianBlur
morphological opening
morphological closing
```

ช่วยลด

* noise จากกล้อง
* เงา
* pixel กระพริบ

---

# Stable Detection

ใช้ตัวแปร

```
STABLE_FRAMES = 8
```

หมายความว่า

สถานะต้อง **เหมือนกัน 8 frame ต่อเนื่อง** ถึงจะยอมรับว่าเปลี่ยน

ข้อดี

ลดปัญหา

* false detection
* detection กระพริบ

---

# Telegram Alert System

เมื่อสถานะเปลี่ยน

```
update_detected = True
```

ระบบจะสร้างข้อความ

```
📊 สถานะลานจอดรถ

ช่องที่ 1 ว่าง
ช่องที่ 2 รถสี Blue
ช่องที่ 3 ว่าง
ช่องที่ 4 รถสี Red
```

แล้วส่งไป Telegram

---

# Visualization

OpenCV จะแสดง

* กรอบสีเขียว = ช่องว่าง
* กรอบสีแดง = มีรถ
* ชื่อสีรถ

ตัวอย่าง

```
1 Blue
2 Empty
3 Red
4 Empty
```

---

# Advantages

## 1. Real-Time Monitoring

สามารถตรวจสอบสถานะช่องจอดแบบทันที

---

## 2. Low Cost System

ใช้เพียง

* Webcam
* Computer
* Python

---

## 3. Simple Algorithm

ใช้ image processing แบบพื้นฐาน

ไม่ต้องใช้ AI หรือ GPU

---

## 4. Telegram Integration

สามารถแจ้งเตือนผ่านมือถือได้ทันที

---

# Limitations

## 1. Lighting Sensitivity

ถ้าแสงเปลี่ยนมาก

อาจทำให้

* detect สีผิด
* detect รถผิด

---

## 2. Fixed Camera Position

กล้องต้อง **อยู่ตำแหน่งเดิม**

ถ้าเลื่อนตำแหน่ง

polygon จะผิด

---

## 3. Limited Color Detection

ระบบตรวจจับแค่

* Red
* Blue
* Green
* Yellow

สีอื่นจะเป็น

```
Unknown
```

---

## 4. Occlusion Problem

ถ้ามีวัตถุอื่นเข้าบัง

อาจ detect ผิด

เช่น

* คนเดินผ่าน
* เงา

---

# Possible Improvements

ระบบสามารถพัฒนาเพิ่มได้ เช่น

### 1. Deep Learning Detection

ใช้

```
YOLOv8
Mask R-CNN
```

เพื่อ detect รถได้แม่นยำกว่า

---

### 2. Web Dashboard

ทำ dashboard ด้วย

```
Flask
React
NodeJS
```

แสดงสถานะลานจอด

---

### 3. Database Logging

บันทึกข้อมูล

```
รถเข้า
รถออก
เวลา
สีรถ
```

ลง

```
MySQL
InfluxDB
```

---

### 4. License Plate Recognition

เพิ่มระบบ

```
OCR
License Plate Detection
```

เพื่อระบุรถ

---

# Conclusion

ระบบนี้เป็นตัวอย่างของการใช้ **Computer Vision สำหรับ Smart Parking System**

จุดเด่นคือ

* ใช้งานง่าย
* ต้นทุนต่ำ
* แจ้งเตือนแบบ Real-time

แม้จะมีข้อจำกัดด้านแสงและมุมกล้อง แต่สามารถพัฒนาต่อยอดเป็น **Smart City Application** ได้
